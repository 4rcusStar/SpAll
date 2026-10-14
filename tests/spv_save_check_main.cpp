// spv_save_check_main.cpp — verifies the saved .spv file is a usable module.
//
// Compiles an example, writes it out the same way AsmPanel::saveBinaryAs() does,
// reads it back, and checks the header plus that SPIRV-Tools accepts it. This
// catches a wrong word order, a truncation or a text-mode write.

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include <cstdio>
#include <cstring>

#include "CompileOptions.h"
#include "Diagnostic.h"
#include "GlslCompiler.h"
#include "SpirvDisassembler.h"

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

QString readTextFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    return stream.readAll();
}

/// Mirror of AsmPanel::saveBinaryAs' write step.
bool writeSpv(const QVector<quint32>& words, const QString& path, QString* error) {
    QByteArray bytes;
    bytes.resize(static_cast<int>(words.size() * sizeof(quint32)));
    if (!words.isEmpty()) {
        memcpy(bytes.data(), words.constData(), static_cast<size_t>(bytes.size()));
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        *error = file.errorString();
        return false;
    }
    const qint64 written = file.write(bytes);
    file.close();
    if (written != bytes.size()) {
        *error = QStringLiteral("short write");
        return false;
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    std::printf("SPIR-V save test\n");
    std::printf("================\n");

    // The examples ship next to the executable, but the test may also be run from
    // the build tree or the source tree, so search the usual relative locations
    // before giving up.
    QString examples = argc > 1 ? QString::fromUtf8(argv[1]) : QString();
    if (examples.isEmpty() || !QFileInfo::exists(examples + QStringLiteral("/basic.vert"))) {
        const QStringList candidates = {
            QDir::current().absoluteFilePath(QStringLiteral("examples")),
            QDir::current().absoluteFilePath(QStringLiteral("../examples")),
            QDir::current().absoluteFilePath(QStringLiteral("../../examples")),
            QDir::current().absoluteFilePath(QStringLiteral("../../../examples")),
            QCoreApplication::applicationDirPath() + QStringLiteral("/examples"),
            QCoreApplication::applicationDirPath() + QStringLiteral("/../examples"),
            QCoreApplication::applicationDirPath() + QStringLiteral("/../../examples"),
        };
        for (const QString& candidate : candidates) {
            if (QFileInfo::exists(candidate + QStringLiteral("/basic.vert"))) {
                examples = QDir(candidate).absolutePath();
                break;
            }
        }
    }
    const QString outDir = QDir::temp().absoluteFilePath(QStringLiteral("spall_spv_test"));
    QDir().mkpath(outDir);

    if (examples.isEmpty() || !QFileInfo::exists(examples + QStringLiteral("/basic.vert"))) {
        std::printf("FATAL: examples dir not found (no basic.vert in candidates)\n");
        return 2;
    }
    std::printf("examples dir: %s\n", examples.toUtf8().constData());

    if (!GlslCompiler::initialize()) {
        std::printf("FATAL: glslang init failed\n");
        return 2;
    }

    struct Case {
        const char* file;
        ShaderStage stage;
    };
    const Case cases[] = {
        {"basic.vert", ShaderStage::Vertex},
        {"ubo.frag", ShaderStage::Fragment},
        {"comp.comp", ShaderStage::Compute},
    };

    GlslCompiler compiler;
    for (const Case& c : cases) {
        std::printf("\n%s:\n", c.file);

        CompileOptions options;
        options.stage = c.stage;
        options.glslVersion = 450;
        options.vulkanVersion = VulkanVersion::Vulkan13;
        options.spirvVersion = SpirvVersion::Spv16;
        options.targetEnv = TargetEnv::Vulkan13;

        const QString path = examples + QLatin1Char('/') + QLatin1String(c.file);
        const CompileResult result = compiler.compile(readTextFile(path), path, options);
        if (!result.success) {
            check(false, "compiles", result.failureTitle);
            continue;
        }

        const QString spvPath = outDir + QLatin1Char('/') + QLatin1String(c.file) +
                                QStringLiteral(".spv");
        QString error;
        if (!writeSpv(result.spirvOptimized, spvPath, &error)) {
            check(false, "write .spv", error);
            continue;
        }

        // --- read it back as raw bytes and verify the header -----------------
        QFile spv(spvPath);
        if (!spv.open(QIODevice::ReadOnly)) {
            check(false, "read back .spv", spv.errorString());
            continue;
        }
        const QByteArray bytes = spv.readAll();
        spv.close();

        check(bytes.size() == static_cast<int>(result.spirvOptimized.size() * 4),
              "file size equals words * 4",
              QStringLiteral("%1 vs %2").arg(bytes.size()).arg(result.spirvOptimized.size() * 4));

        // Magic number in little-endian byte order.
        const unsigned char* raw = reinterpret_cast<const unsigned char*>(bytes.constData());
        const quint32 magic = static_cast<quint32>(raw[0]) | (static_cast<quint32>(raw[1]) << 8) |
                              (static_cast<quint32>(raw[2]) << 16) |
                              (static_cast<quint32>(raw[3]) << 24);
        check(magic == 0x07230203u, "little-endian magic is 0x07230203",
              QStringLiteral("read 0x%1").arg(magic, 8, 16, QLatin1Char('0')));

        // --- the module must still validate after the round trip ------------
        QVector<quint32> reloaded;
        reloaded.resize(bytes.size() / 4);
        if (!reloaded.isEmpty()) {
            memcpy(reloaded.data(), bytes.constData(), static_cast<size_t>(bytes.size()));
        }
        check(reloaded == result.spirvOptimized, "reloaded words match the compiled module");

        const ValidationResult validation =
            SpirvDisassembler::validate(reloaded, TargetEnv::Vulkan13);
        check(validation.ok, "round-tripped .spv passes spvValidate",
              validation.diagnostics.isEmpty() ? QString()
                                               : validation.diagnostics.first().message);

        // A text-mode write would corrupt the module; the bytes must not look like
        // UTF-16 text (a very common bug when saving binary data on Windows).
        bool looksLikeText = bytes.size() >= 4 && raw[0] != 0x03 && raw[1] == 0x00;
        check(!looksLikeText, "file is binary, not a text-mode write");

        std::printf("        wrote %s (%d bytes)\n", spvPath.toUtf8().constData(), bytes.size());
    }

    GlslCompiler::finalize();

    // Leave the generated file in place for external tools, and report where.
    std::printf("\n================\n");
    std::printf("%d / %d checks passed\n", g_checks - g_failures, g_checks);
    std::printf("files in: %s\n", outDir.toUtf8().constData());
    if (g_failures > 0) {
        std::printf("%d FAILED\n", g_failures);
        return 1;
    }
    std::printf("all passed\n");
    return 0;
}
