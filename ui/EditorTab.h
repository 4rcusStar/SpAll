// EditorTab.h — one open shader file: editor, highlighter, completer and state.
//
// Each tab owns its file path, stage, #version, dirty flag, staleness flag and
// the compile products for that file. Nothing shared lives here.
#pragma once

#include <QPlainTextEdit>
#include <QSet>
#include <QString>
#include <QTextCharFormat>
#include <QVector>
#include <QWidget>

#include "CompileOptions.h"
#include "CompileResult.h"
#include "Diagnostic.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QKeyEvent;
QT_END_NAMESPACE

namespace spall {

class GlslCompleter;
class GlslHighlighter;
class LineNumberArea;
struct HighlightTheme;

/// A QPlainTextEdit with a line-number gutter and a highlighted current line.
class ShaderEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit ShaderEdit(QWidget* parent = nullptr);

    /// Width the gutter needs for the current document.
    int lineNumberAreaWidth() const;
    /// Paint the gutter (called by LineNumberArea).
    void paintLineNumbers(QPaintEvent* event);

    /// Tab width in spaces and whether Tab inserts spaces.
    void setIndentSettings(int tabWidth, bool softTabs);
    int tabWidth() const { return m_tabWidth; }
    bool softTabs() const { return m_softTabs; }

    /// Toggle the single-line comment on the selected lines (Ctrl+/).
    void toggleComment();

    /// Set the wave-underlined diagnostic spans. Kept here (rather than in
    /// EditorTab) so the current-line highlight and the diagnostics are applied
    /// as one setExtraSelections() call and neither wipes the other.
    void setDiagnosticSelections(const QVector<Diagnostic>& diagnostics);

    /// Formats used to tint diagnostic lines. They carry both a background and an
    /// explicit foreground so the tinted text always stays readable.
    void setDiagnosticLineFormats(const QTextCharFormat& errorFormat,
                                  const QTextCharFormat& warningFormat);

    /// The gutter, so EditorTab can wire its geometry updates.
    LineNumberArea* lineNumberArea() const { return m_lineNumberArea; }

    /// The completer that owns this editor's popup, set by EditorTab.
    ///
    /// keyPressEvent needs it because a visible completion popup captures the keyboard
    /// on Windows and re-delivers the key to this widget directly — through
    /// QWidget::event(), which never runs an installed event filter. Handling Tab only
    /// in the completer's event filter therefore misses the case that matters, and Tab
    /// falls through to indentation.
    void setCompleter(GlslCompleter* completer) { m_completer = completer; }

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect& rect, int dy);

private:
    /// Rebuild extra selections: current line first, then diagnostics.
    void rebuildExtraSelections();

    LineNumberArea* m_lineNumberArea = nullptr;
    GlslCompleter* m_completer = nullptr;
    int m_tabWidth = 4;
    bool m_softTabs = true;
    QVector<Diagnostic> m_diagnosticSpans;
    QTextCharFormat m_errorLineFormat;
    QTextCharFormat m_warningLineFormat;
};

/// One editor tab: a shader file plus everything derived from it.
class EditorTab : public QWidget {
    Q_OBJECT

public:
    explicit EditorTab(QWidget* parent = nullptr);
    ~EditorTab() override;

    // --- identity -----------------------------------------------------------
    QString filePath() const { return m_filePath; }
    void setFilePath(const QString& path);

    /// Display name: the base file name, or "untitled-N" for new files.
    QString displayName() const;

    ShaderStage stage() const { return m_stage; }
    void setStage(ShaderStage stage);

    // --- document -----------------------------------------------------------
    QString source() const;
    void setSource(const QString& text);

    bool isModified() const;
    void setModified(bool modified);
    /// Mark the document clean (after save/reload).
    void markClean();

    /// True when the source changed since the last successful compile, so the
    /// cached assembly/reflection must not be shown as if it were current.
    bool isStale() const { return m_stale; }
    void setStale(bool stale);

    // --- compile products ---------------------------------------------------
    const CompileResult& lastResult() const { return m_result; }
    void setLastResult(const CompileResult& result);
    void clearLastResult();

    // --- editor access ------------------------------------------------------
    ShaderEdit* editor() const { return m_edit; }
    GlslHighlighter* highlighter() const { return m_highlighter; }
    GlslCompleter* completer() const { return m_completer; }

    /// Apply diagnostics: highlighter tints plus wave underlines.
    void setDiagnostics(const QVector<Diagnostic>& diagnostics);

    /// Move the caret to a 1-based line/column and centre it.
    void gotoLine(int line, int column);
    /// Line number (1-based) of the caret.
    int currentLine() const;
    /// Column number (1-based) of the caret.
    int currentColumn() const;

    /// Feed reflected symbol names into the completer.
    void setReflectedNames(const QStringList& names);

    /// Re-apply every user-visible string from the current catalogue.
    void retranslate();

    /// Switch the editor color scheme.
    void applyTheme(const HighlightTheme& theme);
    /// Apply monospace font family/size.
    void applyFont(const QString& family, int pointSize);

signals:
    /// The document text changed (used to set the dirty marker).
    void modificationChanged(bool modified);
    /// The caret moved; forwards line/column plus the current parameter hint.
    void cursorMoved(int line, int column);
    /// A parameter hint from the completer, for the status bar.
    void parameterHintChanged(const QString& hint);
    /// Emitted when the load/save machinery needs the title refreshed.
    void titleChanged();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onTextChanged();
    void onCursorPositionChanged();
    void updateMargins();

private:
    void applyExtraSelections();

    ShaderEdit* m_edit = nullptr;
    GlslHighlighter* m_highlighter = nullptr;
    GlslCompleter* m_completer = nullptr;
    QString m_filePath;
    ShaderStage m_stage = ShaderStage::Vertex;
    bool m_stale = true;
    bool m_untitled = true;
    CompileResult m_result;
    QVector<Diagnostic> m_diagnostics;
};

}  // namespace spall
