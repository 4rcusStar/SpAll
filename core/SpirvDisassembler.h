// SpirvDisassembler.h — SPIR-V disassembly, validation, optimization, statistics.
//
// All SPIRV-Tools C handles (spv_context / spv_binary / spv_text /
// spv_diagnostic / spv_optimizer) are owned by RAII wrappers, so no code path
// can leak them.
#pragma once

#include <QString>
#include <QVector>

#include "CompileOptions.h"
#include "CompileResult.h"
#include "Diagnostic.h"

namespace spall {

/// Result of an spvBinaryToText call.
struct DisassemblyResult {
    bool ok = false;
    QString text;
    QVector<Diagnostic> diagnostics;
};

/// Result of an spvValidateWithOptions call.
struct ValidationResult {
    bool ok = false;
    QVector<Diagnostic> diagnostics;
};

/// Result of an optimization run.
struct OptimizationResult {
    bool ok = false;
    QVector<quint32> words;  ///< optimized module; empty on failure
    QVector<Diagnostic> diagnostics;
    QString log;             ///< optimizer message-consumer output
};

class SpirvDisassembler {
public:
    /// Translate our TargetEnv enum onto the real spv_target_env.
    static int toTargetEnv(TargetEnv env);
    /// Translate our DisasmFlag bits onto spv_binary_to_text_options_t.
    static unsigned toDisasmOptions(unsigned flags);

    /// Disassemble a module. `flags` is a bitwise OR of DisasmFlag values.
    static DisassemblyResult disassemble(const QVector<quint32>& words,
                                         TargetEnv env,
                                         unsigned flags);

    /// Validate a module against `env`.
    static ValidationResult validate(const QVector<quint32>& words, TargetEnv env);

    /// Run the requested optimization passes. Returns the input unchanged when
    /// `level` is OptimizationLevel::None.
    static OptimizationResult optimize(const QVector<quint32>& words,
                                       TargetEnv env,
                                       OptimizationLevel level);

    /// Per-instruction counts gathered through spvBinaryParse.
    static SpirvStats collectStatistics(const QVector<quint32>& words, TargetEnv env);

    /// "1.6" style module version, derived from the SPIR-V header word.
    static QString moduleVersionString(const QVector<quint32>& words);

    /// SPIR-V magic number check; false when `words` cannot be a module.
    static bool hasValidMagic(const QVector<quint32>& words);
};

}  // namespace spall
