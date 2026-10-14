// GlslCompiler.cpp — glslang front-end wrapper.
#include "GlslCompiler.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QStringList>

#include <atomic>
#include <memory>

#include "IncludePreprocessor.h"

// glslang public API.
//
// Namespace detail that is easy to get wrong in this glslang revision:
//   * GLOBAL scope: TBuiltInResource, EShLanguage, EShMessages and the EShMsg*
//     flags (ResourceLimits.h, ShaderLang.h),
//   * namespace glslang: EShSource, EShClient, EShTarget*, TResourceType,
//     TShader / TProgram / SpvOptions.
// Qualifying a global name as glslang:: fails with "is not a member of glslang";
// leaving a namespaced name unqualified fails with "undeclared".
//
// glslang/Public/ResourceLimits.h is included explicitly because that is where
// GetDefaultResources() lives; ShaderLang.h only pulls in Include/ResourceLimits.h
// (the struct itself).
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/SpvTools.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv-tools/libspirv.h>

namespace spall {

namespace {

// Localised string for this translation context.
//
// A macro rather than a function on purpose: lupdate parses sources statically
// and cannot see through a call that passes the text on to
// QCoreApplication::translate(). Spelling the lookup out here means every trs(QT_TRANSLATE_NOOP("Compiler", ...))
// call site is extracted automatically, while the runtime behaviour is unchanged.
/// Localised string for this translation context.
///
/// The text is passed through QT_TRANSLATE_NOOP at every call site, which is what
/// lets lupdate extract it statically; this function then performs the runtime
/// lookup. core/ has no QObject, so tr() is not available there.
QString trs(const char* text) {
    return QCoreApplication::translate("Compiler", text);
}

std::atomic<bool> g_initialized{false};

/// RAII holder for the glslang objects owned by one compile call.
struct ShaderGuard {
    glslang::TShader* shader = nullptr;
    ~ShaderGuard() { delete shader; }
};

/// Translate our option enums onto glslang's target enums.
glslang::EShTargetClientVersion toClientVersion(VulkanVersion version) {
    switch (version) {
        case VulkanVersion::Vulkan10: return glslang::EShTargetVulkan_1_0;
        case VulkanVersion::Vulkan11: return glslang::EShTargetVulkan_1_1;
        case VulkanVersion::Vulkan12: return glslang::EShTargetVulkan_1_2;
        case VulkanVersion::Vulkan13: return glslang::EShTargetVulkan_1_3;
    }
    return glslang::EShTargetVulkan_1_3;
}

glslang::EShTargetLanguageVersion toSpirvVersion(SpirvVersion version) {
    switch (version) {
        case SpirvVersion::Spv10: return glslang::EShTargetSpv_1_0;
        case SpirvVersion::Spv11: return glslang::EShTargetSpv_1_1;
        case SpirvVersion::Spv12: return glslang::EShTargetSpv_1_2;
        case SpirvVersion::Spv13: return glslang::EShTargetSpv_1_3;
        case SpirvVersion::Spv14: return glslang::EShTargetSpv_1_4;
        case SpirvVersion::Spv15: return glslang::EShTargetSpv_1_5;
        case SpirvVersion::Spv16: return glslang::EShTargetSpv_1_6;
    }
    return glslang::EShTargetSpv_1_6;
}

/// Fill glslang's built-in resource limits from our explicit values so that
/// ordinary shaders do not trip glslang's conservative defaults.
///
/// Fields are named individually rather than through arrays because
/// TBuiltInResource changed shape across glslang releases: this revision uses
/// maxComputeWorkGroupCountX/Y/Z, not maxComputeWorkGroupCount[].
void applyResourceLimits(const GlslangResourceLimits& limits, TBuiltInResource* res) {
    res->maxLights = limits.maxLights;
    res->maxClipPlanes = limits.maxClipPlanes;
    res->maxTextureUnits = limits.maxTextureUnits;
    res->maxTextureCoords = limits.maxTextureCoords;
    res->maxVertexAttribs = limits.maxVertexAttribs;
    res->maxVertexUniformComponents = limits.maxVertexUniformComponents;
    res->maxFragmentUniformComponents = limits.maxFragmentUniformComponents;
    res->maxVaryingFloats = limits.maxVaryingFloats;
    res->maxVertexTextureImageUnits = limits.maxVertexTextureImageUnits;
    res->maxCombinedTextureImageUnits = limits.maxCombinedTextureImageUnits;
    res->maxTextureImageUnits = limits.maxTextureImageUnits;
    res->maxGeometryTextureImageUnits = limits.maxGeometryTextureImageUnits;
    res->maxGeometryOutputVertices = limits.maxGeometryOutputVertices;
    res->maxComputeWorkGroupCountX = limits.maxComputeWorkGroupCountX;
    res->maxComputeWorkGroupCountY = limits.maxComputeWorkGroupCountY;
    res->maxComputeWorkGroupCountZ = limits.maxComputeWorkGroupCountZ;
    res->maxComputeWorkGroupSizeX = limits.maxComputeWorkGroupSizeX;
    res->maxComputeWorkGroupSizeY = limits.maxComputeWorkGroupSizeY;
    res->maxComputeWorkGroupSizeZ = limits.maxComputeWorkGroupSizeZ;
    res->maxComputeUniformComponents = limits.maxComputeUniformComponents;
    res->maxComputeAtomicCounters = limits.maxComputeAtomicCounters;
    res->maxComputeAtomicCounterBuffers = limits.maxComputeAtomicCounterBuffers;
    res->maxCombinedImageUnitsAndFragmentOutputs = limits.maxCombinedImageUnitsAndFragmentOutputs;
    res->maxImageUnits = limits.maxImageUnits;
    res->maxVertexOutputComponents = limits.maxVertexOutputComponents;
    res->maxGeometryInputComponents = limits.maxGeometryInputComponents;
    res->maxGeometryOutputComponents = limits.maxGeometryOutputComponents;
    res->maxFragmentInputComponents = limits.maxFragmentInputComponents;
    res->maxAtomicCounterBufferSize = limits.maxAtomicCounterBufferSize;
    res->maxAtomicCounterBindings = limits.maxAtomicCounterBindings;
    res->maxTransformFeedbackBuffers = limits.maxTransformFeedbackBuffers;
    res->maxViewports = limits.maxViewports;
    res->maxSamples = limits.maxSamples;
    res->maxPatchVertices = limits.maxPatchVertices;
}

/// Human-readable stage name for messages.
QString stageLabel(ShaderStage stage) {
    return stageName(stage);
}

}  // namespace

// --- lifecycle ---------------------------------------------------------------

bool GlslCompiler::initialize() {
    if (g_initialized.load(std::memory_order_acquire)) {
        return true;
    }
    if (!glslang::InitializeProcess()) {
        return false;
    }
    g_initialized.store(true, std::memory_order_release);
    return true;
}

void GlslCompiler::finalize() {
    if (g_initialized.exchange(false, std::memory_order_acq_rel)) {
        glslang::FinalizeProcess();
    }
}

bool GlslCompiler::isInitialized() {
    return g_initialized.load(std::memory_order_acquire);
}

GlslCompiler::GlslCompiler() = default;
GlslCompiler::~GlslCompiler() = default;

// --- compile -----------------------------------------------------------------

CompileResult GlslCompiler::compile(const QString& source,
                                    const QString& fileName,
                                    const CompileOptions& options) const {
    QElapsedTimer timer;
    timer.start();

    CompileResult result;

    // --- guards --------------------------------------------------------------
    if (!isInitialized()) {
        result.failureTitle =
            trs(QT_TRANSLATE_NOOP("Compiler", "glslang has not been initialized, so compilation is impossible."));
        result.failureHint = trs(QT_TRANSLATE_NOOP("Compiler", "Restart the application; if the problem persists, glslang::InitializeProcess() "
            "failed, so check the start-up messages in the log panel."));
        result.compileMs = timer.elapsed();
        return result;
    }
    if (source.trimmed().isEmpty()) {
        result.failureTitle = trs(QT_TRANSLATE_NOOP("Compiler", "The source is empty, so there is nothing to compile."));
        result.failureHint = trs(QT_TRANSLATE_NOOP("Compiler", "Enter GLSL code first, or open a shader file from the file list on the left."));
        result.diagnostics.append(Diagnostic{Severity::Error, DiagSource::GlslSyntax, 0, 0, fileName,
                                             trs(QT_TRANSLATE_NOOP("Compiler", "The source is empty"))});
        result.recountDiagnostics();
        result.compileMs = timer.elapsed();
        return result;
    }
    if (!options.isSane()) {
        result.failureTitle = trs(QT_TRANSLATE_NOOP("Compiler", "GLSL version %1 is not valid.")).arg(options.glslVersion);
        result.failureHint =
            trs(QT_TRANSLATE_NOOP("Compiler", "Pick a version between 100 and 460 in the version combo box on the toolbar."));
        result.compileMs = timer.elapsed();
        return result;
    }

    // --- 1. #include expansion ----------------------------------------------
    IncludePreprocessor preprocessor;
    const PreprocessResult pre = preprocessor.process(source, fileName, options.includePaths,
                                                      options.enableIncludes);
    for (const Diagnostic& diag : pre.diagnostics) {
        result.diagnostics.append(diag);
    }
    if (!pre.ok) {
        result.failureTitle = trs(QT_TRANSLATE_NOOP("Compiler", "#include expansion failed, so compilation did not start."));
        result.failureHint = trs(QT_TRANSLATE_NOOP("Compiler", "Fix the #include paths listed in the problem list below, or turn off "
            "\"Enable #include\" in the settings."));
        result.recountDiagnostics();
        result.compileMs = timer.elapsed();
        return result;
    }

    // glslang needs a trailing newline on the last line.
    QString text = pre.source;
    if (!text.endsWith(QLatin1Char('\n'))) {
        text.append(QLatin1Char('\n'));
    }
    const QByteArray utf8 = text.toUtf8();

    // --- 2. shader object ----------------------------------------------------
    const EShLanguage language = static_cast<EShLanguage>(stageToGlslang(options.stage));
    ShaderGuard guard;
    guard.shader = new glslang::TShader(language);
    glslang::TShader& shader = *guard.shader;

    const char* strings[1] = {utf8.constData()};
    const int lengths[1] = {static_cast<int>(utf8.size())};
    shader.setStringsWithLengths(strings, lengths, 1);

    shader.setEntryPoint("main");
    shader.setSourceEntryPoint("main");

    shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientVulkan,
                       options.glslVersion);
    shader.setEnvClient(glslang::EShClientVulkan, toClientVersion(options.vulkanVersion));
    shader.setEnvTarget(glslang::EShTargetSpv, toSpirvVersion(options.spirvVersion));

    // Auto-mapping is what makes a shader without explicit layout() qualifiers
    // still produce validatable SPIR-V.
    shader.setAutoMapLocations(options.autoMapLocations);
    shader.setAutoMapBindings(options.autoMapBindings);

    // Bias auto-assigned bindings so the "default binding/set" settings have an
    // effect. setDefaultUniformBlockBinding()/Set() do not exist in this glslang
    // revision, so the shift APIs are the portable equivalent.
    if (options.defaultUniformBinding != 0) {
        const unsigned base = static_cast<unsigned>(options.defaultUniformBinding);
        shader.setShiftBinding(glslang::EResUbo, base);
        shader.setShiftBinding(glslang::EResSsbo, base);
        shader.setShiftBinding(glslang::EResSampler, base);
        shader.setShiftBinding(glslang::EResTexture, base);
        shader.setShiftBinding(glslang::EResImage, base);
        shader.setShiftBinding(glslang::EResUav, base);
    }
    if (options.defaultUniformSet != 0) {
        shader.setShiftBindingForSet(glslang::EResUbo,
                                     static_cast<unsigned>(options.defaultUniformBinding),
                                     static_cast<unsigned>(options.defaultUniformSet));
    }

    if (options.enablePreamble) {
        shader.setPreamble("#define SPALL 1\n#define GLSL_SPIRV_STUDIO 1\n");
    }

    // The limits are copied because GetDefaultResources() returns a pointer to
    // glslang's shared table.
    TBuiltInResource resources = *GetDefaultResources();
    applyResourceLimits(options.limits, &resources);

    const int defaultVersion = options.glslVersion;
    const bool forwardCompatible = false;
    // DisplayErrorColumn is what makes glslang report a column alongside the
    // line, which the problem list and editor jump rely on.
    const EShMessages messages =
        static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules |
                                 EShMsgDisplayErrorColumn);

    if (!shader.parse(&resources, defaultVersion, forwardCompatible, messages)) {
        result.failureTitle = trs(QT_TRANSLATE_NOOP("Compiler", "%1 shader (%2) failed to compile."))
                                  .arg(stageLabel(options.stage), fileName);
        result.failureHint = trs(QT_TRANSLATE_NOOP("Compiler", "See the problem list below: double-clicking an entry jumps to the offending line "
            "and column. Common causes are a missing semicolon, an undeclared variable, or a "
            "#version that does not match the features used."));

        const QString infoLog = QString::fromUtf8(shader.getInfoLog());
        const QString debugLog = QString::fromUtf8(shader.getInfoDebugLog());
        parseInfoLog(infoLog, DiagSource::GlslSyntax, pre.map, fileName, &result);
        if (!debugLog.trimmed().isEmpty()) {
            result.generatorLog = debugLog;
        }
        if (result.diagnostics.isEmpty()) {
            // glslang failed without a parseable message: surface the raw log.
            result.diagnostics.append(
                Diagnostic{Severity::Error, DiagSource::GlslSyntax, 0, 0, fileName,
                           infoLog.trimmed().isEmpty()
                               ? trs(QT_TRANSLATE_NOOP("Compiler", "glslang failed to compile and gave no details."))
                               : infoLog.trimmed()});
        }
        result.recountDiagnostics();
        result.compileMs = timer.elapsed();
        return result;
    }

    // Parse-time warnings are worth surfacing even on success.
    parseInfoLog(QString::fromUtf8(shader.getInfoLog()), DiagSource::GlslSyntax, pre.map, fileName,
                 &result);

    // --- 3. link -------------------------------------------------------------
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages)) {
        result.failureTitle = trs(QT_TRANSLATE_NOOP("Compiler", "%1 shader failed to link.")).arg(stageLabel(options.stage));
        result.failureHint = trs(QT_TRANSLATE_NOOP("Compiler", "A link failure usually means the entry point main() is missing or has the wrong "
            "signature. Check that main() is defined and takes no parameters."));
        parseInfoLog(QString::fromUtf8(program.getInfoLog()), DiagSource::Link, pre.map, fileName,
                     &result);
        if (result.diagnostics.isEmpty()) {
            result.diagnostics.append(Diagnostic{Severity::Error, DiagSource::Link, 0, 0, fileName,
                                                 QString::fromUtf8(program.getInfoLog()).trimmed()});
        }
        result.recountDiagnostics();
        result.compileMs = timer.elapsed();
        return result;
    }

    // --- 4. SPIR-V generation ------------------------------------------------
    glslang::TIntermediate* intermediate = program.getIntermediate(language);
    if (intermediate == nullptr) {
        result.failureTitle =
            trs(QT_TRANSLATE_NOOP("Compiler", "glslang did not produce an intermediate representation for stage %1."))
                .arg(stageLabel(options.stage));
        result.failureHint = trs(QT_TRANSLATE_NOOP("Compiler", "Make sure the Stage selected on the toolbar matches the actual stage of the "
            "source (for example, choose Vertex for a vertex shader)."));
        result.diagnostics.append(Diagnostic{Severity::Error, DiagSource::Link, 0, 0, fileName,
                                             trs(QT_TRANSLATE_NOOP("Compiler", "TIntermediate is null"))});
        result.recountDiagnostics();
        result.compileMs = timer.elapsed();
        return result;
    }

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;
    spvOptions.generateDebugInfo = false;
    // Debug info must NOT be stripped: glslang emits the OpName / OpMemberName
    // instructions that carry every variable and block-member name as debug info,
    // and SPIRV-Reflect reports those names. Stripping them leaves the reflection
    // panel with blank names for every descriptor, member and IO variable.
    spvOptions.stripDebugInfo = false;
    spvOptions.disableOptimizer = true;  // optimization is spirv-opt's job
    spvOptions.optimizeSize = false;
    spvOptions.validate = false;         // spvValidate runs separately, on request

    std::vector<unsigned int> spirv;
    glslang::GlslangToSpv(*intermediate, spirv, &logger, &spvOptions);

    const QString loggerText = QString::fromStdString(logger.getAllMessages());
    if (!loggerText.trimmed().isEmpty()) {
        result.generatorLog = loggerText;
    }

    if (spirv.empty()) {
        result.failureTitle = trs(QT_TRANSLATE_NOOP("Compiler", "glslang produced no SPIR-V words at all."));
        result.failureHint = trs(QT_TRANSLATE_NOOP("Compiler", "This usually means there is no compilable code in the source. Check that a "
            "main() function exists and that the Stage selected on the toolbar is correct."));
        result.diagnostics.append(Diagnostic{Severity::Error, DiagSource::GlslSyntax, 0, 0, fileName,
                                             trs(QT_TRANSLATE_NOOP("Compiler", "The generated SPIR-V is empty"))});
        result.recountDiagnostics();
        result.compileMs = timer.elapsed();
        return result;
    }

    result.spirv.reserve(static_cast<int>(spirv.size()));
    for (unsigned int word : spirv) {
        result.spirv.append(static_cast<quint32>(word));
    }
    result.spirvOptimized = result.spirv;

    // Magic number sanity check: catches a structurally broken module early.
    if (result.spirv.size() < 5 || result.spirv.first() != 0x07230203u) {
        result.failureTitle = trs(QT_TRANSLATE_NOOP("Compiler", "The SPIR-V header produced by glslang is invalid "
                                  "(magic number check failed)."));
        result.failureHint = trs(QT_TRANSLATE_NOOP("Compiler", "This is an internal tool failure. Please report it together with the "
            "current source and the compile log so that it can be tracked down."));
        result.diagnostics.append(Diagnostic{Severity::Error, DiagSource::GlslSyntax, 0, 0, fileName,
                                             trs(QT_TRANSLATE_NOOP("Compiler", "SPIR-V magic is not 0x07230203"))});
        result.recountDiagnostics();
        result.compileMs = timer.elapsed();
        return result;
    }

    // id bound is the 4th word of the header; word 1 carries the version.
    result.idBound = result.spirv.at(3);
    result.versionString = QStringLiteral("%1.%2")
                               .arg((result.spirv.at(1) >> 16) & 0xFFu)
                               .arg((result.spirv.at(1) >> 8) & 0xFFu);

    result.success = true;
    result.recountDiagnostics();
    result.compileMs = timer.elapsed();
    return result;
}

// --- diagnostic parsing ------------------------------------------------------

void GlslCompiler::parseInfoLog(const QString& log,
                                DiagSource source,
                                const LineMap& map,
                                const QString& mainFile,
                                CompileResult* result) const {
    if (!result || log.trimmed().isEmpty()) {
        return;
    }

    // glslang lines look like:
    //   ERROR: 0:12: 'x' : undeclared identifier
    //   WARNING: 0:3: '' : some warning
    // The first number is the source-string index, the second is the line.
    static const QRegularExpression withPosition(
        QStringLiteral(R"(^\s*(ERROR|WARNING|INFO|HINT)\s*:\s*(\d+)\s*:\s*(\d+)\s*:\s*(.*)$)"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression withoutPosition(
        QStringLiteral(R"(^\s*(ERROR|WARNING|INFO|HINT)\s*:\s*(.*)$)"),
        QRegularExpression::CaseInsensitiveOption);

    const QStringList lines = log.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty()) {
            continue;
        }

        Diagnostic diag;
        diag.source = source;

        const QRegularExpressionMatch match = withPosition.match(line);
        if (match.hasMatch()) {
            Severity severity = Severity::Error;
            severityFromToken(match.captured(1), &severity);
            diag.severity = severity;
            diag.line = match.captured(3).toInt();
            diag.column = 0;
            diag.message = match.captured(4).trimmed();
            diag.file = mainFile;

            // glslang appends a column hint to some messages.
            static const QRegularExpression columnHint(
                QStringLiteral(R"(\(\s*col(?:umn)?\s*(\d+)\s*\)|:\s*(\d+)\s*$)"),
                QRegularExpression::CaseInsensitiveOption);
            const QRegularExpressionMatch columnMatch = columnHint.match(diag.message);
            if (columnMatch.hasMatch()) {
                const QString captured = columnMatch.captured(1).isEmpty()
                                             ? columnMatch.captured(2)
                                             : columnMatch.captured(1);
                if (!captured.isEmpty()) {
                    diag.column = captured.toInt();
                }
            }

            // Rewrite expanded coordinates back to the user's file/line.
            IncludePreprocessor::remap(&diag, map, mainFile);
        } else {
            const QRegularExpressionMatch loose = withoutPosition.match(line);
            if (loose.hasMatch()) {
                Severity severity = Severity::Warning;
                severityFromToken(loose.captured(1), &severity);
                diag.severity = severity;
                diag.message = loose.captured(2).trimmed();
            } else {
                // Unparseable: keep the whole text as a global message.
                diag.severity = Severity::Info;
                diag.message = line;
            }
            diag.line = 0;
            diag.column = 0;
            diag.file = mainFile;
        }

        if (diag.message.isEmpty()) {
            diag.message = line;
        }
        result->diagnostics.append(diag);
    }
}

}  // namespace spall
