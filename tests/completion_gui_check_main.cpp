// completion_gui_check_main.cpp — exercises completion through EditorTab.
//
// The other completion test drives a bare QPlainTextEdit with a GlslCompleter. The
// application instead uses EditorTab, which owns the editor, the highlighter and the
// completer together. Differences there (extra event filters, widget parenting) can
// change whether the popup appears at all, so this test covers the real path.

#include <QAbstractItemView>
#include <QApplication>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QPoint>
#include <QTextCursor>

#include <cmath>
#include <cstdio>

#include "EditorTab.h"
#include "GlslCompleter.h"

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
                    detail.isEmpty() ? ""
                                     : QStringLiteral(" -- %1").arg(detail).toUtf8().constData());
    }
}

/// Put the caret at the end of the given 1-based line.
void caretToLineEnd(QPlainTextEdit* editor, int line) {
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
    cursor.movePosition(QTextCursor::EndOfBlock);
    editor->setTextCursor(cursor);
}

/// Simulate typing `text` one character at a time, the way a user would.
void typeText(QPlainTextEdit* editor, const QString& text) {
    for (const QChar ch : text) {
        QKeyEvent press(QEvent::KeyPress, 0, Qt::NoModifier, QString(ch));
        QApplication::sendEvent(editor, &press);
    }
}

}  // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    std::printf("Completion through EditorTab\n");
    std::printf("============================\n");

    // --- the popup must appear while typing, as in the running application ---
    std::printf("\nTyping in an EditorTab:\n");
    {
        EditorTab tab;
        tab.resize(700, 400);
        tab.show();

        // Start from a skeleton like MainWindow::onNewFile does.
        tab.setSource(QStringLiteral("#version 450\nvoid main()\n{\n    \n}\n"));
        QPlainTextEdit* editor = tab.editor();
        check(editor != nullptr, "EditorTab exposes its editor");
        check(tab.completer() != nullptr, "EditorTab owns a completer");

        caretToLineEnd(editor, 4);
        typeText(editor, QStringLiteral("norma"));

        check(tab.completer()->isPopupVisible(),
              "popup is visible after typing an identifier prefix",
              QStringLiteral("document now: %1").arg(editor->toPlainText()));
    }

    // --- and it must appear for a manual trigger ----------------------------
    std::printf("\nCtrl+Space / manual trigger:\n");
    {
        EditorTab tab;
        tab.resize(700, 400);
        tab.show();
        tab.setSource(QStringLiteral("#version 450\nvoid main()\n{\n    \n}\n"));

        QPlainTextEdit* editor = tab.editor();
        caretToLineEnd(editor, 4);
        tab.completer()->triggerCompletion();
        check(tab.completer()->isPopupVisible(), "manual trigger shows the popup");
    }

    // --- typing after an accepted completion must work again ----------------
    std::printf("\nAfter accepting a candidate:\n");
    {
        EditorTab tab;
        tab.resize(700, 400);
        tab.show();
        tab.setSource(QStringLiteral("#version 450\nvoid main()\n{\n    \n}\n"));

        QPlainTextEdit* editor = tab.editor();
        caretToLineEnd(editor, 4);
        typeText(editor, QStringLiteral("norma"));
        check(tab.completer()->isPopupVisible(), "popup visible before accepting");

        QKeyEvent tabKey(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
        QApplication::sendEvent(editor, &tabKey);
        check(editor->toPlainText().contains(QStringLiteral("normal")),
              "Tab inserted a candidate", editor->toPlainText());
        check(!tab.completer()->isPopupVisible(), "popup closed after Tab");

        // Typing again must re-open it: this is the case that regressed.
        caretToLineEnd(editor, 4);
        editor->textCursor().movePosition(QTextCursor::EndOfBlock);
        typeText(editor, QStringLiteral(" text"));
        check(tab.completer()->isPopupVisible(),
              "popup appears again for a new prefix after accepting",
              QStringLiteral("document now: %1").arg(editor->toPlainText()));
    }

    // --- the popup must be visible AND usable ---------------------------------
    //
    // isPopupVisible() alone is not enough. QCompleter sizes the popup from the rect
    // handed to complete(), so passing the caret rect made a popup 1px wide: it
    // reported itself visible while being impossible to read or click. These checks
    // pin the geometry to something a user can actually use.
    std::printf("\nPopup geometry:\n");
    {
        EditorTab tab;
        tab.resize(900, 500);
        tab.show();
        tab.setSource(QStringLiteral("#version 450\nvoid main()\n{\n    \n}\n"));

        QPlainTextEdit* editor = tab.editor();
        caretToLineEnd(editor, 4);
        typeText(editor, QStringLiteral("norma"));

        check(tab.completer()->isPopupVisible(), "popup visible for 'norma'");
        QAbstractItemView* popup = tab.completer()->popupView();
        if (popup == nullptr) {
            check(false, "the completer exposes a popup view");
        } else {
            check(popup->width() > 60, "popup is wide enough to read",
                  QStringLiteral("width=%1 -- a caret-wide popup means the caret rect "
                                 "was passed to complete()")
                      .arg(popup->width()));
            check(popup->height() > 10, "popup has a usable height",
                  QStringLiteral("height=%1").arg(popup->height()));

            // The popup must sit at the caret: QCompleter anchors it to the rect's
            // bottom-left, so the popup's top-left has to be just below the caret's
            // bottom-left. Getting QRect::bottomLeft() wrong dropped it a whole
            // popup-height down the screen.
            const QPoint caretBottomLeft =
                editor->viewport()->mapToGlobal(editor->cursorRect().bottomLeft());
            const QPoint popupTopLeft = popup->mapToGlobal(QPoint(0, 0));
            const int dx = popupTopLeft.x() - caretBottomLeft.x();
            const int dy = popupTopLeft.y() - caretBottomLeft.y();
            check(std::abs(dx) < 40 && std::abs(dy) < 40,
                  "popup is anchored at the caret, not pushed down the screen",
                  QStringLiteral("popup at (%1,%2), caret bottom-left at (%3,%4) -> dx=%5 dy=%6")
                      .arg(popupTopLeft.x())
                      .arg(popupTopLeft.y())
                      .arg(caretBottomLeft.x())
                      .arg(caretBottomLeft.y())
                      .arg(dx)
                      .arg(dy));
        }
    }

    std::printf("\n============================\n");
    std::printf("%d / %d checks passed\n", g_checks - g_failures, g_checks);
    if (g_failures > 0) {
        std::printf("%d FAILED\n", g_failures);
        return 1;
    }
    std::printf("all passed\n");
    return 0;
}
