// Diagnostic.cpp — labels and token parsing for diagnostics.
#include "Diagnostic.h"

#include <QCoreApplication>
#include <QHash>
#include <QRegularExpression>
#include <QStringList>

#include <algorithm>

namespace spall {

namespace {

// Localised string for this translation context.
//
// A macro rather than a function on purpose: lupdate parses sources statically
// and cannot see through a call that passes the text on to
// QCoreApplication::translate(). Spelling the lookup out here means every trs(QT_TRANSLATE_NOOP("Diagnostic", ...))
// call site is extracted automatically, while the runtime behaviour is unchanged.
/// Localised string for this translation context.
///
/// The text is passed through QT_TRANSLATE_NOOP at every call site, which is what
/// lets lupdate extract it statically; this function then performs the runtime
/// lookup. core/ has no QObject, so tr() is not available there.
QString trs(const char* text) {
    return QCoreApplication::translate("Diagnostic", text);
}

}  // namespace

QString diagSourceName(DiagSource source) {
    switch (source) {
        case DiagSource::GlslSyntax:      return trs(QT_TRANSLATE_NOOP("Diagnostic", "GLSL syntax"));
        case DiagSource::Link:            return trs(QT_TRANSLATE_NOOP("Diagnostic", "Link"));
        case DiagSource::Preprocess:      return trs(QT_TRANSLATE_NOOP("Diagnostic", "Preprocess"));
        case DiagSource::SpirvValidation: return trs(QT_TRANSLATE_NOOP("Diagnostic", "SPIR-V validation"));
        case DiagSource::SpirvOptimizer:  return trs(QT_TRANSLATE_NOOP("Diagnostic", "Optimization"));
        case DiagSource::StaticHint:      return trs(QT_TRANSLATE_NOOP("Diagnostic", "Hint"));
    }
    return trs(QT_TRANSLATE_NOOP("Diagnostic", "Unknown"));
}

QString severityName(Severity severity) {
    switch (severity) {
        case Severity::Error:   return trs(QT_TRANSLATE_NOOP("Diagnostic", "Error"));
        case Severity::Warning: return trs(QT_TRANSLATE_NOOP("Diagnostic", "Warning"));
        case Severity::Info:    return trs(QT_TRANSLATE_NOOP("Diagnostic", "Info"));
        case Severity::Hint:    return trs(QT_TRANSLATE_NOOP("Diagnostic", "Hint"));
    }
    return trs(QT_TRANSLATE_NOOP("Diagnostic", "Info"));
}

bool severityFromToken(const QString& token, Severity* out) {
    static const QHash<QString, Severity> table = {
        {QStringLiteral("ERROR"), Severity::Error},
        {QStringLiteral("WARNING"), Severity::Warning},
        {QStringLiteral("INFO"), Severity::Info},
        {QStringLiteral("HINT"), Severity::Hint},
        // glslang occasionally emits these variations.
        {QStringLiteral("ERRORS"), Severity::Error},
        {QStringLiteral("WARN"), Severity::Warning},
    };
    const auto it = table.constFind(token.trimmed().toUpper());
    if (it == table.constEnd()) {
        return false;
    }
    if (out) {
        *out = it.value();
    }
    return true;
}

bool diagSourceFromName(const QString& name, DiagSource* out) {
    // The keys must stay in sync with the labels diagSourceName() returns, so the
    // settings round-trip keeps working in any language.
    static const QHash<QString, DiagSource> table = {
        {trs(QT_TRANSLATE_NOOP("Diagnostic", "GLSL syntax")), DiagSource::GlslSyntax},
        {trs(QT_TRANSLATE_NOOP("Diagnostic", "Link")), DiagSource::Link},
        {trs(QT_TRANSLATE_NOOP("Diagnostic", "Preprocess")), DiagSource::Preprocess},
        {trs(QT_TRANSLATE_NOOP("Diagnostic", "SPIR-V validation")), DiagSource::SpirvValidation},
        {trs(QT_TRANSLATE_NOOP("Diagnostic", "Optimization")), DiagSource::SpirvOptimizer},
        {trs(QT_TRANSLATE_NOOP("Diagnostic", "Hint")), DiagSource::StaticHint},
    };
    const auto it = table.constFind(name);
    if (it == table.constEnd()) {
        return false;
    }
    if (out) {
        *out = it.value();
    }
    return true;
}

// --- heuristic syntax hints --------------------------------------------------

namespace hints {

namespace {

/// Remove // and /* */ comments, tracking block comments across lines.
///
/// Without this, a trailing comment makes a complete line look unterminated:
/// `if ( c.r <= 0.0031308 )//note` would be judged by its final characters.
QStringList stripComments(const QStringList& lines) {
    QStringList stripped;
    stripped.reserve(lines.size());
    bool inBlockComment = false;

    for (const QString& raw : lines) {
        QString out;
        int i = 0;
        while (i < raw.size()) {
            if (inBlockComment) {
                const int end = raw.indexOf(QStringLiteral("*/"), i);
                if (end < 0) {
                    i = raw.size();
                    break;
                }
                i = end + 2;
                inBlockComment = false;
                continue;
            }
            if (raw.mid(i, 2) == QLatin1String("//")) {
                break;  // rest of the line is a comment
            }
            if (raw.mid(i, 2) == QLatin1String("/*")) {
                inBlockComment = true;
                i += 2;
                continue;
            }
            out.append(raw.at(i));
            ++i;
        }
        stripped.append(out);
    }
    return stripped;
}

/// Net parenthesis balance of one line.
int parenBalance(const QString& line) {
    int balance = 0;
    for (const QChar ch : line) {
        if (ch == QLatin1Char('(')) {
            ++balance;
        } else if (ch == QLatin1Char(')')) {
            --balance;
        }
    }
    return balance;
}

/// True when the line ends in a way that clearly expects a continuation.
///
/// Deliberately narrow. A line ending in '{' or '}' or ':' is a *complete* line of
/// GLSL (a block opener, a block closer, a label), so treating those as
/// continuations would hide real mistakes on the following line.
bool endsWithContinuation(const QString& trimmed) {
    static const QChar kOpeners[] = {
        QLatin1Char('('), QLatin1Char('['), QLatin1Char(','), QLatin1Char('\\'),
        QLatin1Char('?'),
    };
    for (const QChar ch : kOpeners) {
        if (trimmed.endsWith(ch)) {
            return true;
        }
    }
    // Trailing binary operator, e.g. "float y = a +".
    static const QRegularExpression trailingOperator(
        QStringLiteral(R"([+\-*/%&|^!<>=]=?$)"));
    return trailingOperator.match(trimmed).hasMatch();
}

}  // namespace

QVector<Diagnostic> findMissingSemicolons(const QString& source, const QString& fileName) {
    QVector<Diagnostic> found;
    if (source.trimmed().isEmpty()) {
        return found;
    }

    const QStringList lines = source.split(QLatin1Char('\n'));
    const QStringList code = stripComments(lines);

    // A line starting with one of these never needs a ';' itself: its body or its
    // brace follows on the same or the next line.
    static const QRegularExpression controlFlow(
        QStringLiteral(R"(^\s*(?:if|for|while|switch|else|do)\b)"));
    // Lines that look like an actual statement worth checking.
    static const QRegularExpression looksLikeStatement(QStringLiteral(
        R"(^\s*(?:return|break|continue|discard)\b|^\s*(?:[A-Za-z_]\w*\s+)+[A-Za-z_]\w*\s*=|^\s*[A-Za-z_]\w*(?:\s*\.\s*[A-Za-z_]\w*)*\s*=[^=]|^\s*[A-Za-z_]\w*(?:\s*\.\s*[A-Za-z_]\w*)*\s*\([^;]*\)\s*$)"));

    int braceDepth = 0;

    for (int i = 0; i < lines.size(); ++i) {
        const QString line = code.at(i);
        const QString trimmed = line.trimmed();
        const QString previousTrimmed = i > 0 ? code.at(i - 1).trimmed() : QString();

        const int opens = line.count(QLatin1Char('{'));
        const int closes = line.count(QLatin1Char('}'));
        const int depthBefore = braceDepth;
        braceDepth += opens - closes;

        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
            continue;
        }
        if (depthBefore < 1) {
            continue;  // outside any block: globals, not statements
        }
        // Already terminated: nothing to report. (A trailing comment has been
        // stripped by this point, so "x = 1.0;  // note" still ends in ';'.)
        if (trimmed.endsWith(QLatin1Char(';'))) {
            continue;
        }
        if (endsWithContinuation(trimmed)) {
            continue;
        }
        if (controlFlow.match(trimmed).hasMatch()) {
            continue;
        }
        // The previous line left something open, so this line is its continuation:
        // the argument list of "TBN = mat3(", or the right-hand side of "x = a +".
        // This is the check that keeps continuation lines from being flagged.
        if (!previousTrimmed.isEmpty() && endsWithContinuation(previousTrimmed)) {
            continue;
        }
        // A line that closes a block needs no ';' of its own.
        if (closes > opens) {
            continue;
        }
        // Parentheses must balance on this line; an unbalanced line is part of a
        // larger expression.
        if (parenBalance(line) != 0) {
            continue;
        }
        if (!looksLikeStatement.match(trimmed).hasMatch()) {
            continue;
        }

        found.append(Diagnostic{
            Severity::Error, DiagSource::StaticHint, i + 1, static_cast<int>(lines.at(i).size()),
            fileName,
            trs(QT_TRANSLATE_NOOP("Diagnostic", "This line looks like a complete statement but does not end with ';'. "
                "Ignore it if it just continues the previous statement; otherwise add "
                "the missing semicolon."))});
    }

    return found;
}

}  // namespace hints

}  // namespace spall
