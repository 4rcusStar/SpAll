// SpirvReflection.h — SPIRV-Reflect wrapper producing plain ReflectData.
//
// The SpvReflectShaderModule handle is owned by a RAII wrapper, and every
// enumeration uses the documented two-phase (count, then fill) pattern.
#pragma once

#include <QString>
#include <QVector>

#include <spirv_reflect.h>

#include "Diagnostic.h"
#include "ReflectData.h"

namespace spall {

/// Outcome of a reflection pass.
struct ReflectionResult {
    bool ok = false;
    ReflectData data;
    QVector<Diagnostic> diagnostics;
};

class SpirvReflection {
public:
    /// Reflect a SPIR-V module. Never throws; failures arrive as diagnostics
    /// with a human-readable reason.
    static ReflectionResult reflect(const QVector<quint32>& words);

    // --- enum -> readable string mappings (complete, not partial) -----------

    /// SpvReflectDescriptorType -> "uniform buffer", "combined image sampler", ...
    static QString descriptorTypeName(int type);
    /// SpvReflectFormat -> "VK_FORMAT_R32G32B32_SFLOAT", ...
    static QString formatName(int format);
    /// SpvExecutionModel -> "Vertex", "GLCompute", ...
    static QString executionModelName(int model);
    /// SpvReflectGenerator -> "Khronos Glslang Reference Front End", ...
    static QString generatorName(int generator);
    /// SpvBuiltIn -> "Position", "FragCoord", ...
    static QString builtinName(int builtin);
    /// SpvReflectResult -> readable text, for error reporting.
    static QString resultName(int result);

    /// Reconstruct a GLSL-ish type name from type flags and traits, e.g. "vec4",
    /// "mat4", "float[4]".
    static QString typeNameFromDescription(const SpvReflectTypeDescription& type);
};

}  // namespace spall
