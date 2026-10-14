// GlslCompleter.h — completion model and popup handling for one editor.
//
// Candidates are merged from four sources, in priority order:
//   1. identifiers in the current document (variables, functions, structs,
//      #define macros, layout in/out names)
//   2. local functions, shown with their full signature
//   3. the builtin function table (GlslLanguageData)
//   4. types and keywords
//
// When reflection data is available it replaces source (1), since the compiled
// names are exact.
#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QVector>

#include "GlslLanguageData.h"
#include "ReflectData.h"

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QAbstractItemView;
class QCompleter;
class QPlainTextEdit;
class QShortcut;
class QStringListModel;
QT_END_NAMESPACE
namespace spall {

class CompletionModel;

/// Append a diagnostic line to spall-completion-trace.log, but only when the
/// SPALL_COMPLETION_TRACE environment variable is set. SpAll is a GUI-subsystem binary
/// that a shell often cannot read stderr from, so a file is the only reliable way to
/// observe the completion path on a machine where it misbehaves. Defined in
/// GlslCompleter.cpp; also used by ShaderEdit, which intercepts the same keys.
void completionTrace(const QString& message);

class GlslCompleter : public QObject {
    Q_OBJECT

public:
    /// `editor` is the QPlainTextEdit whose document is completed. It is not
    /// owned by the completer.
    explicit GlslCompleter(QPlainTextEdit* editor, QObject* parent = nullptr);
    ~GlslCompleter() override;

    /// Enable/disable the automatic popup while typing.
    void setAutoTriggerEnabled(bool enabled);
    bool autoTriggerEnabled() const { return m_autoTrigger; }

    /// Feed names discovered by the compiler so completions use real symbols.
    /// Passing a default-constructed value clears them.
    void setReflectedNames(const QStringList& names);

    /// Drop the cached candidate list so the next trigger rebuilds its descriptions in
    /// the current language.
    void retranslate();

    /// Show the popup for the identifier prefix under the cursor.
    void triggerCompletion();

    /// Install the shortcut and connections. Called from the owning widget so
    /// the editor keeps ownership of its own key handling.
    void attach();

    /// If the cursor sits inside a call to a known builtin function, return that
    /// function's signature for the status bar; otherwise an empty string.
    QString parameterHintAtCursor() const;

    /// The text to insert for an accepted candidate name: "name(" for a function
    /// so the caret lands inside the parentheses, plain "name" otherwise.
    ///
    /// Resolution is by name because the popup's model is QCompleter's proxy model,
    /// whose row order does not match the candidate list.
    QString completionTextForName(const QString& name) const;

    /// True while the candidate popup is on screen.
    bool isPopupVisible() const;

    /// The popup view, so callers and tests can check where and how large it is.
    /// Never null once a completion has been requested; not owned by the caller.
    QAbstractItemView* popupView() const;

    /// Insert the highlighted candidate (used by Enter and Tab).
    void acceptCurrentCompletion();

    /// The name acceptance would insert right now: the view's highlighted row, then
    /// QCompleter's current index, then the first matching row. Never returns empty
    /// while the popup is showing a candidate, because an empty result would swallow
    /// the acceptance key and leave the document unchanged.
    QString highlightedName() const;

    /// Replace the word under the cursor with `insertion` and place the caret
    /// inside the parentheses when a function was inserted.
    void insertCompletion(const QString& insertion);

    // --- diagnostics for tests -------------------------------------------------
    /// The prefix the completer is currently filtering on.
    QString debugPrefix() const;
    /// The completer's notion of the highlighted row.
    int debugCurrentRow() const;
    /// The first `count` matching names, comma separated.
    QString debugFirstMatches(int count) const;
    /// The name the completer would insert right now.
    QString debugCurrentCompletion() const;
    /// How many candidates the completer is currently offering.
    int debugCandidateCount() const;

    /// Watches the editor and the popup: Tab/Enter accept, Escape cancels.
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    /// Emitted as the cursor moves, with a signature like
    /// "void textureGrad(sampler2D, vec2, vec2, vec2)" or an empty string.
    void parameterHintChanged(const QString& hint);

private slots:
    void onTextChanged();
    void onCursorPositionChanged();
    void onCompletionActivated(const QModelIndex& index);

private:
    /// Rebuild the candidate list from the document text.
    void refreshCandidates();

    /// Extract a candidate name from a completion-model index, tolerating a model that
    /// only implements DisplayRole.
    static QString nameFromIndex(const QModelIndex& index);

    /// The identifier immediately before the cursor, or empty.
    QString prefixUnderCursor() const;

    void showPopup();

    QPlainTextEdit* m_editor = nullptr;
    QCompleter* m_completer = nullptr;
    QShortcut* m_shortcut = nullptr;
    CompletionModel* m_model = nullptr;
    QVector<CompletionItem> m_candidates;
    /// Candidate lookup by name, used when inserting an accepted item.
    QHash<QString, CompletionItem> m_itemByName;
    QStringList m_reflectedNames;
    bool m_autoTrigger = true;
    /// True while a completion is being inserted, so the resulting textChanged does
    /// not immediately re-open the popup.
    bool m_insertingCompletion = false;
    /// Document text the current candidate list was built from, so typing can skip
    /// a needless model reset.
    QString m_lastScannedText;
    QString m_lastParameterHint;
};

}  // namespace spall
