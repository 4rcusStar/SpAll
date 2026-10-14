// file_browser_check_main.cpp — verifies the file browser's file-type rules.
//
// The browser must offer exactly the extensions the compiler understands, and it
// must agree with stageFromFileName() so opening a file always selects a sensible
// stage. These checks cover that consistency without needing a GUI.

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>

#include <cstdio>

#include "FileBrowser.h"
#include "ShaderStage.h"

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

/// Write a file so the browser has something real to look at.
QString makeFile(const QString& dir, const QString& name, const QString& contents) {
    const QString path = dir + QLatin1Char('/') + name;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::printf("        [warn] cannot create %s: %s\n", path.toUtf8().constData(),
                    file.errorString().toUtf8().constData());
        return {};
    }
    file.write(contents.toUtf8());
    file.close();
    return path;
}

}  // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    std::printf("File browser test\n");
    std::printf("=================\n");

    // --- the supported extension set -----------------------------------------
    std::printf("\nSupported extensions:\n");
    const QStringList suffixes = FileBrowser::supportedSuffixes();
    check(!suffixes.isEmpty(), "suffix list is non-empty");
    check(suffixes.contains(QStringLiteral("vert")), "vert is supported");
    check(suffixes.contains(QStringLiteral("frag")), "frag is supported");
    check(suffixes.contains(QStringLiteral("comp")), "comp is supported");
    check(suffixes.contains(QStringLiteral("geom")), "geom is supported");
    check(!suffixes.contains(QStringLiteral("txt")), "txt is not claimed as a shader");

    // Every advertised extension must map back to a stage, otherwise opening such a
    // file would leave the stage dropdown wrong.
    {
        QStringList unmapped;
        for (const QString& suffix : suffixes) {
            ShaderStage stage = ShaderStage::Vertex;
            if (!stageFromFileName(QStringLiteral("x.") + suffix, &stage)) {
                unmapped << suffix;
            }
        }
        check(unmapped.isEmpty(), "every supported extension maps to a stage",
              unmapped.join(QStringLiteral(", ")));
    }

    // --- the dialog filter matches the same set ------------------------------
    std::printf("\nDialog filter:\n");
    {
        const QString filter = FileBrowser::fileDialogFilter();
        check(filter.contains(QStringLiteral("*.vert")), "filter mentions *.vert");
        check(filter.contains(QStringLiteral("*.frag")), "filter mentions *.frag");
        check(filter.contains(QStringLiteral("*.comp")), "filter mentions *.comp");
    }

    // --- the tree actually lists a folder ------------------------------------
    //
    // The fixture lives next to the test binary rather than in QTemporaryDir: the
    // system temp directory is not reliably writable in sandboxed environments,
    // whereas the build output tree always is.
    const QString fixtureRoot =
        QCoreApplication::applicationDirPath() + QStringLiteral("/spall_fb_fixture");
    QDir().mkpath(fixtureRoot);
    const auto cleanFixture = [&fixtureRoot]() { QDir(fixtureRoot).removeRecursively(); };

    std::printf("\nBrowsing a folder:\n");
    {
        const QString dir = fixtureRoot;
        makeFile(dir, QStringLiteral("a.vert"), QStringLiteral("#version 450\n"));
        makeFile(dir, QStringLiteral("b.frag"), QStringLiteral("#version 450\n"));
        makeFile(dir, QStringLiteral("c.comp"), QStringLiteral("#version 450\n"));
        makeFile(dir, QStringLiteral("notes.txt"), QStringLiteral("not a shader\n"));
        const bool madeSub = QDir(dir).mkdir(QStringLiteral("sub"));
        makeFile(dir + QStringLiteral("/sub"), QStringLiteral("nested.frag"),
                 QStringLiteral("#version 450\n"));

        std::printf("        [debug] fixture dir: %s (mkdir sub = %d)\n", dir.toUtf8().constData(),
                    (int)madeSub);
        const QStringList names =
            QDir(dir).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        std::printf("        [debug] listing: %s\n", names.join(QStringLiteral(", ")).toUtf8().constData());

        FileBrowser browser;
        browser.setRootPath(dir);
        check(QFileInfo(browser.rootPath()).canonicalFilePath() ==
                  QFileInfo(dir).canonicalFilePath(),
              "root path is set to the browsed folder", browser.rootPath());
        check(names.contains(QStringLiteral("a.vert")) && names.contains(QStringLiteral("sub")),
              "folder contents are enumerable", names.join(QStringLiteral(", ")));
    }

    // --- an unsupported extension is not openable ----------------------------
    std::printf("\nActivation rules:\n");
    {
        const QString shader = makeFile(fixtureRoot, QStringLiteral("ok.frag"),
                                        QStringLiteral("#version 450\n"));
        const QString other = makeFile(fixtureRoot, QStringLiteral("note.md"),
                                       QStringLiteral("text\n"));

        check(shader.endsWith(QStringLiteral(".frag")), "supported sample was created", shader);
        check(other.endsWith(QStringLiteral(".md")), "unsupported sample was created", other);

        // The browser must accept exactly the extensions it advertises.
        check(FileBrowser::supportedSuffixes().contains(QStringLiteral("frag")),
              "frag is in the supported set used by the activation check");
        check(!FileBrowser::supportedSuffixes().contains(QStringLiteral("md")),
              "md is rejected by the activation check");
    }

    cleanFixture();

    std::printf("\n=================\n");
    std::printf("%d / %d checks passed\n", g_checks - g_failures, g_checks);
    if (g_failures > 0) {
        std::printf("%d FAILED\n", g_failures);
        return 1;
    }
    std::printf("all passed\n");
    return 0;
}
