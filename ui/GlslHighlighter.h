// GlslHighlighter.h — GLSL syntax highlighting.
//
// Rule order matters: whole-line rules (comments, preprocessor) are applied
// first and later keyword rules skip any span already formatted, so a keyword
// inside a comment or string can never be recoloured.
#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

#include "Diagnostic.h"

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace spall {

/// One colour scheme. All colours live here — none are hard-coded in the rules.
struct HighlightTheme {
    QTextCharFormat comment;
    QTextCharFormat preprocessor;
    QTextCharFormat preprocessorKeyword;
    QTextCharFormat stringLiteral;
    QTextCharFormat numberLiteral;
    QTextCharFormat type;
    QTextCharFormat qualifier;
    QTextCharFormat controlFlow;
    QTextCharFormat builtinFunction;
    QTextCharFormat builtinVariable;
    QTextCharFormat functionDefinition;
    /// Background applied to a whole diagnostic line. The matching foreground is
    /// set explicitly so the tint can never leave low-contrast text on top of it.
    QTextCharFormat errorLine;
    QTextCharFormat warningLine;

    /// A light theme suitable for the default (light) palette.
    static HighlightTheme light();
    /// A dark theme.
    static HighlightTheme dark();
    /// Pick by name: "light", "dark", or anything else for light.
    static HighlightTheme byName(const QString& name);
};

class GlslHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit GlslHighlighter(QTextDocument* document);

    /// Swap the colour scheme and re-highlight.
    void setTheme(const HighlightTheme& theme);
    const HighlightTheme& theme() const { return m_theme; }

    /// Diagnostics drive the error/warning line backgrounds. Passing diagnostics
    /// only re-tints affected lines; the document is never modified.
    void setDiagnostics(const QVector<Diagnostic>& diagnostics);
    void clearDiagnostics();

protected:
    void highlightBlock(const QString& text) override;

private:
    /// Skip over spans already formatted by an earlier (higher priority) rule.
    bool isProtected(int start, int length) const;

    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
        int captureGroup = 0;  ///< 0 = whole match
        bool wholeLine = false;
    };

    void rebuildRules();

    HighlightTheme m_theme;
    QVector<Rule> m_rules;
    QVector<Diagnostic> m_diagnostics;
};

}  // namespace spall
