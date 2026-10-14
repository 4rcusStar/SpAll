// ReflectData.h — plain reflection data produced from a SPIR-V module.
//
// Deliberately free of SPIRV-Reflect types so the UI can consume it without
// including third-party headers. SpirvReflection.cpp fills these in.
#pragma once

#include <QString>
#include <QVector>

namespace spall {

/// A block member (UBO/SSBO/push constant) or a nested struct member.
struct ReflectMember {
    QString name;
    QString typeName;      ///< reconstructed GLSL-ish type, e.g. "mat4", "vec3"
    quint32 offset = 0;    ///< byte offset inside the parent block
    quint32 size = 0;      ///< unpadded size
    quint32 paddedSize = 0;///< size including layout padding
    quint32 arrayDims = 0; ///< number of array dimensions (0 = not an array)
    quint32 arrayStride = 0;
    QVector<ReflectMember> members;  ///< nested members for struct types

    /// True when this member is itself a struct we expanded.
    bool isStruct() const { return !members.isEmpty(); }
};

/// One descriptor binding (uniform buffer, sampler, storage image, ...).
struct DescriptorBinding {
    quint32 set = 0;
    quint32 binding = 0;
    QString typeName;    ///< "uniform buffer", "combined image sampler", ...
    QString name;
    quint32 count = 1;   ///< array element count, 1 when not an array
    quint32 size = 0;    ///< byte size of the bound range
    bool used = false;   ///< SPV_REFLECT_VARIABLE_FLAGS_UNUSED cleared
    QVector<ReflectMember> members;  ///< expanded for UBO/SSBO
};

/// A push-constant block.
struct PushConstantBlock {
    QString name;
    quint32 offset = 0;
    quint32 size = 0;
    QVector<ReflectMember> members;
};

/// A stage input or output variable.
struct StageVar {
    quint32 location = 0;
    quint32 component = 0;
    QString name;
    QString typeName;
    QString format;      ///< VK_FORMAT_* name, or "unknown"
    int builtin = -1;    ///< -1 when not a builtin
    QString builtinName; ///< symbolic builtin name when builtin >= 0

    /// Members of an interface *block* (e.g. gl_PerVertex for gl_Position).
    /// Empty for ordinary variables. Nested blocks are flattened one level, and
    /// only the directly declared members are kept.
    QVector<StageVar> members;

    bool isBuiltin() const { return builtin >= 0; }
};

/// A specialization constant.
struct SpecConstant {
    quint32 constantId = 0;
    QString name;
    QString typeName;
    bool hasDefault = false;
    QString defaultValue;
};

/// Entry-point / execution-model information.
struct ExecutionInfo {
    QString entryPoint = QStringLiteral("main");
    QString executionModel;   ///< "Vertex", "Fragment", "GLCompute", ...
    quint32 localSizeX = 0;
    quint32 localSizeY = 0;
    quint32 localSizeZ = 0;
    quint32 invocations = 0;      ///< geometry only
    quint32 outputVertices = 0;   ///< geometry only
    QStringList executionModes;   ///< human-readable mode names
    QString generator;            ///< SpvReflectGenerator name
    quint32 spirvVersion = 0;     ///< module version word, e.g. 0x00010600
};

/// Complete reflection result for one module.
struct ReflectData {
    QVector<DescriptorBinding>   descriptors;
    QVector<PushConstantBlock>   pushConstants;
    QVector<StageVar>            inputs;
    QVector<StageVar>            outputs;
    QVector<SpecConstant>        specConstants;
    ExecutionInfo                execution;

    /// Total number of interface variables, convenient for "is this empty" checks.
    int interfaceCount() const { return inputs.size() + outputs.size(); }

    /// Render the whole result as an aligned plain-text table for the
    /// "copy as text" button.
    QString toPlainText() const;
};

}  // namespace spall
