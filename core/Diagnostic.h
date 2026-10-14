// Diagnostic.h — one compiler/validator/optimizer message.
#pragma once

#include <QString>

namespace spall {

/// Message severity, ordered from most to least severe for sorting.
enum class Severity {
    Error = 0,
    Warning = 1,
    Info = 2,
    Hint = 3,
};

/// Which producer emitted the message. Used for the "category" column and for
/// filtering, since e.g. SPIR-V validation errors have no GLSL line number.
enum class DiagSource {
    GlslSyntax = 0,       ///< glslang front-end errors
    Link,                 ///< glslang program link errors
    Preprocess,           ///< our own #include expander
    SpirvValidation,      ///< spvValidate
    SpirvOptimizer,       ///< spvOptimizerRun / message consumer
    StaticHint,           ///< our own heuristic syntax hints
};

/// A single diagnostic. Line and column are 1-based; 0 means "no position"
/// (shown as "Global" in the UI).
struct Diagnostic {
    Severity   severity = Severity::Error;
    DiagSource source = DiagSource::GlslSyntax;
    int        line = 0;
    int        column = 0;
    QString    file;     ///< real file after #include line remapping
    QString    message;

    /// True when the diagnostic points at a specific source position.
    bool hasPosition() const { return line > 0; }
};

/// Short label for the category column, e.g. "GLSL syntax".
QString diagSourceName(DiagSource source);

/// Short label for the severity column, e.g. "Error".
QString severityName(Severity severity);

/// Parse a severity from glslang's leading token ("ERROR", "WARNING", ...).
/// Returns false when the token is not a known severity.
bool severityFromToken(const QString& token, Severity* out);

/// Parse a diagnostic-source label back to its enum, for settings round-trips.
bool diagSourceFromName(const QString& name, DiagSource* out);

/// Heuristic, pre-compilation syntax checks over a whole source text.
///
/// These run before the compiler so obvious mistakes show up while typing. They
/// are deliberately conservative: a false "missing semicolon" is far more
/// annoying than a missed one, so anything that could plausibly continue on the
/// next line is skipped.
namespace hints {

/// Find statements that appear to be missing their terminating ';'.
///
/// Each returned Diagnostic carries a 1-based line number and a Severity::Error
/// (a missing ';' is a hard error, not a style note). The column is the end of
/// the offending line.
QVector<Diagnostic> findMissingSemicolons(const QString& source, const QString& fileName);

}  // namespace hints

}  // namespace spall
