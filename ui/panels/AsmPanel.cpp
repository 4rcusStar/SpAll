// AsmPanel.cpp — assembly rendering, chunked loading and label navigation.
#include "AsmPanel.h"

#include <cstring>

#include <QCheckBox>
#include <QClipboard>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

namespace spall {

namespace {

/// Above this size the text is appended in chunks to keep the UI responsive.
constexpr int kChunkThresholdBytes = 2 * 1024 * 1024;
/// Lines appended per chunk.
constexpr int kChunkLines = 4000;

}  // namespace

// --- SPIR-V assembly highlighter --------------------------------------------

/// Colours Op* mnemonics, %ids, result-type ids and ';' comments.
class SpirvAsmHighlighter : public QSyntaxHighlighter {
public:
    explicit SpirvAsmHighlighter(QTextDocument* document) : QSyntaxHighlighter(document) {
        m_instruction.setForeground(QColor(0x0b, 0x53, 0x94));
        m_instruction.setFontWeight(QFont::Bold);
        m_id.setForeground(QColor(0x00, 0x6b, 0x6b));
        m_typeId.setForeground(QColor(0x9a, 0x3d, 0x00));
        m_comment.setForeground(QColor(0x6a, 0x73, 0x7d));
        m_comment.setFontItalic(true);
        m_literal.setForeground(QColor(0x0b, 0x6b, 0x2e));
    }

    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
        int group = 0;
    };

protected:
    void highlightBlock(const QString& text) override {
        // Build the rule list once.
        static const QVector<Rule> rules = [] {
            QVector<Rule> list;
            SpirvAsmHighlighter probe(nullptr);

            // Trailing '; ...' comment.
            list.append(Rule{QRegularExpression(QStringLiteral(R"(;.*$)")), probe.m_comment, 0});
            // Op* mnemonic at the start of an instruction.
            list.append(Rule{QRegularExpression(QStringLiteral(R"(^\s*%?\w*\s*=?\s*(Op\w+))")),
                             probe.m_instruction, 1});
            // Numeric literals before ids, so "1" is not treated as an id.
            list.append(Rule{QRegularExpression(QStringLiteral(R"(\b\d+\b)")), probe.m_literal});
            // %id operands.
            list.append(Rule{QRegularExpression(QStringLiteral(R"(%\w+)")), probe.m_id});
            return list;
        }();

        for (const Rule& rule : rules) {
            auto it = rule.pattern.globalMatch(text);
            while (it.hasNext()) {
                const QRegularExpressionMatch match = it.next();
                const int start = rule.group > 0 ? match.capturedStart(rule.group)
                                                 : match.capturedStart(0);
                const int length = rule.group > 0 ? match.capturedLength(rule.group)
                                                  : match.capturedLength(0);
                if (start >= 0 && length > 0) {
                    setFormat(start, length, rule.format);
                }
            }
        }
        Q_UNUSED(m_typeId);
    }

private:
    QTextCharFormat m_instruction;
    QTextCharFormat m_id;
    QTextCharFormat m_typeId;
    QTextCharFormat m_comment;
    QTextCharFormat m_literal;
};

// --- AsmPanel ----------------------------------------------------------------

AsmPanel::AsmPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    // --- header row: statistics + options -----------------------------------
    auto* header = new QHBoxLayout();
    m_statsLabel = new QLabel(this);
    m_statsLabel->setTextFormat(Qt::RichText);
    header->addWidget(m_statsLabel);
    header->addStretch(1);

    const auto addOption = [this, header](const QString& text, unsigned flag, const QString& tip,
                                          bool checked) {
        auto* box = new QCheckBox(text, this);
        box->setToolTip(tip);
        box->setChecked(checked);
        connect(box, &QCheckBox::toggled, this, [this, flag](bool on) {
            Q_UNUSED(flag);
            emit disasmFlagsChanged(disasmFlags());
        });
        header->addWidget(box);
        return box;
    };

    m_indentBox = addOption(tr("Indent"), DisasmIndent,
                            QStringLiteral("SPV_BINARY_TO_TEXT_OPTION_INDENT"), true);
    m_friendlyBox = addOption(tr("Friendly names"), DisasmFriendlyNames,
                              QStringLiteral("SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES"), true);
    m_commentBox = addOption(tr("Comments"), DisasmComment,
                             QStringLiteral("SPV_BINARY_TO_TEXT_OPTION_COMMENT"), true);
    m_byteOffsetBox = addOption(tr("Byte offsets"), DisasmShowByteOffset,
                                QStringLiteral("SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET"), false);
    m_nestedIndentBox = addOption(tr("Nested indent"), DisasmNestedIndent,
                                  QStringLiteral("SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT"), false);
    m_reorderBox = addOption(tr("Reorder blocks"), DisasmReorderBlocks,
                             QStringLiteral("SPV_BINARY_TO_TEXT_OPTION_REORDER_BLOCKS"), false);
    m_noHeaderBox = addOption(tr("No header"), DisasmNoHeader,
                              QStringLiteral("SPV_BINARY_TO_TEXT_OPTION_NO_HEADER"), false);

    layout->addLayout(header);

    // --- toolbar row --------------------------------------------------------
    auto* toolbar = new QHBoxLayout();
    m_saveBinaryButton = new QPushButton(tr("Save .spv"), this);
    m_saveBinaryButton->setToolTip(
        tr("Save the compiled SPIR-V binary as a .spv file that spirv-dis / spirv-val "
           "or a Vulkan driver can consume directly"));
    m_saveButton = new QPushButton(tr("Export .spvasm"), this);
    m_saveButton->setToolTip(tr("Save the disassembly text as a .spvasm file"));
    m_copyButton = new QPushButton(tr("Copy All"), this);
    m_findButton = new QPushButton(tr("Find"), this);
    toolbar->addWidget(m_saveBinaryButton);
    toolbar->addWidget(m_saveButton);
    toolbar->addWidget(m_copyButton);
    toolbar->addWidget(m_findButton);
    toolbar->addStretch(1);
    layout->addLayout(toolbar);

    // --- the view -----------------------------------------------------------
    m_view = new QPlainTextEdit(this);
    m_view->setReadOnly(true);
    m_view->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_view->setUndoRedoEnabled(false);
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(10);
    m_view->setFont(font);
    m_highlighter = new SpirvAsmHighlighter(m_view->document());
    layout->addWidget(m_view, 1);

    connect(m_saveButton, &QPushButton::clicked, this, &AsmPanel::saveAs);
    connect(m_saveBinaryButton, &QPushButton::clicked, this, &AsmPanel::saveBinaryAs);
    connect(m_copyButton, &QPushButton::clicked, this, &AsmPanel::copyAll);
    connect(m_findButton, &QPushButton::clicked, this, &AsmPanel::showFindDialog);
    connect(m_view, &QPlainTextEdit::cursorPositionChanged, this,
            &AsmPanel::onCursorPositionChanged);
    // Double-click a branch operand to follow it to the target label. An event
    // filter is used because QPlainTextEdit cannot be subclassed here without
    // losing the built-in highlighter wiring.
    m_view->viewport()->installEventFilter(this);

    setBuiltinPlaceholder(Placeholder::NotCompiled);
    updateStatsLabel();
    setBinary({});
}

void AsmPanel::retranslate() {
    updateStatsLabel();

    // The option tooltips are the literal SPIRV-Tools option names, so they never change.
    m_indentBox->setText(tr("Indent"));
    m_friendlyBox->setText(tr("Friendly names"));
    m_commentBox->setText(tr("Comments"));
    m_byteOffsetBox->setText(tr("Byte offsets"));
    m_nestedIndentBox->setText(tr("Nested indent"));
    m_reorderBox->setText(tr("Reorder blocks"));
    m_noHeaderBox->setText(tr("No header"));

    m_saveBinaryButton->setText(tr("Save .spv"));
    m_saveBinaryButton->setToolTip(
        tr("Save the compiled SPIR-V binary as a .spv file that spirv-dis / spirv-val "
           "or a Vulkan driver can consume directly"));
    m_saveButton->setText(tr("Export .spvasm"));
    m_saveButton->setToolTip(tr("Save the disassembly text as a .spvasm file"));
    m_copyButton->setText(tr("Copy All"));
    m_findButton->setText(tr("Find"));

    // Re-show a panel-owned placeholder in the new language; real assembly, a chunked
    // load, or text set from outside the panel is left untouched.
    if (m_placeholder != Placeholder::None) {
        m_view->setPlainText(placeholderText(m_placeholder));
    }
}

unsigned AsmPanel::disasmFlags() const {
    unsigned flags = DisasmNone;
    if (m_indentBox->isChecked())       flags |= DisasmIndent;
    if (m_friendlyBox->isChecked())     flags |= DisasmFriendlyNames;
    if (m_commentBox->isChecked())      flags |= DisasmComment;
    if (m_byteOffsetBox->isChecked())   flags |= DisasmShowByteOffset;
    if (m_nestedIndentBox->isChecked()) flags |= DisasmNestedIndent;
    if (m_reorderBox->isChecked())      flags |= DisasmReorderBlocks;
    if (m_noHeaderBox->isChecked())     flags |= DisasmNoHeader;
    return flags;
}

void AsmPanel::setDisasmFlags(unsigned flags) {
    // Block signals so restoring settings does not trigger a recompile loop.
    const QSignalBlocker b1(m_indentBox), b2(m_friendlyBox), b3(m_commentBox);
    const QSignalBlocker b4(m_byteOffsetBox), b5(m_nestedIndentBox), b6(m_reorderBox);
    const QSignalBlocker b7(m_noHeaderBox);
    m_indentBox->setChecked(flags & DisasmIndent);
    m_friendlyBox->setChecked(flags & DisasmFriendlyNames);
    m_commentBox->setChecked(flags & DisasmComment);
    m_byteOffsetBox->setChecked(flags & DisasmShowByteOffset);
    m_nestedIndentBox->setChecked(flags & DisasmNestedIndent);
    m_reorderBox->setChecked(flags & DisasmReorderBlocks);
    m_noHeaderBox->setChecked(flags & DisasmNoHeader);
}

QString AsmPanel::assemblyText() const {
    return m_view->toPlainText();
}

void AsmPanel::setStatistics(quint32 words, quint32 instructions, quint32 idBound,
                             const QString& version) {
    m_words = words;
    m_instructions = instructions;
    m_idBound = idBound;
    m_version = version;
    updateStatsLabel();
}

void AsmPanel::updateStatsLabel() {
    m_statsLabel->setText(
        tr("<b>words</b> %1 &nbsp; <b>instructions</b> %2 &nbsp; <b>id bound</b> %3 &nbsp; "
           "<b>SPIR-V</b> %4")
            .arg(m_words)
            .arg(m_instructions)
            .arg(m_idBound)
            .arg(m_version.isEmpty() ? QStringLiteral("-") : m_version));
}

QString AsmPanel::placeholderText(Placeholder kind) {
    switch (kind) {
        case Placeholder::NotCompiled:
            return tr("Nothing compiled yet. Press F7 to compile the current tab.");
        case Placeholder::Stale:
            return tr("Stale, press F7 to recompile");
        case Placeholder::Empty:
            return tr("(the disassembly is empty)");
        case Placeholder::None:
            break;
    }
    return QString();
}

void AsmPanel::setBuiltinPlaceholder(Placeholder kind) {
    // setPlaceholder() marks the text as foreign, so the kind is stored afterwards.
    setPlaceholder(placeholderText(kind));
    m_placeholder = kind;
}

void AsmPanel::setPlaceholder(const QString& text) {
    m_placeholder = Placeholder::None;
    m_pendingLines.clear();
    m_pendingIndex = 0;
    m_view->setPlainText(text);
    m_view->setEnabled(true);
}

void AsmPanel::setAssembly(const QString& text, bool stale) {
    if (stale) {
        setBuiltinPlaceholder(Placeholder::Stale);
        return;
    }
    if (text.isEmpty()) {
        setBuiltinPlaceholder(Placeholder::Empty);
        return;
    }
    loadText(text);
}

void AsmPanel::loadText(const QString& text) {
    m_placeholder = Placeholder::None;
    m_pendingLines.clear();
    m_pendingIndex = 0;

    if (text.size() < kChunkThresholdBytes) {
        m_view->setPlainText(text);
        return;
    }

    // Large module: show a first slice, then keep appending so the event loop
    // stays responsive.
    m_pendingLines = text.split(QLatin1Char('\n'));
    m_view->clear();
    m_view->setPlainText(tr("; Large module (%1 lines), loading in chunks...\n")
                             .arg(m_pendingLines.size()));
    QTimer::singleShot(0, this, &AsmPanel::loadNextChunk);
}

void AsmPanel::loadNextChunk() {
    if (m_pendingIndex == 0) {
        m_view->clear();
    }
    const int end = std::min(m_pendingIndex + kChunkLines, static_cast<int>(m_pendingLines.size()));
    QTextCursor cursor(m_view->document());
    cursor.movePosition(QTextCursor::End);
    for (int i = m_pendingIndex; i < end; ++i) {
        cursor.insertText(m_pendingLines.at(i));
        cursor.insertText(QStringLiteral("\n"));
    }
    m_pendingIndex = end;

    if (m_pendingIndex < m_pendingLines.size()) {
        QTimer::singleShot(0, this, &AsmPanel::loadNextChunk);
    } else {
        m_pendingLines.clear();
        m_pendingIndex = 0;
    }
}

void AsmPanel::showFindDialog() {
    // A simple prompt-based search works regardless of whether the platform
    // provides a built-in find bar.
    bool ok = false;
    const QString needle = QInputDialog::getText(this, tr("Find"), tr("Find in assembly:"),
                                                 QLineEdit::Normal,
                                                 m_view->textCursor().selectedText(), &ok);
    if (!ok || needle.isEmpty()) {
        return;
    }
    const QTextDocument::FindFlags flags;
    if (!m_view->find(needle, flags)) {
        // Wrap around once.
        m_view->moveCursor(QTextCursor::Start);
        if (!m_view->find(needle, flags)) {
            QMessageBox::information(this, tr("Find"), tr("Cannot find \"%1\".").arg(needle));
        }
    }
}

void AsmPanel::copyAll() {
    QGuiApplication::clipboard()->setText(m_view->toPlainText());
}

void AsmPanel::setBinary(const QVector<quint32>& words) {
    m_binary = words;
    if (m_saveBinaryButton != nullptr) {
        m_saveBinaryButton->setEnabled(!words.isEmpty());
    }
}

void AsmPanel::saveBinaryAs() {
    if (m_binary.isEmpty()) {
        QMessageBox::information(
            this, tr("Save SPIR-V"),
            tr("There is no SPIR-V module to save.\n\nCompile successfully with F7 first."));
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        this, tr("Save SPIR-V Binary"), QStringLiteral("shader.spv"),
        tr("SPIR-V binary (*.spv);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }

    // The module is a sequence of 32-bit words in host byte order; SPIR-V binaries
    // are little-endian, which is what the tools and drivers expect on x86/ARM.
    QByteArray bytes;
    bytes.resize(static_cast<int>(m_binary.size() * sizeof(quint32)));
    if (!m_binary.isEmpty()) {
        memcpy(bytes.data(), m_binary.constData(), static_cast<size_t>(bytes.size()));
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(
            this, tr("Save Failed"),
            tr("Cannot write %1: %2\n\nChoose another directory, or check whether the file "
               "is in use.")
                .arg(path, file.errorString()));
        return;
    }
    const qint64 written = file.write(bytes);
    file.close();
    if (written != bytes.size()) {
        QMessageBox::warning(this, tr("Save Failed"),
                             tr("Only %1 of %2 bytes were written; the disk may be full.")
                                 .arg(written)
                                 .arg(bytes.size()));
        return;
    }

    QMessageBox::information(
        this, tr("Save Succeeded"),
        tr("Saved %1 words (%2 bytes) to\n%3\n\n"
           "Verify with:\n"
           "  spirv-val \"%3\"\n"
           "  spirv-dis \"%3\"")
            .arg(m_binary.size())
            .arg(bytes.size())
            .arg(path));
}

void AsmPanel::saveAs() {
    if (m_view->toPlainText().isEmpty()) {
        QMessageBox::information(this, tr("Export Assembly"),
                                 tr("There is no assembly text to export. Press F7 to compile "
                                    "first."));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Export SPIR-V Assembly"), QStringLiteral("shader.spvasm"),
        tr("SPIR-V assembly (*.spvasm);;All files (*)"));
    if (path.isEmpty()) {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Export Failed"),
                             tr("Cannot write %1: %2\n\nChoose another directory, or check "
                                "whether the file is in use.")
                                 .arg(path, file.errorString()));
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << m_view->toPlainText();
}

void AsmPanel::onCursorPositionChanged() {
    // Nothing to do on plain caret movement; label navigation is driven by
    // onDoubleClicked().
}

void AsmPanel::onDoubleClicked() {
    jumpToLabel();
}

bool AsmPanel::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_view->viewport() && event->type() == QEvent::MouseButtonDblClick) {
        // Let the default handler place the caret on the operand first, then jump.
        onDoubleClicked();
    }
    return QWidget::eventFilter(watched, event);
}

/// Jump to the %label named in the current line.
void AsmPanel::jumpToLabel() {
    const QTextCursor cursor = m_view->textCursor();
    const QString line = cursor.block().text();

    // Only act on branch-like instructions; the target is the first %id operand
    // after the mnemonic (e.g. "OpBranch %42" or "OpBranchConditional %c %t %f").
    static const QRegularExpression branchRe(
        QStringLiteral(R"(Op(Branch|BranchConditional|Switch|LoopMerge)\s+(%\w+))"));
    const QRegularExpressionMatch match = branchRe.match(line);
    if (!match.hasMatch()) {
        return;
    }
    const QString label = match.captured(2);

    // Find "%label = OpLabel" in the document.
    QTextCursor search(m_view->document());
    search.movePosition(QTextCursor::Start);
    const QRegularExpression labelRe(QStringLiteral(R"(^\s*%1\s*=\s*OpLabel\b)").arg(QRegularExpression::escape(label)),
                                     QRegularExpression::MultilineOption);
    const QRegularExpressionMatch found = labelRe.match(m_view->toPlainText());
    if (!found.hasMatch()) {
        QMessageBox::information(
            this, tr("Go to Label"),
            tr("No definition found for label %1 in the assembly.").arg(label));
        return;
    }
    QTextCursor target(m_view->document());
    target.setPosition(found.capturedStart(0));
    m_view->setTextCursor(target);
    m_view->centerCursor();
}

}  // namespace spall
