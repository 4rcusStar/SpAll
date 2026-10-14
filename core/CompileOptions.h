// CompileOptions.h — every knob the compiler/disassembler/optimizer honors.
//
// UI-facing enums are declared here as plain integers so that no consumer has to
// include SPIRV-Tools or glslang headers; the mapping onto the real
// spv_target_env lives in SpirvDisassembler.cpp.
#pragma once

#include <QStringList>

#include "ShaderStage.h"

namespace spall {

/// Target SPIR-V environment, mirroring spv_target_env values we support.
enum class TargetEnv {
    Vulkan10 = 0,
    Vulkan11,
    Vulkan12,
    Vulkan13,
    Universal10,
    Universal13,
    Universal16,
};

/// Optimization preset.
enum class OptimizationLevel {
    None = 0,
    Performance,  ///< -O
    Size,         ///< -Os
};

/// Vulkan client version, mirroring EShTargetClientVersion.
enum class VulkanVersion {
    Vulkan10 = 0,
    Vulkan11,
    Vulkan12,
    Vulkan13,
};

/// SPIR-V binary version, expressed in the usual 10000/10300/10600 form.
enum class SpirvVersion {
    Spv10 = 10000,
    Spv11 = 10100,
    Spv12 = 10200,
    Spv13 = 10300,
    Spv14 = 10400,
    Spv15 = 10500,
    Spv16 = 10600,
};

/// Bit flags for spvBinaryToText, kept in our own type so the header does not
/// depend on spirv-tools. SpirvDisassembler translates these to the real
/// SPV_BINARY_TO_TEXT_OPTION_* bits.
enum DisasmFlag : unsigned {
    DisasmNone            = 0u,
    DisasmIndent          = 1u << 0,
    DisasmFriendlyNames   = 1u << 1,
    DisasmComment         = 1u << 2,
    DisasmNoHeader        = 1u << 3,
    DisasmShowByteOffset  = 1u << 4,
    DisasmNestedIndent    = 1u << 5,
    DisasmReorderBlocks   = 1u << 6,
};

/// Friendly names for the UI.
QString targetEnvName(TargetEnv env);
QString optimizationLevelName(OptimizationLevel level);
QString vulkanVersionName(VulkanVersion version);
QString spirvVersionName(SpirvVersion version);

/// Default SPIR-V version implied by a Vulkan version: 1.3 targets SPIR-V 1.6,
/// everything older sticks to 1.3 for widest tool compatibility.
SpirvVersion defaultSpirvVersionFor(VulkanVersion vulkan);

/// Target environment implied by a Vulkan version.
TargetEnv defaultTargetEnvFor(VulkanVersion vulkan);

/// Resource limits handed to glslang. glslang's own defaults are small enough to
/// produce surprising errors on ordinary shaders, so we raise them explicitly.
///
/// Every field here has a matching member in glslang's TBuiltInResource; the
/// struct deliberately does not model fields (e.g. array-valued work-group
/// limits) that differ between glslang releases.
struct GlslangResourceLimits {
    int maxLights = 32;
    int maxClipPlanes = 6;
    int maxTextureUnits = 32;
    int maxTextureCoords = 32;
    int maxVertexAttribs = 64;
    int maxVertexUniformComponents = 4096;
    int maxFragmentUniformComponents = 4096;
    int maxVaryingFloats = 64;
    int maxVertexTextureImageUnits = 32;
    int maxCombinedTextureImageUnits = 32;
    int maxTextureImageUnits = 32;
    int maxGeometryTextureImageUnits = 16;
    int maxGeometryOutputVertices = 256;
    // Compute work-group limits.
    int maxComputeWorkGroupCountX = 65535;
    int maxComputeWorkGroupCountY = 65535;
    int maxComputeWorkGroupCountZ = 65535;
    int maxComputeWorkGroupSizeX = 1024;
    int maxComputeWorkGroupSizeY = 1024;
    int maxComputeWorkGroupSizeZ = 64;
    int maxComputeUniformComponents = 1024;
    int maxComputeAtomicCounters = 8;
    int maxComputeAtomicCounterBuffers = 1;
    int maxCombinedImageUnitsAndFragmentOutputs = 8;
    int maxImageUnits = 8;
    int maxVertexOutputComponents = 128;
    int maxGeometryInputComponents = 128;
    int maxGeometryOutputComponents = 128;
    int maxFragmentInputComponents = 128;
    int maxAtomicCounterBufferSize = 16384;
    int maxAtomicCounterBindings = 8;
    int maxTransformFeedbackBuffers = 4;
    int maxViewports = 16;
    int maxSamples = 4;
    int maxPatchVertices = 32;
};

/// Everything a single compilation needs.
struct CompileOptions {
    ShaderStage stage = ShaderStage::Vertex;

    /// GLSL version for the #version directive, e.g. 450.
    int glslVersion = 450;
    bool esProfile = false;

    VulkanVersion vulkanVersion = VulkanVersion::Vulkan13;
    SpirvVersion spirvVersion = SpirvVersion::Spv16;

    /// Fallback binding/set for uniform blocks and samplers that omit
    /// layout(binding=)/layout(set=). Needed for the result to validate.
    int defaultUniformBinding = 0;
    int defaultUniformSet = 0;

    /// Auto-assign locations/bindings so a shader without explicit layout
    /// qualifiers still produces validatable SPIR-V.
    bool autoMapLocations = true;
    bool autoMapBindings = true;

    /// Expand #include before handing the source to glslang.
    bool enableIncludes = true;
    QStringList includePaths;

    /// SPIRV-Tools optimization preset.
    OptimizationLevel optimizationLevel = OptimizationLevel::None;

    TargetEnv targetEnv = TargetEnv::Vulkan13;

    /// Run spvValidate on the produced module.
    bool validate = true;

    /// spvBinaryToText option bits (DisasmFlag values OR-ed together).
    unsigned disasmFlags = DisasmIndent | DisasmFriendlyNames | DisasmComment;

    /// Inject `#define SPALL 1` style preamble macros.
    bool enablePreamble = true;

    GlslangResourceLimits limits;

    /// Compile-time sanity: keeps the UI from producing combinations glslang
    /// cannot express.
    bool isSane() const { return glslVersion >= 100 && glslVersion <= 460; }
};

}  // namespace spall
