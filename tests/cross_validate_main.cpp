// cross_validate_main.cpp — §9.4 cross-validation helper.
//
// Compiles an example, writes the SPIR-V module to a .spv file and an
// spvBinaryToText disassembly to a .spvasm file, plus a raw "plain" disassembly
// with no friendly names or comments. External tools (spirv-dis / spirv-val from
// the Vulkan SDK) can then be run over the same module and compared.
//
// Usage:
//   SpAll_cross_validate <example-file> <stage> <out-basename>

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

#include <cstdio>

#include "CompileOptions.h"
#include "GlslCompiler.h"
#include "SpirvDisassembler.h"

using namespace spall;

static QString readFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    return stream.readAll();
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    if (argc < 4) {
        std::fprintf(stderr, "usage: %s <example-file> <vert|frag|comp|...> <out-basename>\n",
                     argv[0]);
        return 2;
    }

    const QString path = QString::fromUtf8(argv[1]);
    ShaderStage stage = ShaderStage::Vertex;
    if (!stageFromKey(QString::fromUtf8(argv[2]), &stage)) {
        std::fprintf(stderr, "unknown stage: %s\n", argv[2]);
        return 2;
    }
    const QString base = QString::fromUtf8(argv[3]);

    if (!GlslCompiler::initialize()) {
        std::fprintf(stderr, "glslang init failed\n");
        return 2;
    }

    CompileOptions options;
    options.stage = stage;
    options.glslVersion = 450;
    options.vulkanVersion = VulkanVersion::Vulkan13;
    options.spirvVersion = SpirvVersion::Spv16;
    options.targetEnv = TargetEnv::Vulkan13;

    GlslCompiler compiler;
    const CompileResult result = compiler.compile(readFile(path), path, options);
    if (!result.success) {
        std::fprintf(stderr, "compile failed: %s\n", result.failureTitle.toUtf8().constData());
        for (const Diagnostic& d : result.diagnostics) {
            std::fprintf(stderr, "  line %d: %s\n", d.line, d.message.toUtf8().constData());
        }
        GlslCompiler::finalize();
        return 1;
    }

    // --- write the raw module -------------------------------------------------
    QByteArray bytes;
    bytes.resize(static_cast<int>(result.spirv.size() * sizeof(quint32)));
    if (!result.spirv.isEmpty()) {
        memcpy(bytes.data(), result.spirv.constData(), static_cast<size_t>(bytes.size()));
    }
    QFile spv(base + QStringLiteral(".spv"));
    if (!spv.open(QIODevice::WriteOnly)) {
        std::fprintf(stderr, "cannot write %s\n", spv.fileName().toUtf8().constData());
        GlslCompiler::finalize();
        return 1;
    }
    spv.write(bytes);
    spv.close();

    // --- write three disassemblies -------------------------------------------
    // "spvasm": our UI defaults (indent, friendly names, comments)
    // "cli":    exactly what `spirv-dis <file>` prints by default, i.e. indent
    //           plus friendly names, no comments. This is the variant used for
    //           the §9.4 line-by-line cross-validation.
    // "plain":  numeric ids, for reading raw operand structure
    const struct {
        const char* suffix;
        unsigned flags;
    } variants[] = {
        {"spvasm", DisasmIndent | DisasmFriendlyNames | DisasmComment},
        {"cli.spvasm", DisasmIndent | DisasmFriendlyNames},
        {"plain.spvasm", DisasmIndent},
    };

    for (const auto& variant : variants) {
        const DisassemblyResult dis = SpirvDisassembler::disassemble(
            result.spirv, TargetEnv::Vulkan13, variant.flags);
        if (!dis.ok) {
            std::fprintf(stderr, "disassembly failed\n");
            GlslCompiler::finalize();
            return 1;
        }
        QFile file(base + QLatin1Char('.') + QLatin1String(variant.suffix));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            std::fprintf(stderr, "cannot write %s\n", file.fileName().toUtf8().constData());
            GlslCompiler::finalize();
            return 1;
        }
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        stream << dis.text;
    }

    std::printf("wrote %s.spv (%d bytes), %s.spvasm, %s.plain.spvasm\n",
                base.toUtf8().constData(), bytes.size(), base.toUtf8().constData(),
                base.toUtf8().constData());

    GlslCompiler::finalize();
    return 0;
}
