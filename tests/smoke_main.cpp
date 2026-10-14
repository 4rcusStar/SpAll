// smoke_main.cpp — GUI-less acceptance test for SpAll's core.
//
// Exercises the full chain on examples/*.{vert,frag,comp}: glslang compile,
// SPIR-V structural checks, spvValidate, reflection enumeration, disassembly and
// the negative cases (bad shader, missing auto-mapping).
//
// Exits non-zero and prints the reason for every failure.

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>

#include <cstdio>

#include "CompileOptions.h"
#include "CompileResult.h"
#include "GlslCompiler.h"
#include "SpirvDisassembler.h"
#include "SpirvReflection.h"

using namespace spall;

namespace {

int g_failures = 0;
int g_checks = 0;

QTextStream& out() {
    static QTextStream stream(stdout);
    return stream;
}

void reportPass(const QString& what) {
    ++g_checks;
    out() << "  [ OK ] " << what << Qt::endl;
}

void reportFail(const QString& what, const QString& why) {
    ++g_checks;
    ++g_failures;
    out() << "  [FAIL] " << what << Qt::endl;
    out() << "         " << why << Qt::endl;
}

void check(bool condition, const QString& what, const QString& why) {
    if (condition) {
        reportPass(what);
    } else {
        reportFail(what, why);
    }
}

QString readFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    return stream.readAll();
}

/// Locate examples/ regardless of the working directory the test is run from.
QString findExamplesDir() {
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
            return QDir(candidate).absolutePath();
        }
    }
    return QString();
}

CompileOptions baseOptions(ShaderStage stage) {
    CompileOptions options;
    options.stage = stage;
    options.glslVersion = 450;
    options.vulkanVersion = VulkanVersion::Vulkan13;
    options.spirvVersion = SpirvVersion::Spv16;
    options.targetEnv = TargetEnv::Vulkan13;
    options.validate = true;
    return options;
}

/// Compile one example and run the standard structural assertions.
CompileResult compileExample(const QString& path, ShaderStage stage, const QString& label) {
    out() << "\n[" << label << "] " << QFileInfo(path).fileName() << Qt::endl;

    const QString source = readFile(path);
    if (source.isEmpty()) {
        reportFail(QStringLiteral("read source"), QStringLiteral("cannot read %1").arg(path));
        return {};
    }

    GlslCompiler compiler;
    const CompileResult result = compiler.compile(source, path, baseOptions(stage));

    if (!result.success) {
        QStringList messages;
        for (const Diagnostic& diag : result.diagnostics) {
            messages << QStringLiteral("line %1: %2").arg(diag.line).arg(diag.message);
        }
        reportFail(QStringLiteral("compile succeeded"),
                   QStringLiteral("%1\n         %2").arg(result.failureTitle,
                                                         messages.join(QStringLiteral("\n         "))));
        return result;
    }
    reportPass(QStringLiteral("compile succeeded (%1 ms)").arg(result.compileMs));

    check(result.spirv.size() > 5,
          QStringLiteral("SPIR-V word count > 5"),
          QStringLiteral("actual %1 words").arg(result.spirv.size()));
    check(!result.spirv.isEmpty() && result.spirv.first() == 0x07230203u,
          QStringLiteral("magic == 0x07230203"),
          QStringLiteral("actual 0x%1").arg(result.spirv.isEmpty() ? 0 : result.spirv.first(), 8, 16,
                                          QLatin1Char('0')));
    check(result.idBound > 0, QStringLiteral("id bound > 0"),
          QStringLiteral("actual %1").arg(result.idBound));

    // --- validation ---------------------------------------------------------
    const ValidationResult validation = SpirvDisassembler::validate(result.spirv, TargetEnv::Vulkan13);
    if (validation.ok) {
        reportPass(QStringLiteral("spvValidate returns SPV_SUCCESS"));
    } else {
        QStringList messages;
        for (const Diagnostic& diag : validation.diagnostics) {
            messages << diag.message;
        }
        reportFail(QStringLiteral("spvValidate returns SPV_SUCCESS"),
                   messages.join(QStringLiteral("\n         ")));
    }

    // --- disassembly --------------------------------------------------------
    const DisassemblyResult disassembly =
        SpirvDisassembler::disassemble(result.spirv, TargetEnv::Vulkan13,
                                       DisasmIndent | DisasmFriendlyNames | DisasmComment);
    check(disassembly.ok && !disassembly.text.isEmpty(),
          QStringLiteral("disassembly produced text"),
          disassembly.ok ? QStringLiteral("text is empty") : QStringLiteral("disassembly failed"));

    // --- reflection ---------------------------------------------------------
    const ReflectionResult reflection = SpirvReflection::reflect(result.spirv);
    check(reflection.ok, QStringLiteral("reflection succeeded"),
          reflection.diagnostics.isEmpty() ? QStringLiteral("unknown reason")
                                           : reflection.diagnostics.first().message);

    return result;
}

/// Assert the expected descriptor/interface shape of one example.
void checkExpectations(const CompileResult& result,
                       int expectedDescriptors,
                       int expectedPushConstants,
                       const QString& label) {
    if (!result.success) {
        return;
    }
    const ReflectionResult reflection = SpirvReflection::reflect(result.spirv);
    if (!reflection.ok) {
        return;
    }
    check(reflection.data.descriptors.size() == expectedDescriptors,
          QStringLiteral("%1: descriptor count == %2").arg(label).arg(expectedDescriptors),
          QStringLiteral("actual %1").arg(reflection.data.descriptors.size()));
    check(reflection.data.pushConstants.size() == expectedPushConstants,
          QStringLiteral("%1: push constant block count == %2").arg(label).arg(expectedPushConstants),
          QStringLiteral("actual %1").arg(reflection.data.pushConstants.size()));
}

/// Compile deliberately broken source and require a positioned error.
void checkErrorReporting(const QString& examplesDir) {
    out() << "\n[negative] a broken shader must report a positioned error" << Qt::endl;

    const QString broken = QStringLiteral(
        "#version 450\n"
        "layout(location = 0) out vec3 fragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec3 color = vec3(1.0)\n"      // line 6: missing semicolon
        "    fragColor = undeclaredVar;\n"  // line 7: undeclared identifier
        "}\n");

    GlslCompiler compiler;
    const CompileResult result = compiler.compile(broken, QStringLiteral("broken.frag"),
                                                 baseOptions(ShaderStage::Fragment));

    check(!result.success, QStringLiteral("broken source fails to compile"),
          QStringLiteral("it compiled anyway"));

    bool foundPositionedError = false;
    int positionedLine = 0;
    for (const Diagnostic& diag : result.diagnostics) {
        if (diag.severity == Severity::Error && diag.hasPosition()) {
            foundPositionedError = true;
            positionedLine = diag.line;
            break;
        }
    }
    check(foundPositionedError, QStringLiteral("at least one error carries a line"),
          QStringLiteral("no diagnostic carries a line"));

    // Line 7 references an undeclared identifier; glslang may also flag line 6.
    // Accept either, but require the reported line to be inside the function body.
    check(positionedLine >= 6 && positionedLine <= 7,
          QStringLiteral("error line falls within 6..7"),
          QStringLiteral("actual line %1").arg(positionedLine));

    // --- empty source must not crash ----------------------------------------
    const CompileResult emptyResult = compiler.compile(QString(), QStringLiteral("empty.vert"),
                                                       baseOptions(ShaderStage::Vertex));
    check(!emptyResult.success, QStringLiteral("empty source fails instead of crashing"),
          QStringLiteral("empty source succeeded anyway"));

    // --- include remapping --------------------------------------------------
    const QString includeMain = QStringLiteral(
        "#version 450\n"
        "#include \"common.glsl\"\n"
        "void main() { fragColor = helper(); }\n");
    QFile includeFile(QDir(examplesDir).absoluteFilePath(QStringLiteral("common.glsl")));
    if (includeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&includeFile);
        stream.setEncoding(QStringConverter::Utf8);
        // The helper is intentionally broken on purpose, on line 2 of the include.
        stream << "// helper library\n"
               << "vec4 helper() { return notAVariable; }\n";
        includeFile.close();

        CompileOptions includeOptions = baseOptions(ShaderStage::Fragment);
        includeOptions.enableIncludes = true;
        includeOptions.includePaths << examplesDir;
        const CompileResult includeResult =
            compiler.compile(includeMain, QDir(examplesDir).absoluteFilePath(QStringLiteral("main.frag")),
                             includeOptions);

        check(!includeResult.success, QStringLiteral("an error inside #include fails the compile"),
              QStringLiteral("it succeeded anyway"));

        bool remappedToInclude = false;
        for (const Diagnostic& diag : includeResult.diagnostics) {
            if (diag.file.endsWith(QStringLiteral("common.glsl")) && diag.line == 2) {
                remappedToInclude = true;
                break;
            }
        }
        check(remappedToInclude,
              QStringLiteral("#include line maps back to common.glsl:2"),
              QStringLiteral("no diagnostic points at common.glsl line 2"));

        includeFile.remove();
    } else {
        reportFail(QStringLiteral("write common.glsl"), includeFile.errorString());
    }
}

/// Turning off auto-mapping must make Vulkan validation reject the module. This
/// proves the validation link actually runs rather than always passing.
void checkValidationCatchesMissingBindings() {
    out() << "\n[negative] auto-map off: no layout(binding=) must fail" << Qt::endl;

    const QString source = QStringLiteral(
        "#version 450\n"
        "layout(location = 0) out vec4 fragColor;\n"
        "layout(set = 0) uniform Unnamed\n"
        "{\n"
        "    vec4 tint;\n"
        "} ubo;\n"
        "void main() { fragColor = ubo.tint; }\n");

    CompileOptions options = baseOptions(ShaderStage::Fragment);
    options.autoMapBindings = false;
    options.autoMapLocations = false;

    GlslCompiler compiler;
    const CompileResult result = compiler.compile(source, QStringLiteral("nobinding.frag"), options);

    if (!result.success) {
        // glslang itself may reject it, which is also a valid outcome.
        reportPass(QStringLiteral("glslang rejected the module with missing bindings"));
        return;
    }

    const ValidationResult validation = SpirvDisassembler::validate(result.spirv, TargetEnv::Vulkan13);
    check(!validation.ok,
          QStringLiteral("validation rejected the module with missing bindings"),
          QStringLiteral("validation passed anyway, so validation is not really running"));
}

/// Optimization must run, keep the module valid, and actually change it.
void checkOptimization(const QString& examplesDir) {
    out() << "\n[negative->positive] -O must change word count, stay valid" << Qt::endl;

    // A shader with real optimization headroom: constant folding plus redundant
    // arithmetic that spirv-opt can collapse.
    const QString source = QStringLiteral(
        "#version 450\n"
        "layout(location = 0) out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    float a = 1.0 + 2.0;\n"
        "    float b = a * 3.0;\n"
        "    float c = b + 0.0;\n"
        "    float d = c * 1.0;\n"
        "    fragColor = vec4(d, d, d, 1.0);\n"
        "}\n");

    GlslCompiler compiler;
    const CompileResult result = compiler.compile(source, QStringLiteral("opt.frag"),
                                                  baseOptions(ShaderStage::Fragment));
    if (!result.success) {
        reportFail(QStringLiteral("compile optimization case"), result.failureTitle);
        return;
    }

    const OptimizationResult performance =
        SpirvDisassembler::optimize(result.spirv, TargetEnv::Vulkan13, OptimizationLevel::Performance);

#if SPALL_HAVE_SPIRV_OPT
    check(performance.ok, QStringLiteral("optimization ran successfully"),
          performance.diagnostics.isEmpty() ? QStringLiteral("unknown reason")
                                            : performance.diagnostics.first().message);
    if (performance.ok) {
        check(!performance.words.isEmpty(), QStringLiteral("optimized module still has instructions"),
              QStringLiteral("optimization output is empty"));
        check(SpirvDisassembler::hasValidMagic(performance.words),
              QStringLiteral("optimized module has a valid magic"),
              QStringLiteral("invalid magic"));

        const ValidationResult validation =
            SpirvDisassembler::validate(performance.words, TargetEnv::Vulkan13);
        check(validation.ok, QStringLiteral("optimized module passes validation"),
              validation.diagnostics.isEmpty() ? QStringLiteral("unknown reason")
                                               : validation.diagnostics.first().message);

        out() << "         unoptimized " << result.spirv.size() << " words -> optimized "
              << performance.words.size() << " words" << Qt::endl;
        check(performance.words.size() < result.spirv.size(),
              QStringLiteral("optimization reduced the word count"),
              QStringLiteral("word count did not drop (%1 -> %2), level had no effect")
                  .arg(result.spirv.size())
                  .arg(performance.words.size()));

        // -Os must also succeed; it need not shrink further than -O.
        const OptimizationResult size =
            SpirvDisassembler::optimize(result.spirv, TargetEnv::Vulkan13,
                                        OptimizationLevel::Size);
        check(size.ok, QStringLiteral("size level -Os ran successfully"),
              size.diagnostics.isEmpty() ? QStringLiteral("unknown reason")
                                         : size.diagnostics.first().message);
    }

    // None must be a true pass-through.
    const OptimizationResult none =
        SpirvDisassembler::optimize(result.spirv, TargetEnv::Vulkan13, OptimizationLevel::None);
    check(none.ok && none.words == result.spirv,
          QStringLiteral("the none optimization level leaves the module unchanged"),
          QStringLiteral("the none level modified the module"));
#else
    check(performance.ok, QStringLiteral("optimization degrades gracefully without opt"),
          QStringLiteral("optimization returned failure"));
    out() << "         (SPIRV-Tools-opt is not enabled in this build)" << Qt::endl;
#endif

    Q_UNUSED(examplesDir);
}

/// Statistics collection must agree with the module header.
void checkStatistics() {
    out() << "\n[stats] instruction statistics" << Qt::endl;

    GlslCompiler compiler;
    const CompileResult result = compiler.compile(
        QStringLiteral("#version 450\nvoid main() { gl_Position = vec4(0.0); }\n"),
        QStringLiteral("stats.vert"), baseOptions(ShaderStage::Vertex));
    if (!result.success) {
        reportFail(QStringLiteral("compile statistics case"), result.failureTitle);
        return;
    }

    const SpirvStats stats =
        SpirvDisassembler::collectStatistics(result.spirv, TargetEnv::Vulkan13);
    check(stats.wordCount == static_cast<quint32>(result.spirv.size()),
          QStringLiteral("counted words match the module"),
          QStringLiteral("%1 vs %2").arg(stats.wordCount).arg(result.spirv.size()));
    check(stats.instructionCount > 0, QStringLiteral("instruction count > 0"),
          QStringLiteral("actual %1").arg(stats.instructionCount));
    check(!stats.opcodeCounts.isEmpty(), QStringLiteral("opcode counts collected"),
          QStringLiteral("opcode counts are empty"));
    out() << "         " << stats.instructionCount << " instructions, "
          << stats.opcodeCounts.size() << " distinct opcodes" << Qt::endl;
}

}  // namespace

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("SpAll_smoke"));

    out() << "SpAll smoke test" << Qt::endl;
    out() << "================" << Qt::endl;

    if (!GlslCompiler::initialize()) {
        out() << "FATAL: glslang initialization failed" << Qt::endl;
        return 2;
    }

    const QString examplesDir = findExamplesDir();
    if (examplesDir.isEmpty()) {
        out() << "FATAL: examples dir not found (no basic.vert in candidates)" << Qt::endl;
        out() << "       current dir: " << QDir::currentPath() << Qt::endl;
        GlslCompiler::finalize();
        return 2;
    }
    out() << "examples dir: " << examplesDir << Qt::endl;

    // --- the four required examples -----------------------------------------
    const CompileResult vert = compileExample(examplesDir + QStringLiteral("/basic.vert"),
                                              ShaderStage::Vertex, QStringLiteral("basic.vert"));
    checkExpectations(vert, 0, 0, QStringLiteral("basic.vert"));

    // basic.vert must expose gl_Position as a builtin output. gl_Position lives
    // inside the gl_PerVertex interface *block*, so its members are searched too.
    if (vert.success) {
        const ReflectionResult reflection = SpirvReflection::reflect(vert.spirv);
        if (reflection.ok) {
            const auto isPosition = [](const StageVar& var) {
                return var.builtinName == QLatin1String("Position");
            };
            bool foundPosition = false;
            for (const StageVar& var : reflection.data.outputs) {
                if (isPosition(var)) {
                    foundPosition = true;
                    break;
                }
                for (const StageVar& member : var.members) {
                    if (isPosition(member)) {
                        foundPosition = true;
                        break;
                    }
                }
                if (foundPosition) {
                    break;
                }
            }
            check(foundPosition, QStringLiteral("basic.vert output contains builtin Position"),
                  QStringLiteral("gl_Position not found in outputs (incl. block members)"));
        }
    }

    const CompileResult frag = compileExample(examplesDir + QStringLiteral("/basic.frag"),
                                              ShaderStage::Fragment, QStringLiteral("basic.frag"));
    checkExpectations(frag, 0, 0, QStringLiteral("basic.frag"));

    const CompileResult ubo = compileExample(examplesDir + QStringLiteral("/ubo.frag"),
                                             ShaderStage::Fragment, QStringLiteral("ubo.frag"));
    checkExpectations(ubo, 2, 1, QStringLiteral("ubo.frag"));

    // ubo.frag must report the layout offsets the source declares.
    if (ubo.success) {
        const ReflectionResult reflection = SpirvReflection::reflect(ubo.spirv);
        if (reflection.ok && !reflection.data.descriptors.isEmpty()) {
            const DescriptorBinding* uniform = nullptr;
            for (const DescriptorBinding& binding : reflection.data.descriptors) {
                if (binding.typeName == QLatin1String("uniform buffer")) {
                    uniform = &binding;
                    break;
                }
            }
            if (uniform != nullptr) {
                check(uniform->set == 0 && uniform->binding == 0,
                      QStringLiteral("ubo.frag: UBO is at set=0 binding=0"),
                      QStringLiteral("actual set=%1 binding=%2").arg(uniform->set).arg(uniform->binding));
                bool offsetOk = false;
                for (const ReflectMember& member : uniform->members) {
                    if (member.name == QLatin1String("tint") && member.offset == 0) {
                        offsetOk = true;
                    }
                }
                check(offsetOk, QStringLiteral("ubo.frag: UBO member tint has offset == 0"),
                      QStringLiteral("no tint member with offset 0 was found"));
            } else {
                reportFail(QStringLiteral("ubo.frag: uniform buffer found"),
                           QStringLiteral("no uniform buffer among the descriptors"));
            }

            if (!reflection.data.pushConstants.isEmpty()) {
                const PushConstantBlock& block = reflection.data.pushConstants.first();
                check(block.offset == 0, QStringLiteral("ubo.frag: push constant offset == 0"),
                      QStringLiteral("actual %1").arg(block.offset));
                check(block.size >= 16,
                      QStringLiteral("ubo.frag: push constant size >= 16"),
                      QStringLiteral("actual %1").arg(block.size));
            }
        }
    }

    const CompileResult comp = compileExample(examplesDir + QStringLiteral("/comp.comp"),
                                              ShaderStage::Compute, QStringLiteral("comp.comp"));
    checkExpectations(comp, 1, 0, QStringLiteral("comp.comp"));

    if (comp.success) {
        const ReflectionResult reflection = SpirvReflection::reflect(comp.spirv);
        if (reflection.ok) {
            check(reflection.data.execution.localSizeX == 8 &&
                      reflection.data.execution.localSizeY == 8,
                  QStringLiteral("comp.comp: local_size == 8x8"),
                  QStringLiteral("actual %1x%2").arg(reflection.data.execution.localSizeX)
                      .arg(reflection.data.execution.localSizeY));
        }
    }

    // --- negative and cross-cutting cases -----------------------------------
    checkErrorReporting(examplesDir);
    checkValidationCatchesMissingBindings();
    checkOptimization(examplesDir);
    checkStatistics();

    // --- disassembly sanity on a known-good module --------------------------
    if (vert.success) {
        const DisassemblyResult disassembly =
            SpirvDisassembler::disassemble(vert.spirv, TargetEnv::Vulkan13,
                                           DisasmIndent | DisasmFriendlyNames | DisasmComment);
        check(disassembly.ok && disassembly.text.contains(QStringLiteral("OpEntryPoint")),
              QStringLiteral("assembly contains OpEntryPoint"),
              QStringLiteral("OpEntryPoint not found in the disassembly"));
        check(disassembly.text.contains(QStringLiteral("Vertex")),
              QStringLiteral("assembly contains the Vertex execution model"),
              QStringLiteral("Vertex not found in the disassembly"));
    }

    GlslCompiler::finalize();

    out() << "\n================" << Qt::endl;
    std::printf("%d / %d checks passed\n", g_checks - g_failures, g_checks);
    if (g_failures > 0) {
        std::printf("%d FAILED\n", g_failures);
        return 1;
    }
    std::printf("all passed\n");
    return 0;
}
