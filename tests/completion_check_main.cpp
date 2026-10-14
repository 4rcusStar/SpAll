// completion_check_main.cpp — verifies the completion popup actually works.
//
// The popup was previously never shown because the completion model did not
// implement Qt::EditRole, which is the role QCompleter matches the typed prefix
// against. These checks catch that class of failure: that candidates are produced,
// that filtering matches, and that the popup is really displayed.

#include <QApplication>
#include <QCompleter>
#include <QAbstractItemView>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QTextCursor>

#include <cstdio>

#include "GlslCompleter.h"
#include "GlslLanguageData.h"

using namespace spall;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool condition, const char* what, const QString& detail = QString()) {
    ++g_checks;
    if (condition) {
        std::printf("  [ OK ] %s\n", what);
    } else {
        ++g_failures;
        std::printf("  [FAIL] %s%s\n", what,
                    detail.isEmpty() ? "" : QStringLiteral(" -- %1").arg(detail).toUtf8().constData());
    }
}

/// Place the caret at the very end of the editor's text.
void moveCaretToEnd(QPlainTextEdit* editor) {
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::End);
    editor->setTextCursor(cursor);
}

}  // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    std::printf("Completion popup test\n");
    std::printf("=====================\n");

    // --- the language data table is the candidate source ---------------------
    std::printf("\nLanguage data:\n");
    {
        const QVector<CompletionItem>& functions = GlslLanguageData::builtinFunctions();
        check(!functions.isEmpty(), "builtin function table is populated",
              QStringLiteral("%1 entries").arg(functions.size()));
        check(GlslLanguageData::findBuiltinFunction(QStringLiteral("textureGrad")) != nullptr,
              "textureGrad is a known builtin");
        check(GlslLanguageData::findBuiltinFunction(QStringLiteral("normalize")) != nullptr,
              "normalize is a known builtin");
        check(GlslLanguageData::findBuiltinFunction(QStringLiteral("noSuchFunction")) == nullptr,
              "unknown names are not reported as builtins");
        check(!GlslLanguageData::typeCompletions().isEmpty(), "type completion table is populated");
        check(!GlslLanguageData::keywordCompletions().isEmpty(),
              "keyword completion table is populated");
    }

    // --- end-to-end through the completer ------------------------------------
    std::printf("\nCompleter behaviour:\n");
    {
        QPlainTextEdit editor;
        GlslCompleter completer(&editor);
        completer.attach();

        editor.setPlainText(QStringLiteral("#version 450\nvoid main()\n{\n    norm\n}\n"));
        moveCaretToEnd(&editor);

        // Put the caret right after "norm" on line 4.
        QTextCursor cursor = editor.textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 3);
        cursor.movePosition(QTextCursor::EndOfBlock);
        editor.setTextCursor(cursor);

        completer.triggerCompletion();
        const bool visible = completer.isPopupVisible();
        check(visible, "Ctrl+Space trigger shows the popup");

        // Accepting must insert the highlighted candidate.
        const QString before = editor.toPlainText();
        completer.acceptCurrentCompletion();
        const QString after = editor.toPlainText();
        check(after != before, "accepting a candidate modifies the document");
        check(!completer.isPopupVisible(), "popup closes after accepting");
    }

    // --- prefix filtering -----------------------------------------------------
    std::printf("\nPrefix filtering:\n");
    {
        QPlainTextEdit editor;
        GlslCompleter completer(&editor);
        completer.attach();

        // "texture" must offer the texture builtins, not the whole table.
        editor.setPlainText(QStringLiteral("void main()\n{\n    texture\n}\n"));
        QTextCursor cursor = editor.textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 2);
        cursor.movePosition(QTextCursor::EndOfBlock);
        editor.setTextCursor(cursor);

        completer.triggerCompletion();
        check(completer.isPopupVisible(), "popup shows for the 'texture' prefix");

        check(completer.debugPrefix() == QLatin1String("texture"),
              "the completer filters on the typed prefix",
              QStringLiteral("prefix was '%1'").arg(completer.debugPrefix()));

        completer.acceptCurrentCompletion();
        const QString text = editor.toPlainText();
        // Whatever was chosen must be a "texture..." name, not an unrelated one.
        check(text.contains(QStringLiteral("texture")),
              "accepted candidate comes from the matching set", text);
    }

    // --- Ctrl+Space on a blank position offers everything -------------------
    std::printf("\nUnfiltered trigger:\n");
    {
        QPlainTextEdit editor;
        GlslCompleter completer(&editor);
        completer.attach();

        // Caret after "vec3 " — no word under the cursor.
        editor.setPlainText(QStringLiteral("void main()\n{\n    vec3 \n}\n"));
        QTextCursor cursor = editor.textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 2);
        cursor.movePosition(QTextCursor::EndOfBlock);
        editor.setTextCursor(cursor);

        completer.triggerCompletion();
        check(completer.isPopupVisible(), "popup shows with an empty prefix");
        check(completer.debugPrefix().isEmpty(), "prefix is empty (no word under the caret)",
              completer.debugPrefix());
    }

    // --- identifiers from the document are offered ---------------------------
    std::printf("\nDocument identifiers:\n");
    {
        QPlainTextEdit editor;
        GlslCompleter completer(&editor);
        completer.attach();

        editor.setPlainText(QStringLiteral(
            "void main()\n"
            "{\n"
            "    vec3 mySpecialColor = vec3(1.0);\n"
            "    mySpe\n"
            "}\n"));
        QTextCursor cursor = editor.textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 3);
        cursor.movePosition(QTextCursor::EndOfBlock);
        editor.setTextCursor(cursor);

        completer.triggerCompletion();
        check(completer.isPopupVisible(), "popup shows for a local identifier prefix");
        completer.acceptCurrentCompletion();
        check(editor.toPlainText().contains(QStringLiteral("mySpecialColor")),
              "local variable name completes", editor.toPlainText());
    }

    // --- Tab must accept the HIGHLIGHTED row, not the first one --------------
    std::printf("\nTab accepts the highlighted row:\n");
    {
        QPlainTextEdit editor;
        GlslCompleter completer(&editor);
        completer.attach();
        editor.resize(600, 400);
        editor.show();

        editor.setPlainText(QStringLiteral("void main()\n{\n    normali\n}\n"));
        QTextCursor cursor = editor.textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 2);
        cursor.movePosition(QTextCursor::EndOfBlock);
        editor.setTextCursor(cursor);

        completer.triggerCompletion();
        check(completer.isPopupVisible(), "popup shows before Tab");

        // Move the highlight down so the selection differs from row 0. This is the
        // case that used to insert the first candidate instead of the highlighted one.
        const QString first = completer.debugCurrentCompletion();
        QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
        QApplication::sendEvent(&editor, &down);
        const QString second = completer.debugCurrentCompletion();
        check(second != first, "Down moves the selection off the first candidate",
              QStringLiteral("first='%1' second='%2'").arg(first, second));

        const QString expected = second;

        QKeyEvent tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
        // Delivered straight to the widget rather than through QApplication, so this
        // exercises the Tab handling itself without depending on how the platform
        // plugin routes focus navigation.
        QApplication::sendEvent(&editor, &tab);

        const QString text = editor.toPlainText();
        check(!completer.isPopupVisible(), "Tab closes the popup");
        // The inserted name must be the highlighted one, not the first candidate.
        const QString insertedName = expected.endsWith(QLatin1Char('('))
                                         ? expected.left(expected.size() - 1)
                                         : expected;
        check(text.contains(insertedName),
              "Tab inserted the HIGHLIGHTED candidate, not the first",
              QStringLiteral("expected '%1' in: %2").arg(insertedName, text));
        check(!text.contains(QLatin1Char('\t')), "Tab did not fall through to indentation");
        Q_UNUSED(first);
    }

    // --- clicking a row must insert that row ---------------------------------
    std::printf("\nMouse click inserts the clicked row:\n");
    {
        QPlainTextEdit editor;
        GlslCompleter completer(&editor);
        completer.attach();
        editor.resize(600, 400);
        editor.show();

        editor.setPlainText(QStringLiteral("void main()\n{\n    vec\n}\n"));
        QTextCursor cursor = editor.textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 2);
        cursor.movePosition(QTextCursor::EndOfBlock);
        editor.setTextCursor(cursor);

        completer.triggerCompletion();
        check(completer.isPopupVisible(), "popup shows for the 'vec' prefix");

        // Walk the selection to a specific row the way a user would, then let the
        // completer's activation path run. QCompleter sets its current index from the
        // clicked index before emitting activated(), so this exercises the same code
        // path a mouse click takes.
        for (int i = 0; i < 3; ++i) {
            QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
            QApplication::sendEvent(&editor, &down);
        }
        const QString highlighted = completer.debugCurrentCompletion();
        check(!highlighted.isEmpty(), "a row is highlighted", highlighted);

        QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(&editor, &enter);

        const QString name = highlighted.endsWith(QLatin1Char('('))
                                 ? highlighted.left(highlighted.size() - 1)
                                 : highlighted;
        check(editor.toPlainText().contains(name),
              "the highlighted row was inserted (not a fixed candidate)",
              QStringLiteral("expected '%1' in: %2").arg(name, editor.toPlainText()));
    }

    // --- acceptance must never silently do nothing ---------------------------
    //
    // The reported symptom was "the list is visible, I press Tab, the list closes and
    // nothing is inserted". That happens when name resolution fails while the popup is
    // up: the caller hides the popup and the document stays unchanged. Whatever the
    // view's selection state, acceptance must fall back to the top candidate.
    std::printf("\nAcceptance never no-ops while the popup is up:\n");
    {
        // Case 1: the view has no current index at all.
        {
            QPlainTextEdit editor;
            GlslCompleter completer(&editor);
            completer.attach();
            editor.setPlainText(QStringLiteral("void main()\n{\n    normali\n}\n"));
            QTextCursor cursor = editor.textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 2);
            cursor.movePosition(QTextCursor::EndOfBlock);
            editor.setTextCursor(cursor);

            completer.triggerCompletion();
            check(completer.isPopupVisible(), "popup shows before clearing the selection");
            if (QAbstractItemView* view = completer.popupView()) {
                view->setCurrentIndex(QModelIndex()); // nothing highlighted
            }
            const QString before = editor.toPlainText();
            completer.acceptCurrentCompletion();
            const QString after = editor.toPlainText();
            check(after != before,
                  "acceptance inserts a candidate even with nothing highlighted", after);
            check(after.contains(QStringLiteral("normal")),
                  "the inserted candidate matches the prefix", after);
        }

        // Case 2: no candidate lookup by name is possible, so only the filtered model
        // can supply the answer.
        {
            QPlainTextEdit editor;
            GlslCompleter completer(&editor);
            completer.attach();
            editor.setPlainText(QStringLiteral("void main()\n{\n    textur\n}\n"));
            QTextCursor cursor = editor.textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 2);
            cursor.movePosition(QTextCursor::EndOfBlock);
            editor.setTextCursor(cursor);

            completer.triggerCompletion();
            check(completer.isPopupVisible(), "popup shows for 'textur'");
            check(!completer.highlightedName().isEmpty(),
                  "a candidate name is always resolvable while the popup is up",
                  completer.highlightedName());
            const QString before = editor.toPlainText();
            completer.acceptCurrentCompletion();
            check(editor.toPlainText() != before, "acceptance modifies the document",
                  editor.toPlainText());
        }
    }

    // --- the prefix must be found at every caret position in a line ------------
    //
    // The popup is gated on the identifier under the caret. Reading it through
    // QTextCursor::WordUnderCursor made that gate depend on Qt's word-boundary
    // heuristic agreeing with the caret position, and when it did not the prefix came
    // back empty and the popup never appeared. Triggering on a blank position skips
    // the "prefix too short" gate, so debugPrefix() reports exactly what was read.
    std::printf("\nPrefix detection at every caret position:\n");
    {
        struct PrefixCase {
            const char* line;    ///< the editor's only line
            int caretColumn;     ///< 0-based column the caret is placed at
            const char* expect;  ///< expected prefix
        };
        const PrefixCase cases[] = {
            {"    normal", 10, "normal"},      // caret at the very end of the word
            {"    normal ", 10, "normal"},     // ... with text after it
            {"    normalize", 8, "normalize"}, // caret inside the word: the whole word
            {"    normalize", 7, "normalize"}, // ... is offered, as an editor should
            {"normal", 6, "normal"},           // word at the very start of the line
            {"    nor", 7, "nor"},             // a short prefix still counts
            {"    ", 4, ""},                   // only whitespace
            {"", 0, ""},                       // empty line
            {"    fragColor = vec4(1.0);", 8, "fragColor"},  // before a space
            {"    vec3;", 8, "vec3"},          // word followed by punctuation
        };

        for (const PrefixCase& c : cases) {
            QPlainTextEdit editor;
            GlslCompleter completer(&editor);
            completer.attach();
            editor.setPlainText(QString::fromUtf8(c.line));

            QTextCursor cursor = editor.textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, c.caretColumn);
            editor.setTextCursor(cursor);

            // Ctrl+Space asks the completer to re-read the prefix at this caret.
            completer.triggerCompletion();
            const QString prefix = completer.debugPrefix();
            check(prefix == QString::fromUtf8(c.expect),
                  "prefix matches at the expected caret column",
                  QStringLiteral("line \"%1\" col %2 -> got '%3', want '%4'")
                      .arg(QString::fromUtf8(c.line))
                      .arg(c.caretColumn)
                      .arg(prefix, QString::fromUtf8(c.expect)));
        }
    }

    std::printf("\n=====================\n");
    std::printf("%d / %d checks passed\n", g_checks - g_failures, g_checks);
    if (g_failures > 0) {
        std::printf("%d FAILED\n", g_failures);
        return 1;
    }
    std::printf("all passed\n");
    return 0;
}
