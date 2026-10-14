// AsmPanel.h — SPIR-V assembly view.
//
// Read-only text with light SPIR-V highlighting, find, save-as, a statistics
// header row, and double-click navigation from OpBranch %label to its OpLabel.
// Very large modules are appended incrementally so the UI never blocks on a
// single setPlainText() call.
#pragma once

#include <QWidget>

#include "CompileOptions.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTextCursor;
QT_END_NAMESPACE

namespace spall {

class SpirvAsmHighlighter;

class AsmPanel : public QWidget {
    Q_OBJECT

public:
    explicit AsmPanel(QWidget* parent = nullptr);

    /// Replace the contents. `stale` shows the "press F7" notice instead.
    void setAssembly(const QString& text, bool stale);
    /// Show a placeholder message (no compile yet, or a failure).
    void setPlaceholder(const QString& text);

    /// The compiled module, offered to the save actions. Kept here because the
    /// assembly panel is what the .spv / .spvasm actions operate on.
    void setBinary(const QVector<quint32>& words);
    bool hasBinary() const { return !m_binary.isEmpty(); }

    /// Header statistics: word count, instruction count, id bound, version.
    void setStatistics(quint32 words, quint32 instructions, quint32 idBound,
                       const QString& version);

    /// Disassembly option checkboxes, as option bits.
    unsigned disasmFlags() const;
    void setDisasmFlags(unsigned flags);

    QString assemblyText() const;

    /// Re-apply every user-visible string from the current catalogue.
    void retranslate();

signals:
    /// Emitted when a disassembly option changes, so the owner can re-disassemble.
    void disasmFlagsChanged(unsigned flags);

public slots:
    void showFindDialog();
    /// Save the assembly as text (.spvasm).
    void saveAs();
    /// Save the compiled module as a binary .spv file.
    void saveBinaryAs();
    void copyAll();

protected:
    /// Watches the text view for double-clicks (label navigation).
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onCursorPositionChanged();
    /// Double-click handler: follow an OpBranch operand to its OpLabel.
    void onDoubleClicked();

private:
    /// Which of the panel's own placeholder texts the view currently shows, so that
    /// retranslate() can refresh it without discarding real assembly.
    enum class Placeholder {
        None,         ///< real assembly, a chunked load, or text set from outside
        NotCompiled,  ///< nothing has been compiled yet
        Stale,        ///< the module is older than the current source
        Empty,        ///< the disassembly came back empty
    };

    /// Load `text`, chunking when it is very large.
    void loadText(const QString& text);
    /// Continue the chunked load.
    void loadNextChunk();
    /// Jump from an OpBranch operand to the matching %label definition.
    void jumpToLabel();
    void updateStatsLabel();
    /// Show one of the panel's own placeholders and remember which one.
    void setBuiltinPlaceholder(Placeholder kind);
    /// The translated text of a panel-owned placeholder.
    QString placeholderText(Placeholder kind);

    QPlainTextEdit* m_view = nullptr;
    SpirvAsmHighlighter* m_highlighter = nullptr;
    QLabel* m_statsLabel = nullptr;
    QCheckBox* m_indentBox = nullptr;
    QCheckBox* m_friendlyBox = nullptr;
    QCheckBox* m_commentBox = nullptr;
    QCheckBox* m_noHeaderBox = nullptr;
    QCheckBox* m_byteOffsetBox = nullptr;
    QCheckBox* m_nestedIndentBox = nullptr;
    QCheckBox* m_reorderBox = nullptr;
    QPushButton* m_saveButton = nullptr;
    QPushButton* m_saveBinaryButton = nullptr;
    QPushButton* m_copyButton = nullptr;
    QPushButton* m_findButton = nullptr;

    /// The compiled module, so it can be written out as a .spv file.
    QVector<quint32> m_binary;

    /// Pending chunked load.
    QStringList m_pendingLines;
    int m_pendingIndex = 0;

    quint32 m_words = 0;
    quint32 m_instructions = 0;
    quint32 m_idBound = 0;
    QString m_version;

    /// Set while the view shows a panel-owned placeholder rather than a module.
    Placeholder m_placeholder = Placeholder::None;
};

}  // namespace spall
