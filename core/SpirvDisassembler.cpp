// SpirvDisassembler.cpp — SPIRV-Tools C API wrapper with RAII ownership.
#include "SpirvDisassembler.h"

#include <QCoreApplication>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>

#include <cstring>
#include <memory>

#include <spirv-tools/libspirv.h>

#if SPALL_HAVE_SPIRV_OPT
#include <spirv-tools/optimizer.hpp>
#endif

namespace spall {

static_assert(sizeof(quint32) == sizeof(uint32_t), "SPIR-V words must be 32-bit");

namespace {

// Localised string for this translation context.
//
// A macro rather than a function on purpose: lupdate parses sources statically
// and cannot see through a call that passes the text on to
// QCoreApplication::translate(). Spelling the lookup out here means every trs(QT_TRANSLATE_NOOP("Disassembler", ...))
// call site is extracted automatically, while the runtime behaviour is unchanged.
/// Localised string for this translation context.
///
/// The text is passed through QT_TRANSLATE_NOOP at every call site, which is what
/// lets lupdate extract it statically; this function then performs the runtime
/// lookup. core/ has no QObject, so tr() is not available there.
QString trs(const char* text) {
    return QCoreApplication::translate("Disassembler", text);
}

// --- RAII wrappers -----------------------------------------------------------

struct ContextDeleter {
    void operator()(spv_context context) const noexcept {
        if (context != nullptr) {
            spvContextDestroy(context);
        }
    }
};

struct BinaryDeleter {
    void operator()(spv_binary binary) const noexcept {
        if (binary != nullptr) {
            spvBinaryDestroy(binary);
        }
    }
};

struct TextDeleter {
    void operator()(spv_text text) const noexcept {
        if (text != nullptr) {
            spvTextDestroy(text);
        }
    }
};

struct DiagnosticDeleter {
    void operator()(spv_diagnostic diagnostic) const noexcept {
        if (diagnostic != nullptr) {
            spvDiagnosticDestroy(diagnostic);
        }
    }
};

using ContextPtr = std::unique_ptr<spv_context_t, ContextDeleter>;
using BinaryPtr = std::unique_ptr<spv_binary_t, BinaryDeleter>;
using TextPtr = std::unique_ptr<spv_text_t, TextDeleter>;
using DiagnosticPtr = std::unique_ptr<spv_diagnostic_t, DiagnosticDeleter>;

/// Create a context, returning nullptr on failure (spvContextCreate has no
/// out-parameter for errors).
ContextPtr makeContext(spv_target_env env) {
    return ContextPtr(spvContextCreate(env));
}

/// Build a binary view over our word vector. The struct only describes the
/// caller's buffer, so it owns nothing and needs no cleanup.
spv_binary_t makeBinaryView(const QVector<quint32>& words) {
    spv_binary_t view{};
    view.code = const_cast<uint32_t*>(reinterpret_cast<const uint32_t*>(words.constData()));
    view.wordCount = static_cast<size_t>(words.size());
    return view;
}

/// Turn an spv_diagnostic into our Diagnostic type.
Diagnostic fromSpvDiagnostic(const spv_diagnostic_t* diagnostic, DiagSource source) {
    Diagnostic out;
    out.source = source;
    if (diagnostic == nullptr) {
        out.severity = Severity::Error;
        out.message = trs(QT_TRANSLATE_NOOP("Disassembler", "SPIRV-Tools returned an empty diagnostic message."));
        return out;
    }
    out.severity = Severity::Error;
    out.line = static_cast<int>(diagnostic->position.line);
    out.column = static_cast<int>(diagnostic->position.column);
    if (diagnostic->error != nullptr) {
        out.message = QString::fromUtf8(diagnostic->error).trimmed();
    }
    if (out.message.isEmpty()) {
        out.message = trs(QT_TRANSLATE_NOOP("Disassembler", "SPIRV-Tools reported an error with no description."));
    }
    // Validation errors index the instruction stream rather than GLSL lines; make
    // that explicit instead of pretending the number is a source line.
    if (diagnostic->position.index > 0) {
        out.message += trs(QT_TRANSLATE_NOOP("Disassembler", " (instruction index %1)")).arg(diagnostic->position.index);
    }
    return out;
}

/// The SPIR-V environment name used in messages.
QString envLabel(spv_target_env env) {
    const char* name = spvTargetEnvDescription(env);
    return name != nullptr ? QString::fromUtf8(name) : QStringLiteral("unknown");
}

// --- statistics callback -----------------------------------------------------

struct StatCollector {
    SpirvStats* stats = nullptr;
};

spv_result_t countInstruction(void* user_data,
                              const spv_parsed_instruction_t* instruction) {
    auto* collector = static_cast<StatCollector*>(user_data);
    if (collector == nullptr || collector->stats == nullptr || instruction == nullptr) {
        return SPV_SUCCESS;
    }
    collector->stats->instructionCount += 1;
    // spvOpcodeString takes the raw numeric opcode in this SPIRV-Tools version.
    const char* name = spvOpcodeString(instruction->opcode);
    if (name != nullptr) {
        const QString key = QString::fromUtf8(name);
        collector->stats->opcodeCounts[key] = collector->stats->opcodeCounts.value(key) + 1;
    }
    return SPV_SUCCESS;
}

}  // namespace

namespace {

// The optimizer message consumer must be a plain function pointer, so collected
// messages go to a process-global sink that the caller drains after each run.
QMutex g_optimizerLogMutex;
QStringList g_optimizerLog;

void optimizerMessageConsumer(spv_message_level_t /*level*/,
                              const char* /*source*/,
                              const spv_position_t* /*position*/,
                              const char* message) {
    if (message == nullptr) {
        return;
    }
    const QMutexLocker locker(&g_optimizerLogMutex);
    g_optimizerLog.append(QString::fromUtf8(message).trimmed());
}

/// Take and clear whatever the consumer collected.
QString drainOptimizerLog() {
    const QMutexLocker locker(&g_optimizerLogMutex);
    const QString text = g_optimizerLog.join(QLatin1Char('\n'));
    g_optimizerLog.clear();
    return text;
}

}  // namespace

// --- option translation ------------------------------------------------------

int SpirvDisassembler::toTargetEnv(TargetEnv env) {
    switch (env) {
        case TargetEnv::Vulkan10:    return SPV_ENV_VULKAN_1_0;
        case TargetEnv::Vulkan11:    return SPV_ENV_VULKAN_1_1;
        case TargetEnv::Vulkan12:    return SPV_ENV_VULKAN_1_2;
        case TargetEnv::Vulkan13:    return SPV_ENV_VULKAN_1_3;
        case TargetEnv::Universal10: return SPV_ENV_UNIVERSAL_1_0;
        case TargetEnv::Universal13: return SPV_ENV_UNIVERSAL_1_3;
        case TargetEnv::Universal16: return SPV_ENV_UNIVERSAL_1_6;
    }
    return SPV_ENV_VULKAN_1_3;
}

unsigned SpirvDisassembler::toDisasmOptions(unsigned flags) {
    // Only bits that genuinely exist in spv_binary_to_text_options_t are mapped;
    // anything else is ignored rather than invented.
    unsigned options = SPV_BINARY_TO_TEXT_OPTION_NONE;
    if (flags & DisasmIndent)         options |= SPV_BINARY_TO_TEXT_OPTION_INDENT;
    if (flags & DisasmFriendlyNames)  options |= SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;
    if (flags & DisasmComment)        options |= SPV_BINARY_TO_TEXT_OPTION_COMMENT;
    if (flags & DisasmNoHeader)       options |= SPV_BINARY_TO_TEXT_OPTION_NO_HEADER;
    if (flags & DisasmShowByteOffset) options |= SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET;
    if (flags & DisasmNestedIndent)   options |= SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT;
    if (flags & DisasmReorderBlocks)  options |= SPV_BINARY_TO_TEXT_OPTION_REORDER_BLOCKS;
    return options;
}

bool SpirvDisassembler::hasValidMagic(const QVector<quint32>& words) {
    return words.size() >= 5 && words.at(0) == 0x07230203u;
}

QString SpirvDisassembler::moduleVersionString(const QVector<quint32>& words) {
    if (words.size() < 2) {
        return trs(QT_TRANSLATE_NOOP("Disassembler", "Unknown"));
    }
    const quint32 version = words.at(1);
    const quint32 major = (version >> 16) & 0xFFu;
    const quint32 minor = (version >> 8) & 0xFFu;
    return QStringLiteral("%1.%2").arg(major).arg(minor);
}

// --- disassembly -------------------------------------------------------------

DisassemblyResult SpirvDisassembler::disassemble(const QVector<quint32>& words,
                                                 TargetEnv env,
                                                 unsigned flags) {
    DisassemblyResult result;

    if (words.isEmpty()) {
        result.diagnostics.append(Diagnostic{Severity::Error, DiagSource::SpirvValidation, 0, 0, {},
                                             trs(QT_TRANSLATE_NOOP("Disassembler", "There is no SPIR-V data to disassemble."))});
        return result;
    }

    const spv_target_env target = static_cast<spv_target_env>(toTargetEnv(env));
    ContextPtr context = makeContext(target);
    if (!context) {
        result.diagnostics.append(Diagnostic{
            Severity::Error, DiagSource::SpirvValidation, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Disassembler", "Could not create a SPIRV-Tools context (target environment %1)."))
                .arg(envLabel(target))});
        return result;
    }

    // The view only describes the caller's buffer, so nothing to own.
    const spv_binary_t binary = makeBinaryView(words);

    spv_text text = nullptr;
    spv_diagnostic diagnostic = nullptr;
    // spvBinaryToText takes the raw pointer plus word count in this version.
    const spv_result_t status = spvBinaryToText(context.get(),
                                                binary.code,
                                                binary.wordCount,
                                                toDisasmOptions(flags),
                                                &text,
                                                &diagnostic);
    TextPtr textHolder(text);
    DiagnosticPtr diagnosticHolder(diagnostic);

    if (status != SPV_SUCCESS || text == nullptr) {
        Diagnostic diag = fromSpvDiagnostic(diagnostic, DiagSource::SpirvValidation);
        diag.message = trs(QT_TRANSLATE_NOOP("Disassembler", "Disassembly failed: %1")).arg(diag.message);
        result.diagnostics.append(diag);
        return result;
    }

    if (text->str != nullptr && text->length > 0) {
        result.text = QString::fromUtf8(text->str, static_cast<int>(text->length));
    }
    result.ok = true;
    return result;
}

// --- validation --------------------------------------------------------------

ValidationResult SpirvDisassembler::validate(const QVector<quint32>& words, TargetEnv env) {
    ValidationResult result;

    if (words.isEmpty()) {
        result.diagnostics.append(Diagnostic{Severity::Error, DiagSource::SpirvValidation, 0, 0, {},
                                             trs(QT_TRANSLATE_NOOP("Disassembler", "There is no SPIR-V data to validate."))});
        return result;
    }

    const spv_target_env target = static_cast<spv_target_env>(toTargetEnv(env));
    ContextPtr context = makeContext(target);
    if (!context) {
        result.diagnostics.append(Diagnostic{
            Severity::Error, DiagSource::SpirvValidation, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Disassembler", "Could not create a SPIRV-Tools context (target environment %1)."))
                .arg(envLabel(target))});
        return result;
    }

    std::unique_ptr<spv_validator_options_t, void (*)(spv_validator_options_t*)> options(
        spvValidatorOptionsCreate(), [](spv_validator_options_t* value) {
            if (value != nullptr) {
                spvValidatorOptionsDestroy(value);
            }
        });
    if (!options) {
        result.diagnostics.append(Diagnostic{
            Severity::Error, DiagSource::SpirvValidation, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Disassembler", "Could not create the validator options object."))});
        return result;
    }
    spvValidatorOptionsSetRelaxBlockLayout(options.get(), false);

    // spv_const_binary is a non-const pointer typedef (spv_const_binary_t*), so
    // the view cannot itself be const: pass a mutable struct's address.
    const spv_binary_t storage = makeBinaryView(words);
    spv_const_binary_t view{storage.code, storage.wordCount};
    spv_diagnostic diagnostic = nullptr;
    const spv_result_t status = spvValidateWithOptions(context.get(), options.get(), &view,
                                                       &diagnostic);
    DiagnosticPtr diagnosticHolder(diagnostic);

    if (status != SPV_SUCCESS) {
        Diagnostic diag = fromSpvDiagnostic(diagnostic, DiagSource::SpirvValidation);
        diag.message = trs(QT_TRANSLATE_NOOP("Disassembler", "SPIR-V validation failed: %1")).arg(diag.message);
        result.diagnostics.append(diag);
        return result;
    }
    result.ok = true;
    return result;
}

// --- optimization ------------------------------------------------------------

OptimizationResult SpirvDisassembler::optimize(const QVector<quint32>& words,
                                               TargetEnv env,
                                               OptimizationLevel level) {
    OptimizationResult result;
    result.words = words;  // unchanged unless passes actually run

    if (level == OptimizationLevel::None) {
        result.ok = true;
        return result;
    }

#if !SPALL_HAVE_SPIRV_OPT
    result.ok = true;
    result.diagnostics.append(Diagnostic{
        Severity::Warning, DiagSource::SpirvOptimizer, 0, 0, {},
        trs(QT_TRANSLATE_NOOP("Disassembler", "This build does not link SPIRV-Tools-opt, so optimization was skipped. "
            "To enable it, make sure the SPIRV-Tools-opt target is found at build time."))});
    return result;
#else
    if (words.isEmpty()) {
        result.diagnostics.append(Diagnostic{Severity::Error, DiagSource::SpirvOptimizer, 0, 0, {},
                                             trs(QT_TRANSLATE_NOOP("Disassembler", "There is no SPIR-V data to optimize."))});
        return result;
    }

    const spv_target_env target = static_cast<spv_target_env>(toTargetEnv(env));

    spv_optimizer_options optimizerOptions = spvOptimizerOptionsCreate();
    if (optimizerOptions == nullptr) {
        result.diagnostics.append(Diagnostic{
            Severity::Error, DiagSource::SpirvOptimizer, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Disassembler", "Could not create the optimizer options object."))});
        return result;
    }
    std::unique_ptr<spv_optimizer_options_t, void (*)(spv_optimizer_options_t*)> optionsHolder(
        optimizerOptions, [](spv_optimizer_options_t* value) {
            if (value != nullptr) {
                spvOptimizerOptionsDestroy(value);
            }
        });

    // Collect optimizer chatter for the log panel instead of letting it print.
    spvOptimizerOptionsSetRunValidator(optimizerOptions, false);
    spvOptimizerOptionsSetPreserveBindings(optimizerOptions, true);

    spv_optimizer_t* rawOptimizer = spvOptimizerCreate(target);
    if (rawOptimizer == nullptr) {
        result.diagnostics.append(Diagnostic{
            Severity::Error, DiagSource::SpirvOptimizer, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Disassembler", "Could not create an optimizer for environment %1.")).arg(envLabel(target))});
        return result;
    }
    std::unique_ptr<spv_optimizer_t, void (*)(spv_optimizer_t*)> optimizer(
        rawOptimizer, [](spv_optimizer_t* value) {
            if (value != nullptr) {
                spvOptimizerDestroy(value);
            }
        });

    spvOptimizerSetMessageConsumer(optimizer.get(), optimizerMessageConsumer);
    (void)drainOptimizerLog();  // discard anything left over from an earlier run

    switch (level) {
        case OptimizationLevel::Performance:
            spvOptimizerRegisterPerformancePasses(optimizer.get());
            break;
        case OptimizationLevel::Size:
            spvOptimizerRegisterSizePasses(optimizer.get());
            break;
        case OptimizationLevel::None:
            break;
    }

    spv_binary optimized = nullptr;
    spv_diagnostic diagnostic = nullptr;
    const spv_result_t status = spvOptimizerRun(optimizer.get(),
                                               reinterpret_cast<const uint32_t*>(words.constData()),
                                               static_cast<size_t>(words.size()),
                                               &optimized,
                                               optimizerOptions);
    BinaryPtr optimizedHolder(optimized);
    DiagnosticPtr diagnosticHolder(diagnostic);

    result.log = drainOptimizerLog();

    if (status != SPV_SUCCESS || optimized == nullptr) {
        Diagnostic diag = fromSpvDiagnostic(diagnostic, DiagSource::SpirvOptimizer);
        diag.message = trs(QT_TRANSLATE_NOOP("Disassembler", "Optimization failed: %1")).arg(diag.message);
        result.diagnostics.append(diag);
        return result;
    }

    if (optimized->wordCount < 5 || optimized->code == nullptr) {
        result.diagnostics.append(Diagnostic{Severity::Error, DiagSource::SpirvOptimizer, 0, 0, {},
                                             trs(QT_TRANSLATE_NOOP("Disassembler", "The module returned by the optimizer is too "
                                                 "short; its result was ignored."))});
        return result;
    }

    result.words.clear();
    result.words.reserve(static_cast<int>(optimized->wordCount));
    for (size_t i = 0; i < optimized->wordCount; ++i) {
        result.words.append(static_cast<quint32>(optimized->code[i]));
    }

    if (!hasValidMagic(result.words)) {
        result.diagnostics.append(Diagnostic{
            Severity::Error, DiagSource::SpirvOptimizer, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Disassembler", "The module produced by the optimizer has an invalid magic number, so the "
                "unoptimized result was kept."))});
        result.words = words;
        return result;
    }

    result.ok = true;
    return result;
#endif
}

// --- statistics --------------------------------------------------------------

SpirvStats SpirvDisassembler::collectStatistics(const QVector<quint32>& words, TargetEnv env) {
    SpirvStats stats;
    stats.wordCount = static_cast<quint32>(words.size());
    if (!hasValidMagic(words)) {
        return stats;
    }
    stats.idBound = words.at(3);
    stats.version = words.at(1);

    const spv_target_env target = static_cast<spv_target_env>(toTargetEnv(env));
    ContextPtr context = makeContext(target);
    if (!context) {
        return stats;
    }

    StatCollector collector{&stats};
    spv_diagnostic diagnostic = nullptr;
    const spv_result_t status = spvBinaryParse(context.get(),
                                               &collector,
                                               reinterpret_cast<const uint32_t*>(words.constData()),
                                               static_cast<size_t>(words.size()),
                                               nullptr,          // no header callback
                                               countInstruction,
                                               &diagnostic);
    DiagnosticPtr diagnosticHolder(diagnostic);
    if (status != SPV_SUCCESS) {
        // Statistics are best-effort; the caller still has the word count.
        stats.opcodeCounts.clear();
    }
    return stats;
}

}  // namespace spall
