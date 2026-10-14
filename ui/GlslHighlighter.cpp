// GlslHighlighter.cpp — rule-based GLSL syntax highlighting.
#include "GlslHighlighter.h"

#include <QTextDocument>

#include "GlslLanguageData.h"

namespace spall {

namespace {

/// Build a char format from a foreground colour plus optional weight.
QTextCharFormat makeFormat(const QColor& colour, bool bold = false, bool italic = false) {
    QTextCharFormat format;
    format.setForeground(colour);
    if (bold) {
        format.setFontWeight(QFont::Bold);
    }
    if (italic) {
        format.setFontItalic(true);
    }
    return format;
}

/// Escape a list of words into a single alternation pattern, longest first so
/// that "mat4x4" wins over "mat4".
QString alternation(const QStringList& words) {
    QStringList sorted = words;
    std::sort(sorted.begin(), sorted.end(), [](const QString& a, const QString& b) {
        return a.size() > b.size();
    });
    QStringList escaped;
    escaped.reserve(sorted.size());
    for (const QString& word : sorted) {
        escaped << QRegularExpression::escape(word);
    }
    return escaped.join(QLatin1Char('|'));
}

}  // namespace

// --- themes ------------------------------------------------------------------

HighlightTheme HighlightTheme::light() {
    HighlightTheme theme;
    theme.comment = makeFormat(QColor(0x6a, 0x73, 0x7d), false, true);
    theme.preprocessor = makeFormat(QColor(0x9c, 0x27, 0xb0));
    theme.preprocessorKeyword = makeFormat(QColor(0x9c, 0x27, 0xb0), true);
    theme.stringLiteral = makeFormat(QColor(0x0b, 0x6b, 0x2e));
    theme.numberLiteral = makeFormat(QColor(0x0b, 0x53, 0x94));
    theme.type = makeFormat(QColor(0x0b, 0x53, 0x94), true);
    theme.qualifier = makeFormat(QColor(0x9a, 0x3d, 0x00));
    theme.controlFlow = makeFormat(QColor(0xc0, 0x2b, 0x7a), true);
    theme.builtinFunction = makeFormat(QColor(0x79, 0x50, 0x0b));
    theme.builtinVariable = makeFormat(QColor(0x00, 0x6b, 0x6b), false, true);
    theme.functionDefinition = makeFormat(QColor(0x1c, 0x4d, 0x8f), true);
    // Tinted diagnostic lines carry an explicit dark foreground so the text stays
    // readable; the syntax highlighter no longer overrides it (see the note at the
    // end of highlightBlock).
    theme.errorLine.setBackground(QColor(0xff, 0xd6, 0xd6));
    theme.errorLine.setForeground(QColor(0x7a, 0x10, 0x10));
    theme.warningLine.setBackground(QColor(0xff, 0xf2, 0xc8));
    theme.warningLine.setForeground(QColor(0x5c, 0x40, 0x00));
    return theme;
}

HighlightTheme HighlightTheme::dark() {
    HighlightTheme theme;
    theme.comment = makeFormat(QColor(0x7f, 0x8c, 0x8d), false, true);
    theme.preprocessor = makeFormat(QColor(0xd8, 0x9b, 0xe8));
    theme.preprocessorKeyword = makeFormat(QColor(0xd8, 0x9b, 0xe8), true);
    theme.stringLiteral = makeFormat(QColor(0x9c, 0xdc, 0x8e));
    theme.numberLiteral = makeFormat(QColor(0x9c, 0xdc, 0xfe));
    theme.type = makeFormat(QColor(0x4e, 0xc9, 0xb0), true);
    theme.qualifier = makeFormat(QColor(0xdc, 0xdc, 0xaa));
    theme.controlFlow = makeFormat(QColor(0xf9, 0x8c, 0xd8), true);
    theme.builtinFunction = makeFormat(QColor(0xdc, 0xdc, 0xaa));
    theme.builtinVariable = makeFormat(QColor(0x9c, 0xdc, 0xfe), false, true);
    theme.functionDefinition = makeFormat(QColor(0x7f, 0xd0, 0xff), true);
    // On a dark background the tints are dark and the text is light, so the
    // contrast relationship is inverted relative to the light theme.
    theme.errorLine.setBackground(QColor(0x5a, 0x1f, 0x1f));
    theme.errorLine.setForeground(QColor(0xff, 0xd8, 0xd8));
    theme.warningLine.setBackground(QColor(0x4a, 0x3c, 0x12));
    theme.warningLine.setForeground(QColor(0xff, 0xef, 0xc0));
    return theme;
}

HighlightTheme HighlightTheme::byName(const QString& name) {
    return name.compare(QLatin1String("dark"), Qt::CaseInsensitive) == 0 ? dark() : light();
}

// --- highlighter -------------------------------------------------------------

GlslHighlighter::GlslHighlighter(QTextDocument* document)
    : QSyntaxHighlighter(document), m_theme(HighlightTheme::light()) {
    rebuildRules();
}

void GlslHighlighter::setTheme(const HighlightTheme& theme) {
    m_theme = theme;
    rebuildRules();
    rehighlight();
}

void GlslHighlighter::setDiagnostics(const QVector<Diagnostic>& diagnostics) {
    m_diagnostics = diagnostics;
    rehighlight();
}

void GlslHighlighter::clearDiagnostics() {
    if (m_diagnostics.isEmpty()) {
        return;
    }
    m_diagnostics.clear();
    rehighlight();
}

void GlslHighlighter::rebuildRules() {
    m_rules.clear();

    // --- priority 1: whole-line rules ---------------------------------------
    // Comments and preprocessor directives own the line outright; the keyword
    // rules below skip any span these already claimed.

    // // line comment
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"(//[^\n]*)")), m_theme.comment, 0, false});
    // /* block comment */ on one line
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"(/\*.*?\*/)")), m_theme.comment, 0, false});
    // preprocessor directive: #keyword
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"(^\s*#\s*(version|define|undef|include|if|ifdef|ifndef|else|elif|endif|error|pragma|extension|line)\b)")),
                       m_theme.preprocessorKeyword, 2, true});
    // any other #directive
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"(^\s*#)")), m_theme.preprocessor, 0, true});

    // --- priority 2: literals ----------------------------------------------
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"("(?:[^"\\\n]|\\.)*")")),
                       m_theme.stringLiteral});
    // Numbers: 1.0f, 0x1F, 1e-3, 0u, 1.0lf, .5
    m_rules.append(Rule{QRegularExpression(QStringLiteral(
                           R"(\b(?:0[xX][0-9a-fA-F]+[uUlL]*|(?:\d+\.\d*|\.\d+|\d+)(?:[eE][+-]?\d+)?(?:[fFuUlL]{0,3}))\b)")),
                       m_theme.numberLiteral});

    // --- priority 3: types --------------------------------------------------
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"(\b(?:%1)\b)").arg(alternation(GlslLanguageData::allTypes()))),
                       m_theme.type});

    // --- priority 4: qualifiers and keywords --------------------------------
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"(\b(?:%1)\b)").arg(alternation(GlslLanguageData::qualifiers()))),
                       m_theme.qualifier});
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"(\b(?:%1)\b)").arg(alternation(GlslLanguageData::controlFlow()))),
                       m_theme.controlFlow});

    // --- priority 5: builtins ------------------------------------------------
    QStringList builtinNames;
    for (const CompletionItem& item : GlslLanguageData::builtinFunctions()) {
        builtinNames << item.name;
    }
    builtinNames.removeDuplicates();
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"(\b(?:%1)\b)").arg(alternation(builtinNames))),
                       m_theme.builtinFunction});
    m_rules.append(Rule{QRegularExpression(QStringLiteral(R"(\b(?:%1)\b)").arg(alternation(GlslLanguageData::builtinVariables()))),
                       m_theme.builtinVariable});

    // --- priority 6: function definitions ------------------------------------
    // "returnType name(" at the start of a line, capturing the name.
    m_rules.append(Rule{QRegularExpression(QStringLiteral(
                           R"(^\s*(?:[A-Za-z_]\w*\s+)+([A-Za-z_]\w*)\s*\()")),
                       m_theme.functionDefinition, 1});
}

bool GlslHighlighter::isProtected(int start, int length) const {
    // A span is protected when the character at its start already carries a
    // foreground different from the default (i.e. an earlier rule claimed it).
    const QTextBlock block = currentBlock();
    const int blockStart = block.position();
    const QTextCharFormat existing = format(std::max(0, start - blockStart));
    Q_UNUSED(length);
    return existing.foreground().style() != Qt::NoBrush &&
           existing.foreground().color() != m_theme.comment.foreground().color() &&
           existing != QTextCharFormat() && existing.foreground().color().alpha() > 0 &&
           // The default format has no explicit foreground; anything set here came
           // from an earlier rule in this same pass.
           existing.property(QTextFormat::ForegroundBrush).isValid();
}

void GlslHighlighter::highlightBlock(const QString& text) {
    // --- multi-line block comment state -------------------------------------
    // If the previous block left a comment open, everything up to */ is comment.
    setCurrentBlockState(0);
    int commentStart = 0;
    if (previousBlockState() == 1) {
        const int end = text.indexOf(QStringLiteral("*/"));
        if (end == -1) {
            setFormat(0, text.length(), m_theme.comment);
            setCurrentBlockState(1);
            return;  // whole line is inside a block comment
        }
        setFormat(0, end + 2, m_theme.comment);
        commentStart = end + 2;
    }

    // Determine whether this block opens a block comment that never closes.
    {
        // Scan for an unterminated /* after commentStart, ignoring // comments.
        int index = commentStart;
        while (index < text.length()) {
            const int open = text.indexOf(QStringLiteral("/*"), index);
            if (open == -1) {
                break;
            }
            const int lineComment = text.indexOf(QStringLiteral("//"), index);
            if (lineComment != -1 && lineComment < open) {
                break;  // the // comment swallows the rest of the line
            }
            const int close = text.indexOf(QStringLiteral("*/"), open + 2);
            if (close == -1) {
                setFormat(open, text.length() - open, m_theme.comment);
                setCurrentBlockState(1);
                break;
            }
            index = close + 2;
        }
    }

    // `setCurrentBlockState(1)` means "inside a block comment"; the rules below
    // then only need to avoid re-tinting the already-formatted comment spans,
    // which isProtected() handles.
    const bool inBlockComment = currentBlockState() == 1;

    // --- apply the ordered rules --------------------------------------------
    for (const Rule& rule : m_rules) {
        if (rule.wholeLine) {
            // Whole-line rules only apply when the line is not inside a comment.
            if (inBlockComment) {
                continue;
            }
            const QRegularExpressionMatch match = rule.pattern.match(text);
            if (match.hasMatch()) {
                const int start = rule.captureGroup > 0 ? match.capturedStart(rule.captureGroup) : 0;
                const int length = rule.captureGroup > 0 ? match.capturedLength(rule.captureGroup)
                                                         : text.length();
                if (start >= 0 && length > 0) {
                    setFormat(start, length, rule.format);
                }
            }
            continue;
        }

        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            const int start = rule.captureGroup > 0 ? match.capturedStart(rule.captureGroup)
                                                    : match.capturedStart(0);
            const int length = rule.captureGroup > 0 ? match.capturedLength(rule.captureGroup)
                                                     : match.capturedLength(0);
            if (start < 0 || length <= 0) {
                continue;
            }
            if (isProtected(start, length)) {
                continue;  // an earlier, higher-priority rule owns this span
            }
            setFormat(start, length, rule.format);
        }
    }

    // Diagnostic line tinting is deliberately NOT done here.
    //
    // setFormat() replaces the format of a range, so tinting a line from the
    // highlighter would drop the foreground colours the syntax rules just set and
    // leave the text nearly invisible on the tint. The tint is applied instead by
    // ShaderEdit::rebuildExtraSelections() through a full-width ExtraSelection,
    // which draws underneath the text and leaves the syntax colours intact.
}

}  // namespace spall
