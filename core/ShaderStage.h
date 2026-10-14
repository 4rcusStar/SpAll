// ShaderStage.h — shader stage enum plus its mappings to glslang and SPIR-V.
//
// This header deliberately includes no glslang or SPIR-V headers: the mapping
// functions are implemented in ShaderStage.cpp so that UI code can name a stage
// without dragging third-party include paths into every translation unit.
#pragma once

#include <QString>
#include <QStringList>

namespace spall {

/// Pipeline stage a source file is compiled as.
enum class ShaderStage {
    Vertex,
    TessControl,
    TessEvaluation,
    Geometry,
    Fragment,
    Compute,
    RayGeneration,
    AnyHit,
    ClosestHit,
    Miss,
    Intersection,
    Callable,
    Task,
    Mesh,
};

/// All stages, in the order the UI lists them.
QStringList allStageNames();

/// Human-readable name, e.g. "Vertex" or "Tess Control".
QString stageName(ShaderStage stage);

/// Lower-case machine name matching glslang's EShLanguage spelling,
/// e.g. "vert", "tesc", "rgen".
QString stageKey(ShaderStage stage);

/// Default file suffix including the dot, e.g. ".vert".
QString stageFileSuffix(ShaderStage stage);

/// Parse a stage from its key ("vert", "frag", ...) or full name. Returns false
/// when the text is not recognised.
bool stageFromKey(const QString& key, ShaderStage* out);

/// Best-effort stage guess from a file name's extension.
bool stageFromFileName(const QString& fileName, ShaderStage* out);

/// glslang EShLanguage value for the stage, returned as int so this header stays
/// free of glslang includes. Callers pass it straight to the glslang API.
int stageToGlslang(ShaderStage stage);

/// GLSL #version directive for the given stage's SPIR-V flavor.
bool stageIsRayTracing(ShaderStage stage);
bool stageIsMeshPipeline(ShaderStage stage);

}  // namespace spall
