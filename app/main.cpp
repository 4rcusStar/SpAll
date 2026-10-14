// main.cpp — SpAll application entry point.
//
// Owns the glslang process lifetime: InitializeProcess() before any window is
// created, FinalizeProcess() after the event loop returns.
//
// `SpAll --selftest` opens the real window, types a prefix into the live editor and
// reports whether the completion popup appeared, then exits with a non-zero code if
// it did not. It exists because the completion path depends on widget wiring that a
// standalone test cannot reproduce: the popup is opened from the editor's
// textChanged, on a window that is really on screen. Set SPALL_COMPLETION_TRACE=1 as
// well to get a step-by-step log next to the executable.

#include <QApplication>
#include <QMessageBox>
#include <QTimer>

#include "GlslCompiler.h"
#include "MainWindow.h"
#include "StartupTrace.h"
#include "Translator.h"

int main(int argc, char** argv) {
    spall::startupTrace("main: enter");
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("SpAll"));
    QCoreApplication::setApplicationName(QStringLiteral("SpAll"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    spall::startupTrace("main: QApplication constructed");

    // glslang is a process-global singleton and must be up before any compile.
    if (!spall::GlslCompiler::initialize()) {
        QMessageBox::critical(
            nullptr, QCoreApplication::translate("main", "Startup Failed"),
            QCoreApplication::translate(
                "main",
                "glslang could not be initialized, so the compiler is unavailable.\n\n"
                "Restart the application; if the problem persists, check that the "
                "installation is complete (glslang static libraries)."));
        return 1;
    }
    spall::startupTrace("main: glslang initialized");

    const bool selfTest = app.arguments().contains(QStringLiteral("--selftest"));

    // `--lang=<code>` overrides the saved UI language for this run, so the interface can
    // be exercised in another language without touching persisted settings. Useful for
    // screenshots, review and automated checks.
    for (const QString& argument : app.arguments()) {
        if (!argument.startsWith(QLatin1String("--lang="))) {
            continue;
        }
        const QString code = argument.mid(7);
        for (const spall::LanguageInfo& info : spall::Translator::available()) {
            if (code == QLatin1String(info.code)) {
                spall::MainWindow::setLanguageOverride(code);
                break;
            }
        }
    }
    spall::startupTrace("main: arguments parsed");

    int exitCode = 0;
    {
        spall::MainWindow window;
        spall::startupTrace("main: MainWindow constructed");
        window.show();
        spall::startupTrace("main: window shown");

        if (selfTest) {
            // Run once the window has been shown and laid out, so the editor has a
            // real geometry and the popup can be placed.
            QTimer::singleShot(0, &window, [&window, &exitCode, &app]() {
                spall::startupTrace("main: self-test starting");
                exitCode = window.runCompletionSelfTest();
                spall::startupTrace("main: self-test finished");
                app.quit();
            });
        }

        const int eventLoopResult = app.exec();
        spall::startupTrace("main: event loop returned");
        if (!selfTest) {
            exitCode = eventLoopResult;
        }
    }

    spall::GlslCompiler::finalize();
    spall::startupTrace("main: exiting");
    return exitCode;
}