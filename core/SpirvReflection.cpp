// SpirvReflection.cpp — SPIRV-Reflect driven reflection.
#include "SpirvReflection.h"

#include <QCoreApplication>
#include <QHash>
#include <QStringList>

#include <memory>

#include <spirv_reflect.h>

namespace spall {

namespace {

// Localised string for this translation context.
//
// A macro rather than a function on purpose: lupdate parses sources statically
// and cannot see through a call that passes the text on to
// QCoreApplication::translate(). Spelling the lookup out here means every trs(QT_TRANSLATE_NOOP("Reflection", ...))
// call site is extracted automatically, while the runtime behaviour is unchanged.
/// Localised string for this translation context.
///
/// The text is passed through QT_TRANSLATE_NOOP at every call site, which is what
/// lets lupdate extract it statically; this function then performs the runtime
/// lookup. core/ has no QObject, so tr() is not available there.
QString trs(const char* text) {
    return QCoreApplication::translate("Reflection", text);
}

/// RAII owner for a reflected module: guarantees spvReflectDestroyShaderModule.
class ModuleOwner {
public:
    ModuleOwner() = default;
    ~ModuleOwner() {
        if (m_created) {
            spvReflectDestroyShaderModule(&m_module);
        }
    }

    ModuleOwner(const ModuleOwner&) = delete;
    ModuleOwner& operator=(const ModuleOwner&) = delete;

    SpvReflectShaderModule* module() { return &m_module; }

    /// Reflect `words`; returns the raw result code for readable reporting.
    SpvReflectResult create(const QVector<quint32>& words) {
        const SpvReflectResult result = spvReflectCreateShaderModule(
            static_cast<size_t>(words.size()) * sizeof(quint32), words.constData(), &m_module);
        m_created = (result == SPV_REFLECT_RESULT_SUCCESS);
        return result;
    }

private:
    SpvReflectShaderModule m_module{};
    bool m_created = false;
};

QString fromConstChar(const char* text) {
    return text != nullptr ? QString::fromUtf8(text) : QString();
}

/// Reconstruct a GLSL type name for a block member / interface variable.
QString memberTypeName(const SpvReflectTypeDescription* type,
                       const SpvReflectNumericTraits& numeric,
                       const SpvReflectArrayTraits& array) {
    QString base;
    if (type != nullptr && type->type_name != nullptr) {
        base = QString::fromUtf8(type->type_name);
    }
    if (base.isEmpty()) {
        // Fall back to reconstructing from the numeric traits.
        const bool isFloat = (numeric.scalar.width == 64) ? true : true;
        Q_UNUSED(isFloat);
        QString scalar;
        switch (numeric.scalar.width) {
            case 64: scalar = QStringLiteral("double"); break;
            case 32: scalar = QStringLiteral("float"); break;
            case 16: scalar = QStringLiteral("float16_t"); break;
            case 8:  scalar = QStringLiteral("int8_t"); break;
            default: scalar = QStringLiteral("float"); break;
        }
        if (numeric.vector.component_count > 1) {
            base = QStringLiteral("vec%1").arg(numeric.vector.component_count);
        } else {
            base = scalar;
        }
    }

    // Append array dimensions from the outermost inward.
    if (array.dims_count > 0) {
        for (uint32_t i = 0; i < array.dims_count; ++i) {
            const uint32_t dim = array.dims[i];
            if (dim == 0) {
                base += QStringLiteral("[]");  // runtime array
            } else {
                base += QStringLiteral("[%1]").arg(dim);
            }
        }
    }
    return base;
}

/// Convert one SpvReflectBlockVariable (and its children) into our ReflectMember.
ReflectMember toMember(const SpvReflectBlockVariable& variable, int depth) {
    ReflectMember member;
    member.name = fromConstChar(variable.name);
    member.offset = variable.offset;
    member.size = variable.size;
    member.paddedSize = variable.padded_size;
    member.arrayDims = variable.array.dims_count;
    member.arrayStride = variable.array.stride;
    member.typeName = memberTypeName(variable.type_description, variable.numeric, variable.array);

    // Guard against pathological/self-referential structs.
    constexpr int kMaxDepth = 8;
    if (depth < kMaxDepth && variable.member_count > 0 && variable.members != nullptr) {
        member.members.reserve(static_cast<int>(variable.member_count));
        for (uint32_t i = 0; i < variable.member_count; ++i) {
            member.members.append(toMember(variable.members[i], depth + 1));
        }
    }
    return member;
}

ReflectMember blockToMember(const SpvReflectBlockVariable& block) {
    return toMember(block, 0);
}

}  // namespace

// --- enum mappings -----------------------------------------------------------

QString SpirvReflection::descriptorTypeName(int type) {
    switch (static_cast<SpvReflectDescriptorType>(type)) {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
            return QStringLiteral("sampler");
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return QStringLiteral("combined image sampler");
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return QStringLiteral("sampled image");
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return QStringLiteral("storage image");
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            return QStringLiteral("uniform texel buffer");
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return QStringLiteral("storage texel buffer");
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return QStringLiteral("uniform buffer");
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return QStringLiteral("storage buffer");
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            return QStringLiteral("uniform buffer (dynamic)");
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return QStringLiteral("storage buffer (dynamic)");
        case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return QStringLiteral("input attachment");
        case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return QStringLiteral("acceleration structure");
    }
    return trs(QT_TRANSLATE_NOOP("Reflection", "Unknown (%1)")).arg(type);
}

QString SpirvReflection::formatName(int format) {
    switch (static_cast<SpvReflectFormat>(format)) {
        case SPV_REFLECT_FORMAT_UNDEFINED:           return QStringLiteral("VK_FORMAT_UNDEFINED");
        case SPV_REFLECT_FORMAT_R16_UINT:            return QStringLiteral("VK_FORMAT_R16_UINT");
        case SPV_REFLECT_FORMAT_R16_SINT:            return QStringLiteral("VK_FORMAT_R16_SINT");
        case SPV_REFLECT_FORMAT_R16_SFLOAT:          return QStringLiteral("VK_FORMAT_R16_SFLOAT");
        case SPV_REFLECT_FORMAT_R16G16_UINT:         return QStringLiteral("VK_FORMAT_R16G16_UINT");
        case SPV_REFLECT_FORMAT_R16G16_SINT:         return QStringLiteral("VK_FORMAT_R16G16_SINT");
        case SPV_REFLECT_FORMAT_R16G16_SFLOAT:       return QStringLiteral("VK_FORMAT_R16G16_SFLOAT");
        case SPV_REFLECT_FORMAT_R16G16B16_UINT:      return QStringLiteral("VK_FORMAT_R16G16B16_UINT");
        case SPV_REFLECT_FORMAT_R16G16B16_SINT:      return QStringLiteral("VK_FORMAT_R16G16B16_SINT");
        case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT:    return QStringLiteral("VK_FORMAT_R16G16B16_SFLOAT");
        case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:   return QStringLiteral("VK_FORMAT_R16G16B16A16_UINT");
        case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:   return QStringLiteral("VK_FORMAT_R16G16B16A16_SINT");
        case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT: return QStringLiteral("VK_FORMAT_R16G16B16A16_SFLOAT");
        case SPV_REFLECT_FORMAT_R32_UINT:            return QStringLiteral("VK_FORMAT_R32_UINT");
        case SPV_REFLECT_FORMAT_R32_SINT:            return QStringLiteral("VK_FORMAT_R32_SINT");
        case SPV_REFLECT_FORMAT_R32_SFLOAT:          return QStringLiteral("VK_FORMAT_R32_SFLOAT");
        case SPV_REFLECT_FORMAT_R32G32_UINT:         return QStringLiteral("VK_FORMAT_R32G32_UINT");
        case SPV_REFLECT_FORMAT_R32G32_SINT:         return QStringLiteral("VK_FORMAT_R32G32_SINT");
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:       return QStringLiteral("VK_FORMAT_R32G32_SFLOAT");
        case SPV_REFLECT_FORMAT_R32G32B32_UINT:      return QStringLiteral("VK_FORMAT_R32G32B32_UINT");
        case SPV_REFLECT_FORMAT_R32G32B32_SINT:      return QStringLiteral("VK_FORMAT_R32G32B32_SINT");
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:    return QStringLiteral("VK_FORMAT_R32G32B32_SFLOAT");
        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:   return QStringLiteral("VK_FORMAT_R32G32B32A32_UINT");
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:   return QStringLiteral("VK_FORMAT_R32G32B32A32_SINT");
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return QStringLiteral("VK_FORMAT_R32G32B32A32_SFLOAT");
        case SPV_REFLECT_FORMAT_R64_UINT:            return QStringLiteral("VK_FORMAT_R64_UINT");
        case SPV_REFLECT_FORMAT_R64_SINT:            return QStringLiteral("VK_FORMAT_R64_SINT");
        case SPV_REFLECT_FORMAT_R64_SFLOAT:          return QStringLiteral("VK_FORMAT_R64_SFLOAT");
        case SPV_REFLECT_FORMAT_R64G64_UINT:         return QStringLiteral("VK_FORMAT_R64G64_UINT");
        case SPV_REFLECT_FORMAT_R64G64_SINT:         return QStringLiteral("VK_FORMAT_R64G64_SINT");
        case SPV_REFLECT_FORMAT_R64G64_SFLOAT:       return QStringLiteral("VK_FORMAT_R64G64_SFLOAT");
        case SPV_REFLECT_FORMAT_R64G64B64_UINT:      return QStringLiteral("VK_FORMAT_R64G64B64_UINT");
        case SPV_REFLECT_FORMAT_R64G64B64_SINT:      return QStringLiteral("VK_FORMAT_R64G64B64_SINT");
        case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:    return QStringLiteral("VK_FORMAT_R64G64B64_SFLOAT");
        case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:   return QStringLiteral("VK_FORMAT_R64G64B64A64_UINT");
        case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:   return QStringLiteral("VK_FORMAT_R64G64B64A64_SINT");
        case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: return QStringLiteral("VK_FORMAT_R64G64B64A64_SFLOAT");
    }
    return QStringLiteral("VK_FORMAT_UNDEFINED");
}

QString SpirvReflection::executionModelName(int model) {
    switch (static_cast<SpvExecutionModel>(model)) {
        case SpvExecutionModelVertex:                 return QStringLiteral("Vertex");
        case SpvExecutionModelTessellationControl:    return QStringLiteral("TessellationControl");
        case SpvExecutionModelTessellationEvaluation: return QStringLiteral("TessellationEvaluation");
        case SpvExecutionModelGeometry:               return QStringLiteral("Geometry");
        case SpvExecutionModelFragment:               return QStringLiteral("Fragment");
        case SpvExecutionModelGLCompute:              return QStringLiteral("GLCompute");
        case SpvExecutionModelKernel:                 return QStringLiteral("Kernel");
        case SpvExecutionModelTaskNV:                 return QStringLiteral("TaskNV");
        case SpvExecutionModelMeshNV:                 return QStringLiteral("MeshNV");
        case SpvExecutionModelRayGenerationKHR:       return QStringLiteral("RayGenerationKHR");
        case SpvExecutionModelIntersectionKHR:        return QStringLiteral("IntersectionKHR");
        case SpvExecutionModelAnyHitKHR:              return QStringLiteral("AnyHitKHR");
        case SpvExecutionModelClosestHitKHR:          return QStringLiteral("ClosestHitKHR");
        case SpvExecutionModelMissKHR:                return QStringLiteral("MissKHR");
        case SpvExecutionModelCallableKHR:            return QStringLiteral("CallableKHR");
        case SpvExecutionModelTaskEXT:                return QStringLiteral("TaskEXT");
        case SpvExecutionModelMeshEXT:                return QStringLiteral("MeshEXT");
        default: break;
    }
    return trs(QT_TRANSLATE_NOOP("Reflection", "Model %1")).arg(model);
}

QString SpirvReflection::generatorName(int generator) {
    switch (static_cast<SpvReflectGenerator>(generator)) {
        case SPV_REFLECT_GENERATOR_KHRONOS_LLVM_SPIRV_TRANSLATOR:
            return QStringLiteral("Khronos LLVM/SPIR-V Translator");
        case SPV_REFLECT_GENERATOR_KHRONOS_SPIRV_TOOLS_ASSEMBLER:
            return QStringLiteral("Khronos SPIR-V Tools Assembler");
        case SPV_REFLECT_GENERATOR_KHRONOS_GLSLANG_REFERENCE_FRONT_END:
            return QStringLiteral("Khronos Glslang Reference Front End");
        case SPV_REFLECT_GENERATOR_GOOGLE_SHADERC_OVER_GLSLANG:
            return QStringLiteral("Google Shaderc over Glslang");
        case SPV_REFLECT_GENERATOR_GOOGLE_SPIREGG:
            return QStringLiteral("Google spiregg");
        case SPV_REFLECT_GENERATOR_GOOGLE_RSPIRV:
            return QStringLiteral("Google rspirv");
        case SPV_REFLECT_GENERATOR_X_LEGEND_MESA_MESAIR_SPIRV_TRANSLATOR:
            return QStringLiteral("Mesa MesaIR/SPIR-V Translator");
        case SPV_REFLECT_GENERATOR_KHRONOS_SPIRV_TOOLS_LINKER:
            return QStringLiteral("Khronos SPIR-V Tools Linker");
        case SPV_REFLECT_GENERATOR_WINE_VKD3D_SHADER_COMPILER:
            return QStringLiteral("Wine VKD3D Shader Compiler");
        case SPV_REFLECT_GENERATOR_CLAY_CLAY_SHADER_COMPILER:
            return QStringLiteral("Clay Clay Shader Compiler");
        default: break;
    }
    return trs(QT_TRANSLATE_NOOP("Reflection", "Generator %1")).arg(generator);
}

QString SpirvReflection::builtinName(int builtin) {
    switch (static_cast<SpvBuiltIn>(builtin)) {
        case SpvBuiltInPosition:                  return QStringLiteral("Position");
        case SpvBuiltInPointSize:                 return QStringLiteral("PointSize");
        case SpvBuiltInClipDistance:              return QStringLiteral("ClipDistance");
        case SpvBuiltInCullDistance:              return QStringLiteral("CullDistance");
        case SpvBuiltInVertexId:                  return QStringLiteral("VertexId");
        case SpvBuiltInInstanceId:                return QStringLiteral("InstanceId");
        case SpvBuiltInPrimitiveId:               return QStringLiteral("PrimitiveId");
        case SpvBuiltInInvocationId:              return QStringLiteral("InvocationId");
        case SpvBuiltInLayer:                     return QStringLiteral("Layer");
        case SpvBuiltInViewportIndex:             return QStringLiteral("ViewportIndex");
        case SpvBuiltInTessLevelOuter:            return QStringLiteral("TessLevelOuter");
        case SpvBuiltInTessLevelInner:            return QStringLiteral("TessLevelInner");
        case SpvBuiltInTessCoord:                 return QStringLiteral("TessCoord");
        case SpvBuiltInPatchVertices:             return QStringLiteral("PatchVertices");
        case SpvBuiltInFragCoord:                 return QStringLiteral("FragCoord");
        case SpvBuiltInPointCoord:                return QStringLiteral("PointCoord");
        case SpvBuiltInFrontFacing:               return QStringLiteral("FrontFacing");
        case SpvBuiltInSampleId:                  return QStringLiteral("SampleId");
        case SpvBuiltInSamplePosition:            return QStringLiteral("SamplePosition");
        case SpvBuiltInSampleMask:                return QStringLiteral("SampleMask");
        case SpvBuiltInFragDepth:                 return QStringLiteral("FragDepth");
        case SpvBuiltInHelperInvocation:          return QStringLiteral("HelperInvocation");
        case SpvBuiltInNumWorkgroups:             return QStringLiteral("NumWorkgroups");
        case SpvBuiltInWorkgroupSize:             return QStringLiteral("WorkgroupSize");
        case SpvBuiltInWorkgroupId:               return QStringLiteral("WorkgroupId");
        case SpvBuiltInLocalInvocationId:         return QStringLiteral("LocalInvocationId");
        case SpvBuiltInGlobalInvocationId:        return QStringLiteral("GlobalInvocationId");
        case SpvBuiltInLocalInvocationIndex:      return QStringLiteral("LocalInvocationIndex");
        case SpvBuiltInVertexIndex:               return QStringLiteral("VertexIndex");
        case SpvBuiltInInstanceIndex:             return QStringLiteral("InstanceIndex");
        case SpvBuiltInSubgroupSize:              return QStringLiteral("SubgroupSize");
        case SpvBuiltInSubgroupLocalInvocationId: return QStringLiteral("SubgroupLocalInvocationId");
        case SpvBuiltInSubgroupEqMask:            return QStringLiteral("SubgroupEqMask");
        case SpvBuiltInSubgroupGeMask:            return QStringLiteral("SubgroupGeMask");
        case SpvBuiltInSubgroupGtMask:            return QStringLiteral("SubgroupGtMask");
        case SpvBuiltInSubgroupLeMask:            return QStringLiteral("SubgroupLeMask");
        case SpvBuiltInSubgroupLtMask:            return QStringLiteral("SubgroupLtMask");
        case SpvBuiltInBaseVertex:                return QStringLiteral("BaseVertex");
        case SpvBuiltInBaseInstance:              return QStringLiteral("BaseInstance");
        case SpvBuiltInDrawIndex:                 return QStringLiteral("DrawIndex");
        case SpvBuiltInDeviceIndex:               return QStringLiteral("DeviceIndex");
        case SpvBuiltInViewIndex:                 return QStringLiteral("ViewIndex");
        default: break;
    }
    return QStringLiteral("builtin %1").arg(builtin);
}

QString SpirvReflection::resultName(int result) {
    switch (static_cast<SpvReflectResult>(result)) {
        case SPV_REFLECT_RESULT_SUCCESS:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Success"));
        case SPV_REFLECT_RESULT_NOT_READY:
            return trs(QT_TRANSLATE_NOOP("Reflection", "The module is not ready yet"));
        case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED:
            return trs(QT_TRANSLATE_NOOP("Reflection", "SPIR-V parsing failed (the module may be corrupt)"));
        case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Memory allocation failed"));
        case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Index out of range"));
        case SPV_REFLECT_RESULT_ERROR_NULL_POINTER:
            return trs(QT_TRANSLATE_NOOP("Reflection", "A null pointer was passed in"));
        case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR:
            return trs(QT_TRANSLATE_NOOP("Reflection", "SPIRV-Reflect internal error"));
        case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Enumeration count mismatch (the module changed between calls)"));
        case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND:
            return trs(QT_TRANSLATE_NOOP("Reflection", "The requested element was not found"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE:
            return trs(QT_TRANSLATE_NOOP("Reflection", "The SPIR-V byte count is not a multiple of 4"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Invalid SPIR-V magic number"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF:
            return trs(QT_TRANSLATE_NOOP("Reflection", "The SPIR-V data ends unexpectedly"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE:
            return trs(QT_TRANSLATE_NOOP("Reflection", "The SPIR-V references an invalid id"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Descriptor set number overflow"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Invalid SPIR-V storage class"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Recursion while parsing the SPIR-V"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Invalid SPIR-V instruction"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Unexpected SPIR-V block data"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Invalid SPIR-V block member reference"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Invalid SPIR-V entry point"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE:
            return trs(QT_TRANSLATE_NOOP("Reflection", "Invalid SPIR-V execution mode"));
        case SPV_REFLECT_RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED:
            return trs(QT_TRANSLATE_NOOP("Reflection", "SPIR-V recursion depth exceeded"));
    }
    return trs(QT_TRANSLATE_NOOP("Reflection", "Unknown error (%1)")).arg(result);
}

QString SpirvReflection::typeNameFromDescription(const SpvReflectTypeDescription& type) {
    return memberTypeName(&type, type.traits.numeric, type.traits.array);
}

// --- reflection --------------------------------------------------------------

ReflectionResult SpirvReflection::reflect(const QVector<quint32>& words) {
    ReflectionResult result;

    if (words.size() < 5) {
        result.diagnostics.append(Diagnostic{
            Severity::Error, DiagSource::SpirvValidation, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Reflection", "The SPIR-V data is shorter than 5 words, so it cannot be reflected."))});
        return result;
    }
    if (words.first() != 0x07230203u) {
        result.diagnostics.append(Diagnostic{
            Severity::Error, DiagSource::SpirvValidation, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Reflection", "The SPIR-V magic number is not 0x07230203, so it cannot be reflected."))});
        return result;
    }

    ModuleOwner owner;
    const SpvReflectResult createResult = owner.create(words);
    if (createResult != SPV_REFLECT_RESULT_SUCCESS) {
        result.diagnostics.append(Diagnostic{
            Severity::Error, DiagSource::SpirvValidation, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Reflection", "Reflection failed: %1. If the shader was just edited, recompile it before "
                "looking at the reflection information again."))
                .arg(resultName(static_cast<int>(createResult)))});
        return result;
    }
    SpvReflectShaderModule* module = owner.module();

    // --- execution info ------------------------------------------------------
    ReflectData& data = result.data;
    data.execution.entryPoint = fromConstChar(module->entry_point_name);
    if (data.execution.entryPoint.isEmpty()) {
        data.execution.entryPoint = QStringLiteral("main");
    }
    data.execution.executionModel =
        executionModelName(static_cast<int>(module->spirv_execution_model));
    data.execution.generator = generatorName(static_cast<int>(module->generator));
    if (module->entry_point_count > 0 && module->entry_points != nullptr) {
        const SpvReflectEntryPoint& entry = module->entry_points[0];
        data.execution.localSizeX = entry.local_size.x;
        data.execution.localSizeY = entry.local_size.y;
        data.execution.localSizeZ = entry.local_size.z;
        data.execution.invocations = entry.invocations;
        data.execution.outputVertices = entry.output_vertices;
        for (uint32_t i = 0; i < entry.execution_mode_count; ++i) {
            const int mode = static_cast<int>(entry.execution_modes[i]);
            // Execution modes are only meaningful as names; spvExecutionModeString
            // is not part of SPIRV-Reflect, so report the numeric value with the
            // few common ones spelled out.
            QString modeName;
            switch (entry.execution_modes[i]) {
                case SpvExecutionModeInvocations:   modeName = QStringLiteral("Invocations"); break;
                case SpvExecutionModeSpacingEqual:  modeName = QStringLiteral("SpacingEqual"); break;
                case SpvExecutionModeOutputVertices: modeName = QStringLiteral("OutputVertices"); break;
                case SpvExecutionModeLocalSize:     modeName = QStringLiteral("LocalSize"); break;
                case SpvExecutionModeLocalSizeId:   modeName = QStringLiteral("LocalSizeId"); break;
                case SpvExecutionModeOriginUpperLeft: modeName = QStringLiteral("OriginUpperLeft"); break;
                case SpvExecutionModeEarlyFragmentTests: modeName = QStringLiteral("EarlyFragmentTests"); break;
                case SpvExecutionModeDepthReplacing: modeName = QStringLiteral("DepthReplacing"); break;
                default: modeName = trs(QT_TRANSLATE_NOOP("Reflection", "Mode %1")).arg(mode); break;
            }
            data.execution.executionModes << modeName;
        }
    }
    if (module->_internal != nullptr) {
        data.execution.spirvVersion = module->_internal->spirv_word_count > 1
                                          ? words.at(1)
                                          : 0;
    }

    // --- descriptor bindings -------------------------------------------------
    {
        uint32_t count = 0;
        SpvReflectResult status = spvReflectEnumerateDescriptorBindings(module, &count, nullptr);
        if (status != SPV_REFLECT_RESULT_SUCCESS) {
            result.diagnostics.append(Diagnostic{
                Severity::Warning, DiagSource::SpirvValidation, 0, 0, {},
                trs(QT_TRANSLATE_NOOP("Reflection", "Failed to enumerate the descriptor binding count: %1"))
                    .arg(resultName(static_cast<int>(status)))});
        } else if (count > 0) {
            QVector<SpvReflectDescriptorBinding*> bindings(static_cast<int>(count), nullptr);
            status = spvReflectEnumerateDescriptorBindings(module, &count, bindings.data());
            if (status != SPV_REFLECT_RESULT_SUCCESS) {
                result.diagnostics.append(Diagnostic{
                    Severity::Warning, DiagSource::SpirvValidation, 0, 0, {},
                    trs(QT_TRANSLATE_NOOP("Reflection", "Failed to read the descriptor bindings: %1"))
                        .arg(resultName(static_cast<int>(status)))});
            } else {
                data.descriptors.reserve(static_cast<int>(count));
                for (uint32_t i = 0; i < count; ++i) {
                    const SpvReflectDescriptorBinding* binding = bindings.at(static_cast<int>(i));
                    if (binding == nullptr) {
                        continue;
                    }
                    DescriptorBinding out;
                    out.set = binding->set;
                    out.binding = binding->binding;
                    out.typeName = descriptorTypeName(static_cast<int>(binding->descriptor_type));
                    out.name = fromConstChar(binding->name);
                    out.count = binding->count;
                    out.size = binding->block.size;
                    out.used = (binding->accessed != 0) &&
                               ((binding->block.flags & SPV_REFLECT_VARIABLE_FLAGS_UNUSED) == 0);

                    // Uniform/storage buffers expose their member layout.
                    if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                        binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
                        binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                        binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
                        out.members = blockToMember(binding->block).members;
                    }
                    data.descriptors.append(out);
                }
            }
        }
    }

    // --- push constants ------------------------------------------------------
    {
        uint32_t count = 0;
        SpvReflectResult status = spvReflectEnumeratePushConstantBlocks(module, &count, nullptr);
        if (status != SPV_REFLECT_RESULT_SUCCESS) {
            result.diagnostics.append(Diagnostic{
                Severity::Warning, DiagSource::SpirvValidation, 0, 0, {},
                trs(QT_TRANSLATE_NOOP("Reflection", "Failed to enumerate the push constant blocks: %1"))
                    .arg(resultName(static_cast<int>(status)))});
        } else if (count > 0) {
            QVector<SpvReflectBlockVariable*> blocks(static_cast<int>(count), nullptr);
            status = spvReflectEnumeratePushConstantBlocks(module, &count, blocks.data());
            if (status != SPV_REFLECT_RESULT_SUCCESS) {
                result.diagnostics.append(Diagnostic{
                    Severity::Warning, DiagSource::SpirvValidation, 0, 0, {},
                    trs(QT_TRANSLATE_NOOP("Reflection", "Failed to read the push constant blocks: %1"))
                        .arg(resultName(static_cast<int>(status)))});
            } else {
                data.pushConstants.reserve(static_cast<int>(count));
                for (uint32_t i = 0; i < count; ++i) {
                    const SpvReflectBlockVariable* block = blocks.at(static_cast<int>(i));
                    if (block == nullptr) {
                        continue;
                    }
                    PushConstantBlock out;
                    out.name = fromConstChar(block->name);
                    if (out.name.isEmpty()) {
                        out.name = QStringLiteral("push_constant");
                    }
                    out.offset = block->offset;
                    out.size = block->size;
                    out.members = blockToMember(*block).members;
                    data.pushConstants.append(out);
                }
            }
        }
    }

    // --- stage inputs / outputs ---------------------------------------------
    const auto collectInterface = [module](bool inputs, QVector<StageVar>* out) {
        uint32_t count = 0;
        SpvReflectResult status = inputs
                                      ? spvReflectEnumerateInputVariables(module, &count, nullptr)
                                      : spvReflectEnumerateOutputVariables(module, &count, nullptr);
        if (status != SPV_REFLECT_RESULT_SUCCESS || count == 0) {
            return status;
        }
        QVector<SpvReflectInterfaceVariable*> variables(static_cast<int>(count), nullptr);
        status = inputs
                     ? spvReflectEnumerateInputVariables(module, &count, variables.data())
                     : spvReflectEnumerateOutputVariables(module, &count, variables.data());
        if (status != SPV_REFLECT_RESULT_SUCCESS) {
            return status;
        }
        out->reserve(static_cast<int>(count));
        for (uint32_t i = 0; i < count; ++i) {
            const SpvReflectInterfaceVariable* variable = variables.at(static_cast<int>(i));
            if (variable == nullptr) {
                continue;
            }
            StageVar var;
            var.location = variable->location;
            var.component = variable->component;
            var.name = fromConstChar(variable->name);
            if (var.name.isEmpty() && variable->built_in >= 0) {
                var.name = builtinName(variable->built_in);
            }
            var.format = formatName(static_cast<int>(variable->format));
            var.builtin = variable->built_in;
            if (variable->built_in >= 0) {
                var.builtinName = builtinName(variable->built_in);
            }
            if (variable->type_description != nullptr) {
                var.typeName = typeNameFromDescription(*variable->type_description);
            } else {
                var.typeName = memberTypeName(nullptr, variable->numeric, variable->array);
            }

            // Interface blocks (notably gl_PerVertex, which carries gl_Position)
            // expose their contents as members rather than as separate variables,
            // so expand them one level.
            if (variable->member_count > 0 && variable->members != nullptr) {
                var.members.reserve(static_cast<int>(variable->member_count));
                for (uint32_t m = 0; m < variable->member_count; ++m) {
                    const SpvReflectInterfaceVariable& member = variable->members[m];
                    StageVar child;
                    child.location = member.location;
                    child.component = member.component;
                    child.name = fromConstChar(member.name);
                    child.format = formatName(static_cast<int>(member.format));
                    child.builtin = member.built_in;
                    if (member.built_in >= 0) {
                        child.builtinName = builtinName(member.built_in);
                        if (child.name.isEmpty()) {
                            child.name = child.builtinName;
                        }
                    }
                    if (member.type_description != nullptr) {
                        child.typeName = typeNameFromDescription(*member.type_description);
                    } else {
                        child.typeName = memberTypeName(nullptr, member.numeric, member.array);
                    }
                    var.members.append(child);
                }
            }

            out->append(var);
        }
        return status;
    };

    if (const SpvReflectResult status = collectInterface(true, &data.inputs);
        status != SPV_REFLECT_RESULT_SUCCESS && status != SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND) {
        result.diagnostics.append(Diagnostic{
            Severity::Warning, DiagSource::SpirvValidation, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Reflection", "Failed to enumerate the input variables: %1"))
                .arg(resultName(static_cast<int>(status)))});
    }
    if (const SpvReflectResult status = collectInterface(false, &data.outputs);
        status != SPV_REFLECT_RESULT_SUCCESS && status != SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND) {
        result.diagnostics.append(Diagnostic{
            Severity::Warning, DiagSource::SpirvValidation, 0, 0, {},
            trs(QT_TRANSLATE_NOOP("Reflection", "Failed to enumerate the output variables: %1"))
                .arg(resultName(static_cast<int>(status)))});
    }

    // --- specialization constants -------------------------------------------
    {
        uint32_t count = 0;
        SpvReflectResult status = spvReflectEnumerateSpecializationConstants(module, &count, nullptr);
        if (status == SPV_REFLECT_RESULT_SUCCESS && count > 0) {
            QVector<SpvReflectSpecializationConstant*> constants(static_cast<int>(count), nullptr);
            status = spvReflectEnumerateSpecializationConstants(module, &count, constants.data());
            if (status == SPV_REFLECT_RESULT_SUCCESS) {
                data.specConstants.reserve(static_cast<int>(count));
                for (uint32_t i = 0; i < count; ++i) {
                    const SpvReflectSpecializationConstant* constant = constants.at(static_cast<int>(i));
                    if (constant == nullptr) {
                        continue;
                    }
                    SpecConstant out;
                    out.constantId = constant->constant_id;
                    out.name = fromConstChar(constant->name);
                    if (out.name.isEmpty()) {
                        out.name = QStringLiteral("spec_constant_%1").arg(constant->constant_id);
                    }
                    // Default values require evaluating the constant instruction;
                    // SPIRV-Reflect does not expose it, so report "no default".
                    out.hasDefault = false;
                    out.defaultValue = trs(QT_TRANSLATE_NOOP("Reflection", "(not provided)"));
                    data.specConstants.append(out);
                }
            }
        }
    }

    result.ok = true;
    return result;
}

}  // namespace spall
