// ShaderStage.cpp — stage <-> glslang / file-extension mappings.
#include "ShaderStage.h"

#include <QFileInfo>
#include <QHash>

// glslang's EShLanguage values are stable and the enum is small; mirroring the
// numeric values here keeps glslang's headers out of the UI translation units.
// The values match glslang/Public/ShaderLang.h.
namespace {
enum EShLanguageMirror {
    EShLangVertex = 0,
    EShLangTessControl = 1,
    EShLangTessEvaluation = 2,
    EShLangGeometry = 3,
    EShLangFragment = 4,
    EShLangCompute = 5,
    EShLangRayGen = 6,
    EShLangIntersect = 7,
    EShLangAnyHit = 8,
    EShLangClosestHit = 9,
    EShLangMiss = 10,
    EShLangCallable = 11,
    EShLangTask = 12,
    EShLangMesh = 13,
};
}  // namespace

namespace spall {

namespace {

struct StageInfo {
    ShaderStage stage;
    const char* name;   // UI name
    const char* key;    // glslang-style short key
    const char* suffix; // file suffix with dot
    int glslang;        // EShLanguage
};

// Order defines the order the UI lists stages in.
const StageInfo kStages[] = {
    {ShaderStage::Vertex,         "Vertex",           "vert", ".vert", EShLangVertex},
    {ShaderStage::TessControl,    "Tess Control",     "tesc", ".tesc", EShLangTessControl},
    {ShaderStage::TessEvaluation, "Tess Evaluation",  "tese", ".tese", EShLangTessEvaluation},
    {ShaderStage::Geometry,       "Geometry",         "geom", ".geom", EShLangGeometry},
    {ShaderStage::Fragment,       "Fragment",         "frag", ".frag", EShLangFragment},
    {ShaderStage::Compute,        "Compute",          "comp", ".comp", EShLangCompute},
    {ShaderStage::RayGeneration,  "Ray Generation",   "rgen", ".rgen", EShLangRayGen},
    {ShaderStage::AnyHit,         "Any Hit",          "rahit", ".rahit", EShLangAnyHit},
    {ShaderStage::ClosestHit,     "Closest Hit",      "rchit", ".rchit", EShLangClosestHit},
    {ShaderStage::Miss,           "Miss",             "rmiss", ".rmiss", EShLangMiss},
    {ShaderStage::Intersection,   "Intersection",     "rint", ".rint", EShLangIntersect},
    {ShaderStage::Callable,       "Callable",         "rcall", ".rcall", EShLangCallable},
    {ShaderStage::Task,           "Task",             "task", ".task", EShLangTask},
    {ShaderStage::Mesh,           "Mesh",             "mesh", ".mesh", EShLangMesh},
};

const StageInfo& infoFor(ShaderStage stage) {
    for (const StageInfo& info : kStages) {
        if (info.stage == stage) {
            return info;
        }
    }
    return kStages[0];  // Vertex — unreachable for valid enum values
}

}  // namespace

QStringList allStageNames() {
    QStringList names;
    for (const StageInfo& info : kStages) {
        names << QString::fromLatin1(info.name);
    }
    return names;
}

QString stageName(ShaderStage stage) {
    return QString::fromLatin1(infoFor(stage).name);
}

QString stageKey(ShaderStage stage) {
    return QString::fromLatin1(infoFor(stage).key);
}

QString stageFileSuffix(ShaderStage stage) {
    return QString::fromLatin1(infoFor(stage).suffix);
}

bool stageFromKey(const QString& key, ShaderStage* out) {
    const QString needle = key.trimmed().toLower();
    for (const StageInfo& info : kStages) {
        if (needle == QLatin1String(info.key) || needle == QString::fromLatin1(info.name).toLower()) {
            if (out) {
                *out = info.stage;
            }
            return true;
        }
    }
    // Common aliases people actually type.
    static const QHash<QString, ShaderStage> aliases = {
        {QStringLiteral("vertex"), ShaderStage::Vertex},
        {QStringLiteral("fragment"), ShaderStage::Fragment},
        {QStringLiteral("pixel"), ShaderStage::Fragment},
        {QStringLiteral("glsl"), ShaderStage::Vertex},
        {QStringLiteral("rgen"), ShaderStage::RayGeneration},
        {QStringLiteral("raygen"), ShaderStage::RayGeneration},
        {QStringLiteral("compute"), ShaderStage::Compute},
    };
    const auto it = aliases.constFind(needle);
    if (it != aliases.constEnd()) {
        if (out) {
            *out = it.value();
        }
        return true;
    }
    return false;
}

bool stageFromFileName(const QString& fileName, ShaderStage* out) {
    const QString suffix = QFileInfo(fileName).suffix().toLower();
    if (suffix.isEmpty()) {
        return false;
    }
    for (const StageInfo& info : kStages) {
        // info.suffix includes the leading dot.
        if (suffix == QLatin1String(info.suffix + 1)) {
            if (out) {
                *out = info.stage;
            }
            return true;
        }
    }
    // GLSL also uses .glsl for shared includes; do not guess a stage for those.
    return false;
}

int stageToGlslang(ShaderStage stage) {
    return infoFor(stage).glslang;
}

bool stageIsRayTracing(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::RayGeneration:
        case ShaderStage::AnyHit:
        case ShaderStage::ClosestHit:
        case ShaderStage::Miss:
        case ShaderStage::Intersection:
        case ShaderStage::Callable:
            return true;
        default:
            return false;
    }
}

bool stageIsMeshPipeline(ShaderStage stage) {
    return stage == ShaderStage::Task || stage == ShaderStage::Mesh;
}

}  // namespace spall
