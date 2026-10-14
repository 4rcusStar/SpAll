// EditorTab.cpp — the shader editor widget and its per-tab state.
#include "EditorTab.h"

#include <QAbstractTextDocumentLayout>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

#include <algorithm>

#include "GlslCompleter.h"
#include "GlslHighlighter.h"
#include "GlslLanguageData.h"
#include "LineNumberArea.h"

namespace spall {

// --- ShaderEdit --------------------------------------------------------------

ShaderEdit::ShaderEdit(QWidget* parent) : QPlainTextEdit(parent) {
    m_lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged, this,
            &ShaderEdit::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &ShaderEdit::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this,
            &ShaderEdit::rebuildExtraSelections);
    // Monospaced by default; EditorTab can override via applyFont().
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(10);
    font.setStyleHint(QFont::Monospace);
    setFont(font);
    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * m_tabWidth);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    updateLineNumberAreaWidth();
    rebuildExtraSelections();
}

int ShaderEdit::lineNumberAreaWidth() const {
    int digits = 1;
    int maxLines = std::max(1, static_cast<int>(blockCount()));
    while (maxLines >= 10) {
        maxLines /= 10;
        ++digits;
    }
    return 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void ShaderEdit::updateLineNumberAreaWidth() {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ShaderEdit::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy != 0) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth();
    }
}

void ShaderEdit::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ShaderEdit::setDiagnosticSelections(const QVector<Diagnostic>& diagnostics) {
    m_diagnosticSpans = diagnostics;
    rebuildExtraSelections();
}

void ShaderEdit::setDiagnosticLineFormats(const QTextCharFormat& errorFormat,
                                         const QTextCharFormat& warningFormat) {
    m_errorLineFormat = errorFormat;
    m_warningLineFormat = warningFormat;
    rebuildExtraSelections();
}

void ShaderEdit::rebuildExtraSelections() {
    QList<QTextEdit::ExtraSelection> selections;

    // 1. Current line highlight (full width).
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(palette().alternateBase());
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    // 2. Diagnostic line tint plus a wavy underline.
    //
    // Both are ExtraSelections rather than highlighter formats, for two reasons:
    // setFormat() would replace the syntax colours of that line, and a
    // FullWidthSelection is what makes the tint span the whole row instead of only
    // the characters that happen to be on it.
    //
    // These are appended after the current-line highlight, so a diagnostic wins on
    // a line that is also the current line.
    QSet<int> tinted;
    for (const Diagnostic& diag : m_diagnosticSpans) {
        if (!diag.hasPosition() || tinted.contains(diag.line)) {
            continue;
        }
        const QTextBlock block = document()->findBlockByNumber(diag.line - 1);
        if (!block.isValid()) {
            continue;
        }
        tinted.insert(diag.line);

        const bool isError = diag.severity == Severity::Error;

        // 2a. Full-width background with an explicit foreground: the tint must
        // never leave white-on-light text.
        QTextEdit::ExtraSelection tint;
        tint.format = isError ? m_errorLineFormat : m_warningLineFormat;
        tint.format.setProperty(QTextFormat::FullWidthSelection, true);
        tint.cursor = QTextCursor(block);
        tint.cursor.clearSelection();
        selections.append(tint);

        // 2b. Wavy underline over the text itself, without a background so the
        // per-character syntax colours are untouched.
        QTextEdit::ExtraSelection wave;
        wave.format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
        wave.format.setUnderlineColor(isError ? QColor(0xc0, 0x28, 0x28)
                                              : QColor(0xc0, 0x82, 0x00));
        wave.cursor = QTextCursor(block);
        wave.cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        if (wave.cursor.hasSelection()) {
            selections.append(wave);
        }
    }

    setExtraSelections(selections);
}

void ShaderEdit::paintLineNumbers(QPaintEvent* event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), palette().base().color().darker(105));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    const int currentLine = textCursor().blockNumber();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const QString number = QString::number(blockNumber + 1);
            if (blockNumber == currentLine) {
                painter.setPen(palette().text().color());
            } else {
                painter.setPen(palette().text().color().lighter(140));
            }
            painter.drawText(0, top, m_lineNumberArea->width() - 6,
                             fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void ShaderEdit::setIndentSettings(int tabWidth, bool softTabs) {
    m_tabWidth = std::max(1, tabWidth);
    m_softTabs = softTabs;
    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * m_tabWidth);
}

void ShaderEdit::keyPressEvent(QKeyEvent* event) {
    // --- completion acceptance ----------------------------------------------
    //
    // This is the first thing checked, and it has to be here rather than only in the
    // completer's event filter. A visible completion popup takes the keyboard on
    // Windows and passes the key back to this widget via QWidget::event(), which does
    // not run installed event filters — so a filter-only implementation never sees
    // Tab, and the key reaches indentation below instead. Enter is included so both
    // acceptance keys behave the same no matter which path the event takes.
    if (m_completer != nullptr && m_completer->isPopupVisible() &&
        (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab ||
         event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::ShiftModifier)) {
        completionTrace(QStringLiteral("ShaderEdit::keyPressEvent: key=%1 -> accept "
                                       "(editor path, popup was visible)")
                            .arg(event->key()));
        m_completer->acceptCurrentCompletion();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
        completionTrace(QStringLiteral("ShaderEdit::keyPressEvent: Tab, popupVisible=%1 "
                                       "-> falling through to indentation")
                            .arg(m_completer != nullptr && m_completer->isPopupVisible() ? 1 : 0));
    }

    // --- Tab / Backtab indentation ------------------------------------------
    if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::NoModifier) {
        QTextCursor cursor = textCursor();
        if (cursor.hasSelection()) {
            // Indent every selected line.
            const int startBlock = document()->findBlock(cursor.selectionStart()).blockNumber();
            const int endBlock = document()->findBlock(cursor.selectionEnd()).blockNumber();
            cursor.beginEditBlock();
            for (int i = startBlock; i <= endBlock; ++i) {
                QTextBlock block = document()->findBlockByNumber(i);
                QTextCursor lineCursor(block);
                lineCursor.movePosition(QTextCursor::StartOfBlock);
                lineCursor.insertText(QString(m_tabWidth, QLatin1Char(' ')));
            }
            cursor.endEditBlock();
            return;
        }
        cursor.insertText(m_softTabs ? QString(m_tabWidth, QLatin1Char(' '))
                                     : QString(QLatin1Char('\t')));
        return;
    }
    if (event->key() == Qt::Key_Backtab) {
        QTextCursor cursor = textCursor();
        const int startBlock = document()->findBlock(cursor.selectionStart()).blockNumber();
        const int endBlock = document()->findBlock(cursor.selectionEnd()).blockNumber();
        cursor.beginEditBlock();
        for (int i = startBlock; i <= endBlock; ++i) {
            QTextBlock block = document()->findBlockByNumber(i);
            const QString text = block.text();
            int remove = 0;
            while (remove < m_tabWidth && remove < text.size() &&
                   text.at(remove) == QLatin1Char(' ')) {
                ++remove;
            }
            if (remove == 0 && !text.isEmpty() && text.at(0) == QLatin1Char('\t')) {
                remove = 1;
            }
            if (remove > 0) {
                QTextCursor lineCursor(block);
                lineCursor.movePosition(QTextCursor::StartOfBlock);
                for (int r = 0; r < remove; ++r) {
                    lineCursor.deleteChar();
                }
            }
        }
        cursor.endEditBlock();
        return;
    }

    // --- auto-indent ---------------------------------------------------------
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTextCursor cursor = textCursor();
        const QString line = cursor.block().text();
        const int positionInLine = cursor.positionInBlock();

        // Carry over the leading whitespace of the current line.
        QString indent;
        for (const QChar ch : line) {
            if (ch == QLatin1Char(' ') || ch == QLatin1Char('\t')) {
                indent += ch;
            } else {
                break;
            }
        }
        // A line ending in '{' earns one extra level.
        const QString beforeCursor = line.left(positionInLine).trimmed();
        const bool opensBrace = beforeCursor.endsWith(QLatin1Char('{'));
        if (opensBrace) {
            indent += QString(m_tabWidth, QLatin1Char(' '));
        }

        QPlainTextEdit::keyPressEvent(event);

        // If the next character is '}', put it on its own line at the outer level.
        const QString afterCursor = line.mid(positionInLine).trimmed();
        if (opensBrace && afterCursor.startsWith(QLatin1Char('}'))) {
            cursor = textCursor();
            const int save = cursor.position();
            cursor.movePosition(QTextCursor::EndOfBlock);
            cursor.insertText(QStringLiteral("\n") +
                              indent.left(std::max(0, static_cast<int>(indent.size()) - m_tabWidth)));
            cursor.setPosition(save);
            setTextCursor(cursor);
        }
        if (!indent.isEmpty()) {
            cursor = textCursor();
            cursor.insertText(indent);
        }
        return;
    }

    // --- brace auto-close ----------------------------------------------------
    if (event->text() == QLatin1String("}")) {
        // Do not duplicate a brace that is already there.
        QTextCursor cursor = textCursor();
        const int position = cursor.position();
        const QString text = toPlainText();
        if (position < text.size() && text.at(position) == QLatin1Char('}')) {
            cursor.movePosition(QTextCursor::Right);
            setTextCursor(cursor);
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void ShaderEdit::toggleComment() {
    QTextCursor cursor = textCursor();
    const int startBlock = document()->findBlock(cursor.selectionStart()).blockNumber();
    const int endBlock = document()->findBlock(cursor.selectionEnd()).blockNumber();

    // Comment out when any line is uncommented; otherwise uncomment all.
    bool anyUncommented = false;
    for (int i = startBlock; i <= endBlock; ++i) {
        const QString text = document()->findBlockByNumber(i).text().trimmed();
        if (!text.isEmpty() && !text.startsWith(QLatin1String("//"))) {
            anyUncommented = true;
            break;
        }
    }

    cursor.beginEditBlock();
    for (int i = startBlock; i <= endBlock; ++i) {
        QTextBlock block = document()->findBlockByNumber(i);
        QTextCursor lineCursor(block);
        lineCursor.movePosition(QTextCursor::StartOfBlock);
        if (anyUncommented) {
            lineCursor.insertText(QStringLiteral("// "));
        } else {
            const QString text = block.text();
            const int marker = text.indexOf(QLatin1String("//"));
            if (marker >= 0) {
                // Remove "//" plus one following space, if present.
                int remove = 2;
                if (marker + 2 < text.size() && text.at(marker + 2) == QLatin1Char(' ')) {
                    remove = 3;
                }
                for (int r = 0; r < remove; ++r) {
                    lineCursor.deleteChar();
                }
            }
        }
    }
    cursor.endEditBlock();
}

// --- EditorTab ---------------------------------------------------------------

EditorTab::EditorTab(QWidget* parent) : QWidget(parent) {
    m_edit = new ShaderEdit(this);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_edit);

    m_highlighter = new GlslHighlighter(m_edit->document());
    // Seed the diagnostic line formats from the highlighter's default theme, so
    // tints are correct even before applyTheme() is called.
    m_edit->setDiagnosticLineFormats(m_highlighter->theme().errorLine,
                                     m_highlighter->theme().warningLine);
    m_completer = new GlslCompleter(m_edit, this);
    m_completer->attach();
    // The editor handles Tab acceptance itself as well: a visible popup re-delivers
    // keys through QWidget::event(), which skips event filters.
    m_edit->setCompleter(m_completer);

    connect(m_edit->document(), &QTextDocument::modificationChanged, this,
            [this](bool modified) { emit modificationChanged(modified); });
    connect(m_edit, &QPlainTextEdit::textChanged, this, &EditorTab::onTextChanged);
    connect(m_edit, &QPlainTextEdit::cursorPositionChanged, this,
            &EditorTab::onCursorPositionChanged);
    connect(m_completer, &GlslCompleter::parameterHintChanged, this,
            &EditorTab::parameterHintChanged);
}

EditorTab::~EditorTab() = default;

void EditorTab::setFilePath(const QString& path) {
    m_filePath = path;
    m_untitled = path.isEmpty();
    emit titleChanged();
}

QString EditorTab::displayName() const {
    if (m_filePath.isEmpty()) {
        return QCoreApplication::translate("EditorTab", "Untitled");
    }
    const int slash = std::max(m_filePath.lastIndexOf(QLatin1Char('/')),
                               m_filePath.lastIndexOf(QLatin1Char('\\')));
    return slash >= 0 ? m_filePath.mid(slash + 1) : m_filePath;
}

void EditorTab::setStage(ShaderStage stage) {
    m_stage = stage;
    emit titleChanged();
}

QString EditorTab::source() const {
    return m_edit->toPlainText();
}

void EditorTab::setSource(const QString& text) {
    m_edit->setPlainText(text);
    markClean();
    m_stale = true;
}

bool EditorTab::isModified() const {
    return m_edit->document()->isModified();
}

void EditorTab::setModified(bool modified) {
    m_edit->document()->setModified(modified);
}

void EditorTab::markClean() {
    m_edit->document()->setModified(false);
}

void EditorTab::setStale(bool stale) {
    if (m_stale == stale) {
        return;
    }
    m_stale = stale;
    emit titleChanged();
}

void EditorTab::setLastResult(const CompileResult& result) {
    m_result = result;
    m_stale = false;
    setDiagnostics(result.diagnostics);
    emit titleChanged();
}

void EditorTab::clearLastResult() {
    m_result = CompileResult{};
    m_stale = true;
    setDiagnostics({});
    emit titleChanged();
}

void EditorTab::setDiagnostics(const QVector<Diagnostic>& diagnostics) {
    m_diagnostics = diagnostics;
    // Only diagnostics for this file tint this editor's lines.
    QVector<Diagnostic> local;
    for (const Diagnostic& diag : diagnostics) {
        if (diag.file.isEmpty() || m_filePath.isEmpty() || diag.file == m_filePath) {
            local.append(diag);
        }
    }
    m_highlighter->setDiagnostics(local);
    applyExtraSelections();
}

void EditorTab::applyExtraSelections() {
    // Only diagnostics for this file tint this editor's lines; an #include's
    // errors belong to the tab that pulled it in.
    QVector<Diagnostic> local;
    for (const Diagnostic& diag : m_diagnostics) {
        if (!diag.file.isEmpty() && !m_filePath.isEmpty() && diag.file != m_filePath) {
            continue;
        }
        local.append(diag);
    }
    m_edit->setDiagnosticSelections(local);
}

void EditorTab::gotoLine(int line, int column) {
    if (line <= 0) {
        return;
    }
    QTextBlock block = m_edit->document()->findBlockByNumber(line - 1);
    if (!block.isValid()) {
        return;
    }
    QTextCursor cursor(block);
    if (column > 1) {
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column - 1);
    }
    m_edit->setTextCursor(cursor);
    m_edit->centerCursor();
    m_edit->setFocus();
}

int EditorTab::currentLine() const {
    return m_edit->textCursor().blockNumber() + 1;
}

int EditorTab::currentColumn() const {
    return m_edit->textCursor().positionInBlock() + 1;
}

void EditorTab::setReflectedNames(const QStringList& names) {
    m_completer->setReflectedNames(names);
}

void EditorTab::retranslate() {
    // The tab's own visible strings are the completer's candidate descriptions and the
    // parameter hint; the tab label itself is rebuilt by MainWindow::refreshTitles().
    m_completer->retranslate();
    emit titleChanged();
}

void EditorTab::applyTheme(const HighlightTheme& theme) {
    m_highlighter->setTheme(theme);
    // The line tints have to reach ShaderEdit too: it paints them as full-width
    // ExtraSelections so the syntax colours underneath survive.
    m_edit->setDiagnosticLineFormats(theme.errorLine, theme.warningLine);
}

void EditorTab::applyFont(const QString& family, int pointSize) {
    QFont font = m_edit->font();
    if (!family.isEmpty()) {
        font.setFamily(family);
    }
    if (pointSize > 0) {
        font.setPointSize(pointSize);
    }
    m_edit->setFont(font);
    m_edit->setIndentSettings(m_edit->tabWidth(), m_edit->softTabs());
}

void EditorTab::onTextChanged() {
    // Any edit invalidates previously compiled products.
    setStale(true);
}

void EditorTab::onCursorPositionChanged() {
    emit cursorMoved(currentLine(), currentColumn());
}

void EditorTab::updateMargins() {
    // The gutter width is handled inside ShaderEdit, which owns the viewport
    // margins (setViewportMargins is protected on QAbstractScrollArea).
}

void EditorTab::keyPressEvent(QKeyEvent* event) {
    QWidget::keyPressEvent(event);
}

}  // namespace spall
