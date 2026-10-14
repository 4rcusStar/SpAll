// IncludePreprocessor.cpp — recursive #include expansion.
#include "IncludePreprocessor.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace spall {

// --- LineMap -----------------------------------------------------------------

void LineMap::append(const QString& file, int line) {
    m_lines.append(SourceLine{file, line});
}

bool LineMap::resolve(int expandedLine, QString* file, int* line) const {
    const int index = expandedLine - 1;
    if (index < 0 || index >= m_lines.size()) {
        return false;
    }
    if (file) {
        *file = m_lines.at(index).file;
    }
    if (line) {
        *line = m_lines.at(index).line;
    }
    return true;
}

bool LineMap::isIdentity(const QString& mainFile) const {
    for (int i = 0; i < m_lines.size(); ++i) {
        if (m_lines.at(i).line != i + 1 || m_lines.at(i).file != mainFile) {
            return false;
        }
    }
    return true;
}

// --- IncludePreprocessor -----------------------------------------------------

namespace {

// Localised string for this translation context.
//
// A macro rather than a function on purpose: lupdate parses sources statically
// and cannot see through a call that passes the text on to
// QCoreApplication::translate(). Spelling the lookup out here means every trs(QT_TRANSLATE_NOOP("Preprocessor", ...))
// call site is extracted automatically, while the runtime behaviour is unchanged.
/// Localised string for this translation context.
///
/// The text is passed through QT_TRANSLATE_NOOP at every call site, which is what
/// lets lupdate extract it statically; this function then performs the runtime
/// lookup. core/ has no QObject, so tr() is not available there.
QString trs(const char* text) {
    return QCoreApplication::translate("Preprocessor", text);
}

const QRegularExpression& includeRegex() {
    // Matches:  #  include  "file"  /  <file>   with optional spaces.
    static const QRegularExpression re(
        QStringLiteral(R"(^\s*#\s*include\s*([<"])([^>"]+)[>"])"));
    return re;
}

const QRegularExpression& anyDirectiveRegex() {
    static const QRegularExpression re(QStringLiteral(R"(^\s*#\s*(\w+))"));
    return re;
}

/// Read a file as UTF-8 text. Returns false (and fills `error`) on failure.
bool readTextFile(const QString& path, QString* out, QString* error) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) {
            *error = trs(QT_TRANSLATE_NOOP("Preprocessor", "Cannot read file %1: %2")).arg(path, file.errorString());
        }
        return false;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    *out = stream.readAll();
    return true;
}

/// Normalize path separators so comparisons and diagnostics are stable.
QString canonicalPath(const QString& path) {
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

}  // namespace

PreprocessResult IncludePreprocessor::process(const QString& source,
                                              const QString& mainFile,
                                              const QStringList& includePaths,
                                              bool enabled) const {
    PreprocessResult result;
    const QString main = canonicalPath(mainFile.isEmpty() ? QStringLiteral("<editor>") : mainFile);

    if (!enabled) {
        // Pass through, but still build an identity map so callers can remap
        // unconditionally.
        result.source = source;
        const QStringList lines = source.split(QLatin1Char('\n'));
        for (int i = 0; i < lines.size(); ++i) {
            result.map.append(main, i + 1);
        }
        result.ok = true;
        return result;
    }

    QStringList stack;
    stack << main;
    result.includedFiles << main;
    expand(main, source, includePaths, &result.source, &result.map, &result, &stack);
    result.ok = result.diagnostics.isEmpty();
    return result;
}

void IncludePreprocessor::expand(const QString& file,
                                 const QString& source,
                                 const QStringList& includePaths,
                                 QString* out,
                                 LineMap* map,
                                 PreprocessResult* result,
                                 QStringList* stack) const {
    const QStringList lines = source.split(QLatin1Char('\n'));
    const QDir fileDir = QFileInfo(file).absoluteDir();

    for (int i = 0; i < lines.size(); ++i) {
        const QString& rawLine = lines.at(i);
        const int originalLine = i + 1;

        const QRegularExpressionMatch match = includeRegex().match(rawLine);
        if (!match.hasMatch()) {
            // Handle the line-continuation form "#incl\<newline>ude" by falling
            // back to a plain directive check: anything else is emitted verbatim.
            const QRegularExpressionMatch directive = anyDirectiveRegex().match(rawLine);
            if (directive.hasMatch() && directive.captured(1) == QLatin1String("error")) {
                result->diagnostics.append(Diagnostic{Severity::Error, DiagSource::Preprocess,
                                                      originalLine, 0, file,
                                                      rawLine.mid(rawLine.indexOf(QStringLiteral("error")) + 5).trimmed()});
            }
            out->append(rawLine);
            out->append(QLatin1Char('\n'));
            map->append(file, originalLine);
            continue;
        }

        const QString bracket = match.captured(1);
        const QString target = match.captured(2).trimmed();
        const bool angled = bracket == QLatin1String("<");

        // Resolve the include target.
        QString resolved;
        if (!angled) {
            const QString candidate = fileDir.absoluteFilePath(target);
            if (QFileInfo::exists(candidate)) {
                resolved = candidate;
            }
        }
        if (resolved.isEmpty() && !includePaths.isEmpty()) {
            for (const QString& dir : includePaths) {
                const QString candidate = QDir(dir).absoluteFilePath(target);
                if (QFileInfo::exists(candidate)) {
                    resolved = candidate;
                    break;
                }
            }
        }
        if (resolved.isEmpty()) {
            // Last resort: treat a quoted name as relative to the main file.
            const QString candidate = fileDir.absoluteFilePath(target);
            if (QFileInfo::exists(candidate)) {
                resolved = candidate;
            }
        }

        if (resolved.isEmpty()) {
            result->diagnostics.append(Diagnostic{
                Severity::Error, DiagSource::Preprocess, originalLine,
                static_cast<int>(match.capturedStart(2)) + 1, file,
                trs(QT_TRANSLATE_NOOP("Preprocessor", "Cannot find the file \"%1\" for this #include. Add an #include search "
                    "path in the settings, or use a path relative to the current file."))
                    .arg(target)});
            // Emit a comment so the line count and downstream parsing stay sane.
            out->append(QStringLiteral("// [SpAll] unresolved include: %1\n").arg(target));
            map->append(file, originalLine);
            continue;
        }

        const QString canonical = canonicalPath(resolved);

        if (stack->size() >= kMaxDepth) {
            result->diagnostics.append(Diagnostic{
                Severity::Error, DiagSource::Preprocess, originalLine, 0, file,
                trs(QT_TRANSLATE_NOOP("Preprocessor", "#include nesting is deeper than %1 levels. Check for an overly deep "
                    "include chain.")).arg(kMaxDepth)});
            out->append(QLatin1Char('\n'));
            map->append(file, originalLine);
            continue;
        }

        if (stack->contains(canonical)) {
            result->diagnostics.append(Diagnostic{
                Severity::Error, DiagSource::Preprocess, originalLine, 0, file,
                trs(QT_TRANSLATE_NOOP("Preprocessor", "Circular #include detected: %1 has already been included. "
                    "Break this include cycle.")).arg(canonical)});
            out->append(QLatin1Char('\n'));
            map->append(file, originalLine);
            continue;
        }

        QString included;
        QString error;
        if (!readTextFile(canonical, &included, &error)) {
            result->diagnostics.append(Diagnostic{Severity::Error, DiagSource::Preprocess,
                                                  originalLine, 0, file, error});
            out->append(QLatin1Char('\n'));
            map->append(file, originalLine);
            continue;
        }

        // Emit a "#line" style marker as a comment for readability, but keep the
        // real tracking in the line map (comments do not shift glslang's lines
        // because we never inject extra physical lines).
        result->includedFiles << canonical;
        stack->append(canonical);
        expand(canonical, included, includePaths, out, map, result, stack);
        stack->removeLast();

        // Guard against an included file without a trailing newline.
        if (!out->endsWith(QLatin1Char('\n'))) {
            out->append(QLatin1Char('\n'));
        }
    }
}

void IncludePreprocessor::remap(Diagnostic* diagnostic, const LineMap& map, const QString& mainFile) {
    if (!diagnostic || !diagnostic->hasPosition()) {
        return;
    }
    QString file;
    int line = 0;
    if (!map.resolve(diagnostic->line, &file, &line)) {
        // Out of range: keep the position but attribute it to the main file so
        // the UI can still jump somewhere sensible.
        diagnostic->file = mainFile;
        return;
    }
    diagnostic->file = file.isEmpty() ? mainFile : file;
    diagnostic->line = line;
}

}  // namespace spall
