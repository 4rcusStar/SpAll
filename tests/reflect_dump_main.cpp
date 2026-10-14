// Temporary diagnostic: dump what SPIRV-Reflect actually reports for the
// examples, so the two failing checks can be fixed against real data.

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

#include <cstdio>

#include "CompileOptions.h"
#include "GlslCompiler.h"
#include "SpirvDisassembler.h"
#include "SpirvReflection.h"

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

static void dumpInterfaces(const ReflectData& data, const char* label) {
    std::printf("\n--- %s: %d inputs, %d outputs ---\n", label, (int)data.inputs.size(),
                (int)data.outputs.size());
    for (const StageVar& v : data.inputs) {
        std::printf("  IN  loc=%-3u builtin=%-6d name=%-24s type=%-14s builtinName=%s\n",
                    v.location, v.builtin, v.name.toUtf8().constData(),
                    v.typeName.toUtf8().constData(), v.builtinName.toUtf8().constData());
    }
    for (const StageVar& v : data.outputs) {
        std::printf("  OUT loc=%-3u builtin=%-6d name=%-24s type=%-14s builtinName=%s\n",
                    v.location, v.builtin, v.name.toUtf8().constData(),
                    v.typeName.toUtf8().constData(), v.builtinName.toUtf8().constData());
        for (const StageVar& m : v.members) {
            std::printf("        sub loc=%-3u builtin=%-6d name=%-20s type=%-10s builtinName=%s\n",
                        m.location, m.builtin, m.name.toUtf8().constData(),
                        m.typeName.toUtf8().constData(), m.builtinName.toUtf8().constData());
        }
    }
    for (const DescriptorBinding& b : data.descriptors) {
        std::printf("  DESC set=%u bind=%u type=%s name=%s size=%u members=%d\n", b.set, b.binding,
                    b.typeName.toUtf8().constData(), b.name.toUtf8().constData(), b.size,
                    (int)b.members.size());
        for (const ReflectMember& m : b.members) {
            std::printf("        member name=%-20s type=%-10s offset=%u size=%u padded=%u\n",
                        m.name.toUtf8().constData(), m.typeName.toUtf8().constData(), m.offset,
                        m.size, m.paddedSize);
        }
    }
    for (const PushConstantBlock& b : data.pushConstants) {
        std::printf("  PUSH %s offset=%u size=%u members=%d\n", b.name.toUtf8().constData(),
                    b.offset, b.size, (int)b.members.size());
        for (const ReflectMember& m : b.members) {
            std::printf("        member name=%-20s type=%-10s offset=%u size=%u padded=%u\n",
                        m.name.toUtf8().constData(), m.typeName.toUtf8().constData(), m.offset,
                        m.size, m.paddedSize);
        }
    }
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    GlslCompiler::initialize();

    const QString dir = argc > 1 ? QString::fromUtf8(argv[1]) : QStringLiteral("examples");

    const struct {
        const char* file;
        ShaderStage stage;
        const char* label;
    } cases[] = {
        {"basic.vert", ShaderStage::Vertex, "basic.vert"},
        {"ubo.frag", ShaderStage::Fragment, "ubo.frag"},
    };

    GlslCompiler compiler;
    for (const auto& c : cases) {
        CompileOptions options;
        options.stage = c.stage;
        options.glslVersion = 450;
        options.vulkanVersion = VulkanVersion::Vulkan13;
        options.spirvVersion = SpirvVersion::Spv16;
        options.targetEnv = TargetEnv::Vulkan13;

        const QString path = dir + QLatin1Char('/') + QLatin1String(c.file);
        const CompileResult result = compiler.compile(readFile(path), path, options);
        if (!result.success) {
            std::printf("%s: COMPILE FAILED: %s\n", c.label,
                        result.failureTitle.toUtf8().constData());
            continue;
        }
        const ReflectionResult refl = SpirvReflection::reflect(result.spirv);
        if (!refl.ok) {
            std::printf("%s: REFLECTION FAILED\n", c.label);
            continue;
        }
        dumpInterfaces(refl.data, c.label);

        // --- optimization check ---------------------------------------------
        std::printf("  built with SPALL_HAVE_SPIRV_OPT=%d\n", SPALL_HAVE_SPIRV_OPT);
        const OptimizationResult opt =
            SpirvDisassembler::optimize(result.spirv, TargetEnv::Vulkan13,
                                        OptimizationLevel::Performance);
        std::printf("  optimize: ok=%d words %d -> %d\n", (int)opt.ok, (int)result.spirv.size(),
                    (int)opt.words.size());
        for (const Diagnostic& d : opt.diagnostics) {
            std::printf("      diag: %s\n", d.message.toUtf8().constData());
        }
        if (!opt.log.isEmpty()) {
            std::printf("      log: %s\n", opt.log.left(200).toUtf8().constData());
        }
    }

    GlslCompiler::finalize();
    return 0;
}
