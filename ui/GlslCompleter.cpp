// GlslCompleter.cpp — document scanning, candidate merging and popup handling.
#include "GlslCompleter.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCompleter>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScreen>
#include <QScrollBar>
#include <QSet>
#include <QShortcut>
#include <QStringListModel>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextStream>
#include <QToolTip>

#include <algorithm>

namespace spall {

namespace {

/// Append a line to the trace file created by completionTrace().
void appendTraceLine(const QString& message) {
    QFile file(QCoreApplication::applicationDirPath() +
               QStringLiteral("/spall-completion-trace.log"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << message << QLatin1Char('\n');
}

/// Words that must never be offered as identifiers.
const QSet<QString>& reservedWords() {
    static const QSet<QString> words = [] {
        QSet<QString> set;
        for (const QString& word : GlslLanguageData::allTypes()) {
            set.insert(word);
        }
        for (const QString& word : GlslLanguageData::qualifiers()) {
            set.insert(word);
        }
        for (const QString& word : GlslLanguageData::controlFlow()) {
            set.insert(word);
        }
        return set;
    }();
    return words;
}

/// Extract identifiers declared in `text`: variable/function/struct names and
/// #define macros.
QStringList scanDocumentIdentifiers(const QString& text) {
    QStringList found;
    QSet<QString> seen;

    const auto add = [&found, &seen](const QString& name) {
        if (name.isEmpty() || name.size() < 2) {
            return;
        }
        if (reservedWords().contains(name) || seen.contains(name)) {
            return;
        }
        seen.insert(name);
        found << name;
    };

    // #define NAME
    static const QRegularExpression defineRe(
        QStringLiteral(R"(^\s*#\s*define\s+([A-Za-z_]\w*))"),
        QRegularExpression::MultilineOption);
    auto it = defineRe.globalMatch(text);
    while (it.hasNext()) {
        add(it.next().captured(1));
    }

    // struct NAME
    static const QRegularExpression structRe(QStringLiteral(R"(\bstruct\s+([A-Za-z_]\w*))"));
    it = structRe.globalMatch(text);
    while (it.hasNext()) {
        add(it.next().captured(1));
    }

    // Declarations: a type (possibly with qualifiers) followed by a name.
    static const QRegularExpression declRe(QStringLiteral(
        "(?:^|[;{}()\\s])(?:layout\\s*\\([^)]*\\)\\s*)?"
        "(?:const\\s+|flat\\s+|smooth\\s+|noperspective\\s+|centroid\\s+"
        "|coherent\\s+|readonly\\s+|writeonly\\s+|restrict\\s+|volatile\\s+"
        "|highp\\s+|mediump\\s+|lowp\\s+|precise\\s+|invariant\\s+"
        "|uniform\\s+|buffer\\s+|shared\\s+|in\\s+|out\\s+|inout\\s+"
        "|attribute\\s+|varying\\s+)*"
        "([A-Za-z_]\\w*)\\s+([A-Za-z_]\\w*)\\s*"
        "[;=,\\[)]"));
    it = declRe.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        // Group 2 is the declared name. Group 1 may be a user struct type or a
        // builtin type; both are useful, so add groups 1 and 2.
        add(match.captured(2));
    }

    return found;
}

/// Extract local function definitions with their full signature.
struct LocalFunction {
    QString returnType;
    QString name;
    QString parameters;
};

QVector<LocalFunction> scanLocalFunctions(const QString& text) {
    QVector<LocalFunction> functions;
    // Walk the text manually instead of using a complex regular expression:
    // find "name(" then the matching close paren, and keep it when the next
    // non-space character is '{' or ';'.
    const int length = text.size();
    int i = 0;
    while (i < length) {
        if (text.at(i) != QLatin1Char('(')) {
            ++i;
            continue;
        }
        // Read the identifier immediately before '('.
        int nameEnd = i;
        while (nameEnd > 0 && text.at(nameEnd - 1).isSpace()) {
            --nameEnd;
        }
        int nameStart = nameEnd;
        while (nameStart > 0 && (text.at(nameStart - 1).isLetterOrNumber() ||
                                 text.at(nameStart - 1) == QLatin1Char('_'))) {
            --nameStart;
        }
        if (nameStart == nameEnd) {
            ++i;
            continue;
        }
        // Find the matching ')'.
        int depth = 0;
        int close = -1;
        for (int k = i; k < length; ++k) {
            const QChar ch = text.at(k);
            if (ch == QLatin1Char('(')) {
                ++depth;
            } else if (ch == QLatin1Char(')')) {
                --depth;
                if (depth == 0) {
                    close = k;
                    break;
                }
            }
        }
        if (close < 0) {
            break;
        }
        // The next meaningful character must open a body or end a declaration.
        int next = close + 1;
        while (next < length && text.at(next).isSpace()) {
            ++next;
        }
        const QChar follower = next < length ? text.at(next) : QChar();
        if (follower != QLatin1Char('{') && follower != QLatin1Char(';')) {
            i = close + 1;
            continue;
        }

        const QString name = text.mid(nameStart, nameEnd - nameStart);
        if (name == QLatin1String("if") || name == QLatin1String("for") ||
            name == QLatin1String("while") || name == QLatin1String("switch") ||
            name == QLatin1String("return")) {
            i = close + 1;
            continue;
        }

        // Walk back over the return type: identifiers and whitespace before it.
        int typeStart = nameStart;
        while (typeStart > 0 && text.at(typeStart - 1).isSpace()) {
            --typeStart;
        }
        int typeEnd = typeStart;
        while (typeStart > 0) {
            const QChar ch = text.at(typeStart - 1);
            if (ch.isLetterOrNumber() || ch == QLatin1Char('_') || ch.isSpace()) {
                --typeStart;
            } else {
                break;
            }
        }

        LocalFunction function;
        function.name = name;
        function.returnType = text.mid(typeStart, typeEnd - typeStart).trimmed();
        function.parameters = text.mid(i + 1, close - i - 1).trimmed();
        if (!function.returnType.isEmpty()) {
            functions.append(function);
        }
        i = close + 1;
    }
    return functions;
}

}  // namespace

// Opt-in diagnostic for completion problems. Set SPALL_COMPLETION_TRACE=1 and the
// completion path appends what it decides to spall-completion-trace.log next to the
// executable. SpAll is a GUI-subsystem binary, so a shell frequently cannot read its
// stderr; a file is the only reliable way to observe it on a machine where the popup
// misbehaves. Normal runs write nothing. Declared in GlslCompleter.h so that
// ShaderEdit, which intercepts the same keys, can trace them too.
void completionTrace(const QString& message) {
    static const bool enabled = !qEnvironmentVariableIsEmpty("SPALL_COMPLETION_TRACE");
    if (enabled) {
        appendTraceLine(message);
    }
}

// --- completion model --------------------------------------------------------

/// Flat list model over the merged candidate list.
class CompletionModel : public QAbstractListModel {
public:
    explicit CompletionModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}

    void setItems(QVector<CompletionItem> items) {
        beginResetModel();
        m_items = std::move(items);
        endResetModel();
    }

    const CompletionItem& itemAt(int row) const { return m_items.at(row); }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        return parent.isValid() ? 0 : m_items.size();
    }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
            return {};
        }
        const CompletionItem& item = m_items.at(index.row());
        switch (role) {
            // QCompleter matches the typed prefix against EditRole, and falls back
            // to DisplayRole only when a string list model is used. Omitting
            // EditRole here is what made the popup never match anything and stay
            // hidden.
            case Qt::EditRole:
            case Qt::UserRole:
                return item.name;
            // Shown in the popup: "name(signature)" for functions, "name" otherwise.
            case Qt::DisplayRole:
                return item.displaySignature();
            case Qt::ToolTipRole: {
                QString tip = item.description;
                if (!item.returnType.isEmpty()) {
                    tip = QStringLiteral("%1 %2\n%3").arg(item.returnType, item.displaySignature(),
                                                          item.description);
                }
                return tip;
            }
            // Let the view size columns from the item text as well.
            case Qt::AccessibleTextRole:
                return item.displaySignature();
            default:
                return {};
        }
    }

private:
    QVector<CompletionItem> m_items;
};

// --- GlslCompleter -----------------------------------------------------------

GlslCompleter::GlslCompleter(QPlainTextEdit* editor, QObject* parent)
    : QObject(parent), m_editor(editor) {
    m_model = new CompletionModel(this);
    m_completer = new QCompleter(this);
    m_completer->setModel(m_model);
    m_completer->setWidget(editor);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setWrapAround(true);
    // Match anywhere in the name, not only at the start, so "tex" also finds
    // "textureGrad".
    m_completer->setFilterMode(Qt::MatchContains);
    m_completer->setCompletionColumn(0);
    m_completer->popup()->setAlternatingRowColors(true);

    // The editor and its popup both need to see key presses: the editor so that
    // Tab can accept a completion instead of indenting, the popup so that keys
    // reach the completer rather than being swallowed.
    editor->installEventFilter(this);
    m_completer->popup()->installEventFilter(this);

    // Let Tab reach keyPressEvent instead of being consumed as focus navigation.
    // With the default (true) a QPlainTextEdit never delivers Tab to the widget, so
    // neither the editor's indentation nor this completer would ever see it — focus
    // simply moved into the completion list.
    editor->setTabChangesFocus(false);
}

GlslCompleter::~GlslCompleter() = default;

void GlslCompleter::setAutoTriggerEnabled(bool enabled) {
    m_autoTrigger = enabled;
}

void GlslCompleter::setReflectedNames(const QStringList& names) {
    m_reflectedNames = names;
}

void GlslCompleter::retranslate() {
    // Candidate descriptions are localised. The list is only rebuilt when the document
    // text changes, so invalidating the cached text is what forces refreshCandidates()
    // to re-create the descriptions in the new language on the next trigger.
    m_lastScannedText.clear();
    m_itemByName.clear();
    m_candidates.clear();
    m_lastParameterHint.clear();
}

void GlslCompleter::attach() {
    if (m_editor == nullptr) {
        return;
    }
    connect(m_editor, &QPlainTextEdit::textChanged, this, &GlslCompleter::onTextChanged);
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged, this,
            &GlslCompleter::onCursorPositionChanged);
    connect(m_completer, QOverload<const QModelIndex&>::of(&QCompleter::activated), this,
            &GlslCompleter::onCompletionActivated);

    if (m_shortcut == nullptr) {
        // Ctrl+Space. WidgetWithChildrenShortcut keeps it local to the editor and
        // its popup, so it cannot shadow an application-wide binding.
        m_shortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Space")), m_editor);
        m_shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(m_shortcut, &QShortcut::activated, this, &GlslCompleter::triggerCompletion);
    }
}

void GlslCompleter::onTextChanged() {
    completionTrace(QStringLiteral("onTextChanged: auto=%1 inserting=%2")
                        .arg(m_autoTrigger ? 1 : 0)
                        .arg(m_insertingCompletion ? 1 : 0));
    if (!m_autoTrigger || m_editor == nullptr) {
        return;
    }
    // Inserting an accepted candidate changes the text, which would otherwise pop the
    // list straight back up over the code the user just completed.
    if (m_insertingCompletion) {
        return;
    }
    const QString prefix = prefixUnderCursor();
    completionTrace(QStringLiteral("onTextChanged: prefix='%1' line=%2")
                        .arg(prefix)
                        .arg(m_editor->textCursor().blockNumber()));
    if (prefix.size() < 2) {
        m_completer->popup()->hide();
        return;
    }
    showPopup();
}

void GlslCompleter::onCursorPositionChanged() {
    const QString hint = parameterHintAtCursor();
    if (hint != m_lastParameterHint) {
        m_lastParameterHint = hint;
        emit parameterHintChanged(hint);
    }
    // Close the popup once the cursor leaves the word being completed.
    if (m_completer->popup()->isVisible() && prefixUnderCursor().isEmpty()) {
        m_completer->popup()->hide();
    }
}

QString GlslCompleter::prefixUnderCursor() const {
    if (m_editor == nullptr) {
        return {};
    }
    // The identifier under the caret is read straight out of the line text rather
    // than via QTextCursor::WordUnderCursor. The selection-based version depends on
    // where Qt decides the word boundaries are, and on the caret position still
    // matching the selection it just produced; when those disagree the prefix comes
    // back empty and the popup silently never appears. Scanning the block text is
    // deterministic and needs no layout.
    const QTextCursor cursor = m_editor->textCursor();
    const QString line = cursor.block().text();
    const int position = cursor.positionInBlock();
    if (position < 0 || position > line.size()) {
        return {};
    }

    // Characters that may appear in a GLSL identifier.
    const auto isIdentifierChar = [](QChar ch) {
        return ch.isLetterOrNumber() || ch == QLatin1Char('_');
    };

    int start = position;
    while (start > 0 && isIdentifierChar(line.at(start - 1))) {
        --start;
    }
    int end = position;
    while (end < line.size() && isIdentifierChar(line.at(end))) {
        ++end;
    }
    if (start == end) {
        return {};
    }

    // The caret may be *after* the identifier, which is the normal case while
    // typing, so the character under the caret is allowed to be a non-identifier —
    // but it must not be inside something that is not an identifier at all.
    const QString word = line.mid(start, end - start);
    static const QRegularExpression identRe(QStringLiteral("^[A-Za-z_]\\w*$"));
    return identRe.match(word).hasMatch() ? word : QString();
}

void GlslCompleter::refreshCandidates() {
    const QString text = m_editor != nullptr ? m_editor->toPlainText() : QString();

    // Rebuilding the candidate list resets the model, which would disturb the
    // popup's selection. Skip the work when nothing has changed.
    if (text == m_lastScannedText && !m_candidates.isEmpty()) {
        return;
    }
    m_lastScannedText = text;

    QVector<CompletionItem> candidates;
    QSet<QString> seen;

    const auto addItem = [&candidates, &seen](const CompletionItem& item) {
        if (item.name.isEmpty() || seen.contains(item.name)) {
            return;
        }
        seen.insert(item.name);
        candidates.append(item);
    };

    // 1. reflected names win over raw source scanning.
    if (!m_reflectedNames.isEmpty()) {
        for (const QString& name : m_reflectedNames) {
            CompletionItem item;
            item.name = name;
            item.description = tr("Symbol from the compiled module");
            addItem(item);
        }
    } else {
        for (const QString& name : scanDocumentIdentifiers(text)) {
            CompletionItem item;
            item.name = name;
            item.description = tr("Identifier declared in this document");
            addItem(item);
        }
    }

    // 2. local functions, with signature.
    for (const LocalFunction& function : scanLocalFunctions(text)) {
        CompletionItem item;
        item.name = function.name;
        item.returnType = function.returnType;
        item.signature = QStringLiteral("(%1)").arg(function.parameters);
        item.description = tr("Function defined in this document");
        addItem(item);
    }

    // 3. builtin functions.
    for (const CompletionItem& item : GlslLanguageData::builtinFunctions()) {
        addItem(item);
    }

    // 4. types and keywords.
    for (const CompletionItem& item : GlslLanguageData::typeCompletions()) {
        addItem(item);
    }
    for (const CompletionItem& item : GlslLanguageData::keywordCompletions()) {
        addItem(item);
    }

    m_candidates = candidates;

    // Name lookup used when inserting an accepted candidate. The candidate list
    // itself cannot be indexed by a model row, so acceptance resolves by name.
    m_itemByName.clear();
    m_itemByName.reserve(candidates.size());
    for (const CompletionItem& item : candidates) {
        m_itemByName.insert(item.name, item);
    }

    m_model->setItems(candidates);
}

void GlslCompleter::triggerCompletion() {
    if (m_editor == nullptr) {
        return;
    }
    // Manual trigger. When the caret is inside a word the typed prefix still
    // applies, so Ctrl+Space narrows an existing word; on a blank position there is
    // no prefix and every candidate is offered.
    showPopup();
}

void GlslCompleter::showPopup() {
    if (m_editor == nullptr) {
        return;
    }
    completionTrace(QStringLiteral("showPopup: enter"));

    // A fresh candidate list each time: the document may have changed since the
    // popup was last shown.
    refreshCandidates();

    const QString prefix = prefixUnderCursor();
    m_completer->setCompletionPrefix(prefix);
    completionTrace(QStringLiteral("showPopup: prefix='%1' built=%2 count=%3")
                        .arg(prefix)
                        .arg(m_candidates.size())
                        .arg(m_completer->completionCount()));

    if (m_completer->completionCount() == 0) {
        m_completer->popup()->hide();
        completionTrace(QStringLiteral("showPopup: no candidates -> hide"));
        return;
    }

    // Placement and size hint for QCompleter. QCompleterPrivate::showPopup() takes the
    // popup position from the rect's bottom-left corner and the popup's size from the
    // rect's width and height:
    //
    //     w = rect.width(); rh = rect.height();
    //     pos = widget->mapToGlobal(rect.bottomLeft());
    //     popup->setGeometry(pos.x(), pos.y(), w, h);
    //
    // The rect therefore has to carry the popup's own size while sitting exactly on the
    // caret. Both halves matter and both were wrong before:
    //
    //  * Size — passing the caret rect made the popup as wide as a caret (about 1px),
    //    which is "visible" but unusable. Passing an empty rect anchors the wrong
    //    corner and lets QCompleter stretch the popup across the whole widget.
    //  * Position — QRect::bottomLeft() is (x, y + height - 1), so the rect must be
    //    *positioned* by its bottom-left. Writing QRect(x, y + caretHeight, w, h) puts
    //    the bottom-left a whole popup-height below the caret, which is what dropped
    //    the popup far down the screen.
    const QRect caretRect = m_editor->cursorRect();
    const QAbstractItemView* popup = m_completer->popup();
    const QSize popupSize = popup->sizeHint();
    QRect popupRect(QPoint(0, 0), popupSize);
    popupRect.moveBottomLeft(caretRect.bottomLeft());
    completionTrace(QStringLiteral("showPopup: caretRect=(%1,%2 %3x%4) editorVisible=%5 focus=%6")
                        .arg(caretRect.x())
                        .arg(caretRect.y())
                        .arg(caretRect.width())
                        .arg(caretRect.height())
                        .arg(m_editor->isVisible() ? 1 : 0)
                        .arg(m_editor->hasFocus() ? 1 : 0));
    completionTrace(QStringLiteral("showPopup: popupRect=%1,%2 %3x%4 (bottom-left anchored)")
                        .arg(popupRect.x())
                        .arg(popupRect.y())
                        .arg(popupRect.width())
                        .arg(popupRect.height()));

    m_completer->complete(popupRect);

    // Select the first candidate so Tab/Enter always has a defined target.
    m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
    completionTrace(QStringLiteral("showPopup: done, popupVisible=%1 popupGeom=(%2,%3 %4x%5)")
                        .arg(m_completer->popup()->isVisible() ? 1 : 0)
                        .arg(m_completer->popup()->x())
                        .arg(m_completer->popup()->y())
                        .arg(m_completer->popup()->width())
                        .arg(m_completer->popup()->height()));
}

QString GlslCompleter::completionTextForName(const QString& name) const {
    if (name.isEmpty()) {
        return {};
    }
    const auto it = m_itemByName.constFind(name);
    if (it != m_itemByName.constEnd() && !it.value().signature.isEmpty()) {
        // Insert "name(" for functions so the caret lands inside the parentheses.
        return it.value().name + QLatin1Char('(');
    }
    return name;
}

QString GlslCompleter::highlightedName() const {
    if (m_completer == nullptr) {
        return {};
    }
    // The view's index is what the user sees highlighted, so it wins.
    if (const QAbstractItemView* view = m_completer->popup()) {
        if (QModelIndex index = view->currentIndex(); index.isValid()) {
            if (QString name = nameFromIndex(index); !name.isEmpty()) {
                return name;
            }
        }
    }
    // QCompleter's own current index, which follows setCurrentRow() and the
    // highlighted() signal.
    if (QModelIndex index = m_completer->currentIndex(); index.isValid()) {
        if (QString name = nameFromIndex(index); !name.isEmpty()) {
            return name;
        }
    }
    // Last resort: the first row of the *filtered* model, which is exactly what
    // QCompleter inserts when nothing is highlighted. Returning empty here instead
    // would swallow the key — the popup closes and the document stays unchanged.
    if (QAbstractItemModel* model = m_completer->completionModel()) {
        if (QModelIndex index = model->index(0, 0); index.isValid()) {
            if (QString name = nameFromIndex(index); !name.isEmpty()) {
                return name;
            }
        }
    }
    // Even a model with no usable index means "the top candidate", and the candidate
    // list itself is authoritative.
    for (const CompletionItem& item : m_candidates) {
        if (!item.name.isEmpty()) {
            return item.name;
        }
    }
    return {};
}

QString GlslCompleter::nameFromIndex(const QModelIndex& index) {
    // EditRole carries the bare name; DisplayRole carries "name(signature)".
    QString name = index.data(Qt::EditRole).toString();
    if (name.isEmpty()) {
        name = index.data(Qt::UserRole).toString();
    }
    if (name.isEmpty()) {
        // Fall back to the display string with any "(...)" signature stripped, so a
        // model that only implements DisplayRole still yields a usable name.
        name = index.data(Qt::DisplayRole).toString();
        const int paren = name.indexOf(QLatin1Char('('));
        if (paren > 0) {
            name = name.left(paren);
        }
    }
    return name;
}

bool GlslCompleter::isPopupVisible() const {
    return m_completer != nullptr && m_completer->popup()->isVisible();
}

QAbstractItemView* GlslCompleter::popupView() const {
    return m_completer != nullptr ? m_completer->popup() : nullptr;
}

void GlslCompleter::acceptCurrentCompletion() {
    if (m_editor == nullptr) {
        return;
    }
    if (!isPopupVisible()) {
        completionTrace(QStringLiteral("accept: no popup, nothing to accept"));
        return;
    }

    // Resolution never gives up while the popup is showing a candidate: an empty name
    // used to abort the insertion while the caller had already hidden the popup, so
    // pressing Tab closed the list and changed nothing.
    const QString name = highlightedName();
    completionTrace(QStringLiteral("accept: name='%1' candidates=%2 modelRows=%3")
                        .arg(name)
                        .arg(m_candidates.size())
                        .arg(m_completer->completionCount()));
    if (name.isEmpty()) {
        m_completer->popup()->hide();
        return;
    }

    // Hide before inserting. The insertion modifies the document, which fires
    // textChanged; if the popup were still visible at that point, the auto-trigger
    // would immediately re-open it and it would appear stuck on screen.
    m_completer->popup()->hide();
    insertCompletion(completionTextForName(name));
}

void GlslCompleter::insertCompletion(const QString& insertion) {
    if (m_editor == nullptr || insertion.isEmpty()) {
        return;
    }

    // Guard against the auto-trigger reacting to our own edit.
    m_insertingCompletion = true;

    QTextCursor cursor = m_editor->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    cursor.insertText(insertion);

    m_insertingCompletion = false;

    // For a function, drop the caret between the parentheses and show the hint.
    if (insertion.endsWith(QLatin1Char('('))) {
        const QString hint = parameterHintAtCursor();
        if (!hint.isEmpty()) {
            m_lastParameterHint = hint;
            emit parameterHintChanged(hint);
        }
    }
}

bool GlslCompleter::eventFilter(QObject* watched, QEvent* event) {
    if (m_completer == nullptr || event->type() != QEvent::KeyPress) {
        return QObject::eventFilter(watched, event);
    }
    auto* keyEvent = static_cast<QKeyEvent*>(event);
    completionTrace(QStringLiteral("eventFilter: watched=%1 key=%2 mods=%3 popupVisible=%4 "
                                   "focus=%5")
                        .arg(watched == m_editor ? QStringLiteral("editor")
                                                 : (watched == m_completer->popup()
                                                        ? QStringLiteral("popup")
                                                        : QStringLiteral("other")))
                        .arg(keyEvent->key())
                        .arg(static_cast<int>(keyEvent->modifiers()))
                        .arg(isPopupVisible() ? 1 : 0)
                        .arg(m_editor != nullptr && m_editor->hasFocus() ? 1 : 0));

    // Events delivered directly to the popup are the navigation keys this filter
    // forwarded itself. They must not be handled a second time: the popup's default
    // handler treats Tab as focus navigation, which would move focus into the list
    // instead of accepting a candidate.
    if (watched == m_completer->popup()) {
        return false;
    }

    if (watched != m_editor || !isPopupVisible()) {
        return QObject::eventFilter(watched, event);
    }

    const int key = keyEvent->key();
    const bool noModifier = keyEvent->modifiers() == Qt::NoModifier ||
                            keyEvent->modifiers() == Qt::ShiftModifier;

    // Navigation. The editor keeps the focus while the popup is open, so nothing
    // would move the highlight: arrow keys would move the text caret instead, and
    // accepting would always target the first row. QCompleter forwards these to the
    // popup itself, but only for the widget types it manages (QLineEdit/QComboBox),
    // so it is done here for a QPlainTextEdit. Only navigation is forwarded — never
    // Tab/Enter/Escape, which are acted on below.
    if (noModifier) {
        switch (key) {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown: {
                QKeyEvent forwarded(QEvent::KeyPress, key, keyEvent->modifiers());
                QApplication::sendEvent(m_completer->popup(), &forwarded);
                return true;
            }
            default:
                break;
        }
    }

    // Tab accepts the highlighted candidate. Without this, Tab would be consumed by
    // the editor's indentation handling while the popup is open.
    if ((key == Qt::Key_Tab || key == Qt::Key_Backtab) && noModifier) {
        completionTrace(QStringLiteral("eventFilter: Tab -> accept (filter path)"));
        acceptCurrentCompletion();
        return true;
    }
    // Escape closes the popup; Shift+Escape falls through to the editor.
    if (key == Qt::Key_Escape && noModifier) {
        m_completer->popup()->hide();
        return true;
    }
    // Enter accepts only when the popup is showing, so a plain newline still works
    // whenever no candidate is on screen.
    if ((key == Qt::Key_Return || key == Qt::Key_Enter) &&
        keyEvent->modifiers() == Qt::NoModifier) {
        completionTrace(QStringLiteral("eventFilter: Enter -> accept (filter path)"));
        acceptCurrentCompletion();
        return true;
    }

    return QObject::eventFilter(watched, event);
}

void GlslCompleter::onCompletionActivated(const QModelIndex& index) {
    if (m_editor == nullptr || !index.isValid()) {
        return;
    }
    // `index` belongs to the completion (proxy) model, whose row order differs from
    // the candidate list, so index.row() must never be used to index m_candidates —
    // that is what inserted a seemingly arbitrary fixed entry. Resolve the name and
    // insert that instead.
    const QString name = nameFromIndex(index);
    if (name.isEmpty()) {
        return;
    }
    completionTrace(QStringLiteral("activated: name='%1'").arg(name));
    m_completer->popup()->hide();
    insertCompletion(completionTextForName(name));
}

QString GlslCompleter::debugPrefix() const {
    return m_completer != nullptr ? m_completer->completionPrefix() : QString();
}

int GlslCompleter::debugCurrentRow() const {
    return m_completer != nullptr ? m_completer->currentRow() : -1;
}

QString GlslCompleter::debugFirstMatches(int count) const {
    if (m_completer == nullptr) {
        return {};
    }
    QStringList names;
    const QAbstractItemModel* model = m_completer->completionModel();
    const int rows = std::min(count, m_completer->completionCount());
    for (int row = 0; row < rows; ++row) {
        names << model->index(row, 0).data(Qt::EditRole).toString();
    }
    return names.join(QStringLiteral(", "));
}

QString GlslCompleter::debugCurrentCompletion() const {    // Mirrors what acceptCurrentCompletion() would insert: the highlighted row's
    // name. QCompleter::currentCompletion() is not usable for this because it does
    // not follow arrow-key navigation in this setup.
    if (m_completer == nullptr) {
        return {};
    }
    const QModelIndex index = m_completer->popup()->currentIndex();
    if (!index.isValid()) {
        return {};
    }
    QString name = index.data(Qt::EditRole).toString();
    if (name.isEmpty()) {
        name = index.data(Qt::UserRole).toString();
    }
    return name;
}

int GlslCompleter::debugCandidateCount() const {
    return m_completer != nullptr ? m_completer->completionCount() : -1;
}

QString GlslCompleter::parameterHintAtCursor() const {
    if (m_editor == nullptr) {
        return {};
    }
    QTextCursor cursor = m_editor->textCursor();
    const int position = cursor.position();
    const QString text = m_editor->toPlainText();
    if (position <= 0 || position > text.size()) {
        return {};
    }

    // Walk backwards to find the innermost unclosed '(' before the cursor, and
    // the identifier in front of it.
    int depth = 0;
    int openIndex = -1;
    for (int i = position - 1; i >= 0; --i) {
        const QChar ch = text.at(i);
        if (ch == QLatin1Char(')')) {
            ++depth;
        } else if (ch == QLatin1Char('(')) {
            if (depth == 0) {
                openIndex = i;
                break;
            }
            --depth;
        } else if (ch == QLatin1Char(';') || ch == QLatin1Char('{') || ch == QLatin1Char('}')) {
            return {};  // left the statement without finding a call
        }
    }
    if (openIndex <= 0) {
        return {};
    }

    // Read the identifier immediately before '('.
    int nameEnd = openIndex;
    while (nameEnd > 0 && text.at(nameEnd - 1).isSpace()) {
        --nameEnd;
    }
    int nameStart = nameEnd;
    while (nameStart > 0 && (text.at(nameStart - 1).isLetterOrNumber() ||
                             text.at(nameStart - 1) == QLatin1Char('_'))) {
        --nameStart;
    }
    const QString name = text.mid(nameStart, nameEnd - nameStart);
    if (name.isEmpty()) {
        return {};
    }

    if (const CompletionItem* builtin = GlslLanguageData::findBuiltinFunction(name)) {
        const QString returnType = builtin->returnType.isEmpty() ? QStringLiteral("void")
                                                                 : builtin->returnType;
        return QStringLiteral("%1 %2%3").arg(returnType, builtin->name, builtin->signature);
    }
    return {};
}

}  // namespace spall
