// CompileResult.h — the complete outcome of one compile request.
#pragma once

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

#include <optional>

#include "CompileOptions.h"
#include "Diagnostic.h"
#include "ReflectData.h"

namespace spall {

/// Instruction-level statistics gathered while parsing the binary. Collecting
/// them from the parsed binary is more reliable than counting text lines.
struct SpirvStats {
    quint32 wordCount = 0;
    quint32 instructionCount = 0;
    quint32 idBound = 0;
    quint32 version = 0;  ///< module version word, e.g. 0x00010600
    /// opcode mnemonic ("OpStore") -> occurrence count.
    QHash<QString, quint32> opcodeCounts;
};

/// Result of one compilation, including preprocessing, codegen, validation,
/// optimization, disassembly and reflection.
struct CompileResult {
    // --- status -------------------------------------------------------------
    bool success = false;

    /// Two-part user-facing failure text: what happened, and what to try.
    QString failureTitle;
    QString failureHint;

    // --- artifacts ----------------------------------------------------------
    QVector<quint32> spirv;           ///< module as produced by glslang
    QVector<quint32> spirvOptimized;  ///< after spvOptimizerRun (== spirv when off)
    QString assembly;                 ///< disassembly of spirvOptimized
    QByteArray optimizedBytes() const;

    // --- diagnostics --------------------------------------------------------
    QVector<Diagnostic> diagnostics;

    // --- reflection ---------------------------------------------------------
    std::optional<ReflectData> reflection;

    // --- statistics ---------------------------------------------------------
    SpirvStats stats;             ///< of the optimized module
    SpirvStats statsBeforeOpt;    ///< of the unoptimized module
    quint32 idBound = 0;          ///< module id bound (SPIR-V header word 3)
    qint64 compileMs = 0;         ///< wall-clock time of the whole request
    QString versionString;        ///< "1.6"
    QString generatorLog;         ///< SpvBuildLogger output, for the log panel
    QString optimizerLog;         ///< optimizer message-consumer output

    // --- error source accounting -------------------------------------------
    int errorCount = 0;
    int warningCount = 0;
    int hintCount = 0;

    /// Count diagnostics by severity; call after filling `diagnostics`.
    void recountDiagnostics();

    /// Diagnostics sorted by severity then position, for the problem list.
    QVector<Diagnostic> sortedDiagnostics() const;
};

}  // namespace spall
