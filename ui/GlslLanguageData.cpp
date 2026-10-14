// GlslLanguageData.cpp — the static GLSL 4.50 tables.
#include "GlslLanguageData.h"

#include <QCoreApplication>
#include <QHash>

namespace spall {

namespace {

// Localised string for this translation context.
//
// A macro rather than a function on purpose: lupdate parses sources statically
// and cannot see through a call that passes the text on to
// QCoreApplication::translate(). Spelling the lookup out here means every trs(QT_TRANSLATE_NOOP("GlslLanguageData", ...))
// call site is extracted automatically, while the runtime behaviour is unchanged.
/// Localised string for this translation context.
///
/// The text is passed through QT_TRANSLATE_NOOP at every call site, which is what
/// lets lupdate extract it statically; this function then performs the runtime
/// lookup. core/ has no QObject, so tr() is not available there.
QString trs(const char* text) {
    return QCoreApplication::translate("GlslLanguageData", text);
}

/// Build a CompletionItem for a builtin function.
///
/// The description is user-visible documentation and goes through the
/// translator; name, return type and signature are GLSL identifiers and type
/// names, so they are copied verbatim.
CompletionItem fn(const char* name, const char* ret, const char* signature, const char* description) {
    CompletionItem item;
    item.name = QString::fromLatin1(name);
    item.returnType = QString::fromLatin1(ret);
    item.signature = QString::fromLatin1(signature);
    item.description = trs(QT_TRANSLATE_NOOP("GlslLanguageData", description));
    return item;
}

/// Build the builtin function table once.
QVector<CompletionItem> makeBuiltinFunctions() {
    QVector<CompletionItem> out;

    // --- angle & trigonometry ----------------------------------------------
    out << fn("radians", "float", "(genType degrees)", QT_TRANSLATE_NOOP("GlslLanguageData", "Converts degrees to radians"));
    out << fn("degrees", "float", "(genType radians)", QT_TRANSLATE_NOOP("GlslLanguageData", "Converts radians to degrees"));
    out << fn("sin", "float", "(genType angle)", QT_TRANSLATE_NOOP("GlslLanguageData", "Sine"));
    out << fn("cos", "float", "(genType angle)", QT_TRANSLATE_NOOP("GlslLanguageData", "Cosine"));
    out << fn("tan", "float", "(genType angle)", QT_TRANSLATE_NOOP("GlslLanguageData", "Tangent"));
    out << fn("asin", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Arc sine; x must be in [-1, 1]"));
    out << fn("acos", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Arc cosine; x must be in [-1, 1]"));
    out << fn("atan", "float", "(genType y, genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Arc tangent (the two-argument form returns the correct quadrant)"));
    out << fn("sinh", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Hyperbolic sine"));
    out << fn("cosh", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Hyperbolic cosine"));
    out << fn("tanh", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Hyperbolic tangent"));
    out << fn("asinh", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Inverse hyperbolic sine"));
    out << fn("acosh", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Inverse hyperbolic cosine; x >= 1"));
    out << fn("atanh", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Inverse hyperbolic tangent; |x| < 1"));

    // --- exponential --------------------------------------------------------
    out << fn("pow", "float", "(genType x, genType y)", QT_TRANSLATE_NOOP("GlslLanguageData", "x raised to the power y"));
    out << fn("exp", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Natural exponential, e^x"));
    out << fn("log", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Natural logarithm"));
    out << fn("exp2", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "2 raised to the power x"));
    out << fn("log2", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Base-2 logarithm"));
    out << fn("sqrt", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Square root"));
    out << fn("inversesqrt", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Reciprocal square root, 1/sqrt(x)"));

    // --- common -------------------------------------------------------------
    out << fn("abs", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Absolute value"));
    out << fn("sign", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Sign: -1, 0 or 1"));
    out << fn("floor", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Rounds down to the nearest integer"));
    out << fn("trunc", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Truncates towards zero"));
    out << fn("round", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Rounds to the nearest integer (tie direction is implementation-defined)"));
    out << fn("roundEven", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Rounds to the nearest integer, ties to even"));
    out << fn("ceil", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Rounds up to the nearest integer"));
    out << fn("fract", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Fractional part, x - floor(x)"));
    out << fn("mod", "float", "(genType x, genType y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Modulo, x - y*floor(x/y)"));
    out << fn("mod", "float", "(genType x, float y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Modulo with a scalar divisor"));
    out << fn("modf", "float", "(genType x, out genType i)", QT_TRANSLATE_NOOP("GlslLanguageData", "Splits x into integral and fractional parts"));
    out << fn("min", "float", "(genType x, genType y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Smaller of the two values"));
    out << fn("max", "float", "(genType x, genType y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Larger of the two values"));
    out << fn("clamp", "float", "(genType x, genType minVal, genType maxVal)", QT_TRANSLATE_NOOP("GlslLanguageData", "Clamps x to the range [minVal, maxVal]"));
    out << fn("mix", "float", "(genType x, genType y, genType a)", QT_TRANSLATE_NOOP("GlslLanguageData", "Linear interpolation"));
    out << fn("mix", "float", "(genType x, genType y, float a)", QT_TRANSLATE_NOOP("GlslLanguageData", "Linear interpolation with a scalar weight"));
    out << fn("step", "float", "(genType edge, genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Returns 0 when x < edge, otherwise 1"));
    out << fn("smoothstep", "float", "(genType e0, genType e1, genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Smooth Hermite interpolation"));
    out << fn("isnan", "bvec", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "True where x is NaN"));
    out << fn("isinf", "bvec", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "True where x is infinite"));
    out << fn("floatBitsToInt", "int", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Reinterprets the bits as int"));
    out << fn("floatBitsToUint", "uint", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Reinterprets the bits as uint"));
    out << fn("intBitsToFloat", "float", "(genIType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Reinterprets the bits as float"));
    out << fn("uintBitsToFloat", "float", "(genUType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Reinterprets the bits as float"));
    out << fn("fma", "float", "(genType a, genType b, genType c)", QT_TRANSLATE_NOOP("GlslLanguageData", "Fused multiply-add a*b+c"));
    out << fn("frexp", "float", "(genType x, out genIType exp)", QT_TRANSLATE_NOOP("GlslLanguageData", "Splits x into mantissa and exponent"));
    out << fn("ldexp", "float", "(genType x, genIType exp)", QT_TRANSLATE_NOOP("GlslLanguageData", "x * 2^exp"));
    out << fn("packSnorm2x16", "uint", "(vec2 v)", QT_TRANSLATE_NOOP("GlslLanguageData", "Packs two normalized signed 16-bit values into a uint"));
    out << fn("unpackSnorm2x16", "vec2", "(uint p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Unpacks two normalized signed 16-bit values from a uint"));
    out << fn("packUnorm2x16", "uint", "(vec2 v)", QT_TRANSLATE_NOOP("GlslLanguageData", "Packs two normalized unsigned 16-bit values into a uint"));
    out << fn("unpackUnorm2x16", "vec2", "(uint p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Unpacks two normalized unsigned 16-bit values from a uint"));
    out << fn("packHalf2x16", "uint", "(vec2 v)", QT_TRANSLATE_NOOP("GlslLanguageData", "Packs two half-floats into a uint"));
    out << fn("unpackHalf2x16", "vec2", "(uint v)", QT_TRANSLATE_NOOP("GlslLanguageData", "Unpacks two half-floats from a uint"));
    out << fn("packUnorm4x8", "uint", "(vec4 v)", QT_TRANSLATE_NOOP("GlslLanguageData", "Packs four normalized unsigned bytes into a uint"));
    out << fn("unpackUnorm4x8", "vec4", "(uint v)", QT_TRANSLATE_NOOP("GlslLanguageData", "Unpacks four normalized unsigned bytes from a uint"));
    out << fn("packSnorm4x8", "uint", "(vec4 v)", QT_TRANSLATE_NOOP("GlslLanguageData", "Packs four normalized signed bytes into a uint"));
    out << fn("unpackSnorm4x8", "vec4", "(uint v)", QT_TRANSLATE_NOOP("GlslLanguageData", "Unpacks four normalized signed bytes from a uint"));

    // --- geometric ----------------------------------------------------------
    out << fn("length", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Length of a vector"));
    out << fn("distance", "float", "(genType p0, genType p1)", QT_TRANSLATE_NOOP("GlslLanguageData", "Distance between two points"));
    out << fn("dot", "float", "(genType x, genType y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Dot product"));
    out << fn("cross", "vec3", "(vec3 x, vec3 y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Cross product"));
    out << fn("normalize", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Normalizes a vector"));
    out << fn("faceforward", "float", "(genType N, genType I, genType Nref)", QT_TRANSLATE_NOOP("GlslLanguageData", "Flips the normal to face away from the reference direction"));
    out << fn("reflect", "float", "(genType I, genType N)", QT_TRANSLATE_NOOP("GlslLanguageData", "Reflection vector"));
    out << fn("refract", "float", "(genType I, genType N, float eta)", QT_TRANSLATE_NOOP("GlslLanguageData", "Refraction vector"));

    // --- matrix -------------------------------------------------------------
    out << fn("matrixCompMult", "mat", "(mat x, mat y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Component-wise multiply (not a matrix product)"));
    out << fn("outerProduct", "mat", "(vec c, vec r)", QT_TRANSLATE_NOOP("GlslLanguageData", "Outer product c * r^T"));
    out << fn("transpose", "mat", "(mat m)", QT_TRANSLATE_NOOP("GlslLanguageData", "Transpose"));
    out << fn("determinant", "float", "(mat m)", QT_TRANSLATE_NOOP("GlslLanguageData", "Determinant"));
    out << fn("inverse", "mat", "(mat m)", QT_TRANSLATE_NOOP("GlslLanguageData", "Inverse matrix"));

    // --- vector relational --------------------------------------------------
    out << fn("lessThan", "bvec", "(vec x, vec y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Component-wise <"));
    out << fn("lessThanEqual", "bvec", "(vec x, vec y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Component-wise <="));
    out << fn("greaterThan", "bvec", "(vec x, vec y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Component-wise >"));
    out << fn("greaterThanEqual", "bvec", "(vec x, vec y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Component-wise >="));
    out << fn("equal", "bvec", "(vec x, vec y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Component-wise =="));
    out << fn("notEqual", "bvec", "(vec x, vec y)", QT_TRANSLATE_NOOP("GlslLanguageData", "Component-wise !="));
    out << fn("any", "bool", "(bvec x)", QT_TRANSLATE_NOOP("GlslLanguageData", "True when any component is true"));
    out << fn("all", "bool", "(bvec x)", QT_TRANSLATE_NOOP("GlslLanguageData", "True when all components are true"));
    out << fn("not", "bvec", "(bvec x)", QT_TRANSLATE_NOOP("GlslLanguageData", "Component-wise logical not"));

    // --- integer ------------------------------------------------------------
    out << fn("uaddCarry", "uint", "(uint x, uint y, out uint carry)", QT_TRANSLATE_NOOP("GlslLanguageData", "Unsigned add, also returning the carry"));
    out << fn("usubBorrow", "uint", "(uint x, uint y, out uint borrow)", QT_TRANSLATE_NOOP("GlslLanguageData", "Unsigned subtract, also returning the borrow"));
    out << fn("umulExtended", "void", "(uint x, uint y, out uint msb, out uint lsb)", QT_TRANSLATE_NOOP("GlslLanguageData", "64-bit unsigned multiply"));
    out << fn("imulExtended", "void", "(int x, int y, out int msb, out int lsb)", QT_TRANSLATE_NOOP("GlslLanguageData", "64-bit signed multiply"));
    out << fn("bitfieldExtract", "int", "(genIType value, int offset, int bits)", QT_TRANSLATE_NOOP("GlslLanguageData", "Extracts a bit field"));
    out << fn("bitfieldInsert", "int", "(genIType base, genIType insert, int offset, int bits)", QT_TRANSLATE_NOOP("GlslLanguageData", "Inserts a bit field"));
    out << fn("bitfieldReverse", "int", "(genIType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Reverses the bit order"));
    out << fn("bitCount", "int", "(genIType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Number of set bits"));
    out << fn("findLSB", "int", "(genIType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Index of the least significant set bit"));
    out << fn("findMSB", "int", "(genIType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Index of the most significant set bit"));

    // --- texture ------------------------------------------------------------
    out << fn("texture", "vec4", "(sampler2D sampler, vec2 P)", QT_TRANSLATE_NOOP("GlslLanguageData", "Samples a texture (LOD chosen automatically)"));
    out << fn("texture", "vec4", "(sampler2D sampler, vec2 P, float bias)", QT_TRANSLATE_NOOP("GlslLanguageData", "Samples a texture with an LOD bias"));
    out << fn("textureProj", "vec4", "(sampler2D sampler, vec3 P)", QT_TRANSLATE_NOOP("GlslLanguageData", "Projective texture sampling"));
    out << fn("textureLod", "vec4", "(sampler2D sampler, vec2 P, float lod)", QT_TRANSLATE_NOOP("GlslLanguageData", "Samples at an explicit LOD"));
    out << fn("textureOffset", "vec4", "(sampler2D sampler, vec2 P, ivec2 offset)", QT_TRANSLATE_NOOP("GlslLanguageData", "Samples with a texel offset"));
    out << fn("texelFetch", "vec4", "(sampler2D sampler, ivec2 P, int lod)", QT_TRANSLATE_NOOP("GlslLanguageData", "Fetches a texel by integer coordinates"));
    out << fn("texelFetchOffset", "vec4", "(sampler2D sampler, ivec2 P, int lod, ivec2 offset)", QT_TRANSLATE_NOOP("GlslLanguageData", "Fetches a texel by integer coordinates plus an offset"));
    out << fn("textureProjOffset", "vec4", "(sampler2D sampler, vec3 P, ivec2 offset)", QT_TRANSLATE_NOOP("GlslLanguageData", "Projective sampling with an offset"));
    out << fn("textureLodOffset", "vec4", "(sampler2D sampler, vec2 P, float lod, ivec2 offset)", QT_TRANSLATE_NOOP("GlslLanguageData", "Samples at an explicit LOD with an offset"));
    out << fn("textureProjLod", "vec4", "(sampler2D sampler, vec3 P, float lod)", QT_TRANSLATE_NOOP("GlslLanguageData", "Projective sampling at an explicit LOD"));
    out << fn("textureProjLodOffset", "vec4", "(sampler2D sampler, vec3 P, float lod, ivec2 offset)", QT_TRANSLATE_NOOP("GlslLanguageData", "Projective sampling at an explicit LOD with an offset"));
    out << fn("textureGrad", "vec4", "(sampler2D sampler, vec2 P, vec2 dPdx, vec2 dPdy)", QT_TRANSLATE_NOOP("GlslLanguageData", "Samples with explicit gradients"));
    out << fn("textureGradOffset", "vec4", "(sampler2D sampler, vec2 P, vec2 dPdx, vec2 dPdy, ivec2 offset)", QT_TRANSLATE_NOOP("GlslLanguageData", "Samples with explicit gradients and an offset"));
    out << fn("textureProjGrad", "vec4", "(sampler2D sampler, vec3 P, vec2 dPdx, vec2 dPdy)", QT_TRANSLATE_NOOP("GlslLanguageData", "Projective sampling with explicit gradients"));
    out << fn("textureProjGradOffset", "vec4", "(sampler2D sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset)", QT_TRANSLATE_NOOP("GlslLanguageData", "Projective sampling with explicit gradients and an offset"));
    out << fn("textureGather", "vec4", "(sampler2D sampler, vec2 P)", QT_TRANSLATE_NOOP("GlslLanguageData", "Gathers four neighbouring texels"));
    out << fn("textureGatherOffset", "vec4", "(sampler2D sampler, vec2 P, ivec2 offset)", QT_TRANSLATE_NOOP("GlslLanguageData", "Gathers four neighbouring texels with an offset"));
    out << fn("textureGatherOffsets", "vec4", "(sampler2D sampler, vec2 P, ivec2 offsets[4])", QT_TRANSLATE_NOOP("GlslLanguageData", "Gathers texels using four offsets"));
    out << fn("textureSize", "ivec2", "(sampler2D sampler, int lod)", QT_TRANSLATE_NOOP("GlslLanguageData", "Queries the texture size"));
    out << fn("textureQueryLod", "vec2", "(sampler2D sampler, vec2 P)", QT_TRANSLATE_NOOP("GlslLanguageData", "Queries the LOD that would be used"));
    out << fn("textureQueryLevels", "int", "(sampler2D sampler)", QT_TRANSLATE_NOOP("GlslLanguageData", "Queries the number of mip levels"));
    out << fn("textureSamples", "int", "(sampler2DMS sampler)", QT_TRANSLATE_NOOP("GlslLanguageData", "Queries the number of samples"));

    // --- atomic -------------------------------------------------------------
    out << fn("atomicAdd", "uint", "(uint mem, uint data)", QT_TRANSLATE_NOOP("GlslLanguageData", "Atomic add"));
    out << fn("atomicMin", "uint", "(uint mem, uint data)", QT_TRANSLATE_NOOP("GlslLanguageData", "Atomic minimum"));
    out << fn("atomicMax", "uint", "(uint mem, uint data)", QT_TRANSLATE_NOOP("GlslLanguageData", "Atomic maximum"));
    out << fn("atomicAnd", "uint", "(uint mem, uint data)", QT_TRANSLATE_NOOP("GlslLanguageData", "Atomic bitwise AND"));
    out << fn("atomicOr", "uint", "(uint mem, uint data)", QT_TRANSLATE_NOOP("GlslLanguageData", "Atomic bitwise OR"));
    out << fn("atomicXor", "uint", "(uint mem, uint data)", QT_TRANSLATE_NOOP("GlslLanguageData", "Atomic bitwise XOR"));
    out << fn("atomicExchange", "uint", "(uint mem, uint data)", QT_TRANSLATE_NOOP("GlslLanguageData", "Atomic exchange"));
    out << fn("atomicCompSwap", "uint", "(uint mem, uint compare, uint data)", QT_TRANSLATE_NOOP("GlslLanguageData", "Atomic compare and swap"));

    // --- barriers -----------------------------------------------------------
    out << fn("barrier", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Synchronizes every invocation in the work group"));
    out << fn("memoryBarrier", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Memory barrier for all memory types"));
    out << fn("memoryBarrierShared", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Memory barrier for shared memory"));
    out << fn("memoryBarrierBuffer", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Memory barrier for buffer memory"));
    out << fn("memoryBarrierImage", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Memory barrier for image memory"));
    out << fn("memoryBarrierAtomicCounter", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Memory barrier for atomic counters"));
    out << fn("groupMemoryBarrier", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Work-group memory barrier"));

    // --- derivatives --------------------------------------------------------
    out << fn("dFdx", "float", "(genType p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Partial derivative in x (precision is implementation-defined)"));
    out << fn("dFdy", "float", "(genType p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Partial derivative in y (precision is implementation-defined)"));
    out << fn("dFdxFine", "float", "(genType p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Partial derivative in x (fine)"));
    out << fn("dFdyFine", "float", "(genType p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Partial derivative in y (fine)"));
    out << fn("dFdxCoarse", "float", "(genType p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Partial derivative in x (coarse)"));
    out << fn("dFdyCoarse", "float", "(genType p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Partial derivative in y (coarse)"));
    out << fn("fwidth", "float", "(genType p)", QT_TRANSLATE_NOOP("GlslLanguageData", "abs(dFdx) + abs(dFdy)"));
    out << fn("fwidthFine", "float", "(genType p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Fine-precision fwidth"));
    out << fn("fwidthCoarse", "float", "(genType p)", QT_TRANSLATE_NOOP("GlslLanguageData", "Coarse-precision fwidth"));

    // --- interpolation ------------------------------------------------------
    out << fn("interpolateAtCentroid", "float", "(genType interpolant)", QT_TRANSLATE_NOOP("GlslLanguageData", "Interpolates at the centroid"));
    out << fn("interpolateAtSample", "float", "(genType interpolant, int sample)", QT_TRANSLATE_NOOP("GlslLanguageData", "Interpolates at the given sample"));
    out << fn("interpolateAtOffset", "float", "(genType interpolant, vec2 offset)", QT_TRANSLATE_NOOP("GlslLanguageData", "Interpolates at the given offset"));

    // --- subgroup (GLSL 4.50 / KHR) ----------------------------------------
    out << fn("subgroupAdd", "float", "(genType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Sum across the subgroup"));
    out << fn("subgroupMul", "float", "(genType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Product across the subgroup"));
    out << fn("subgroupMin", "float", "(genType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Minimum across the subgroup"));
    out << fn("subgroupMax", "float", "(genType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Maximum across the subgroup"));
    out << fn("subgroupAnd", "float", "(genType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Bitwise AND across the subgroup"));
    out << fn("subgroupOr", "float", "(genType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Bitwise OR across the subgroup"));
    out << fn("subgroupXor", "float", "(genType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Bitwise XOR across the subgroup"));
    out << fn("subgroupAll", "bool", "(bool value)", QT_TRANSLATE_NOOP("GlslLanguageData", "True when the value is true in every lane of the subgroup"));
    out << fn("subgroupAny", "bool", "(bool value)", QT_TRANSLATE_NOOP("GlslLanguageData", "True when the value is true in any lane of the subgroup"));
    out << fn("subgroupBallot", "uvec4", "(bool value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Subgroup vote returning a bit mask"));
    out << fn("subgroupBroadcast", "float", "(genType value, uint id)", QT_TRANSLATE_NOOP("GlslLanguageData", "Broadcasts the value of the given lane"));
    out << fn("subgroupBroadcastFirst", "float", "(genType value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Broadcasts the value of the lowest lane"));
    out << fn("subgroupElect", "bool", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Elects a representative lane of the subgroup"));
    out << fn("subgroupBarrier", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Synchronizes within the subgroup"));
    out << fn("subgroupMemoryBarrier", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Subgroup memory barrier"));
    out << fn("subgroupMemoryBarrierBuffer", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Subgroup memory barrier for buffer memory"));
    out << fn("subgroupMemoryBarrierShared", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Subgroup memory barrier for shared memory"));
    out << fn("subgroupMemoryBarrierImage", "void", "()", QT_TRANSLATE_NOOP("GlslLanguageData", "Subgroup memory barrier for image memory"));
    out << fn("subgroupBallotBitCount", "uint", "(uvec4 value)", QT_TRANSLATE_NOOP("GlslLanguageData", "Counts the set bits in a bit mask"));

    // --- noise (legacy but still present) -----------------------------------
    out << fn("noise1", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "1D noise (implementation-defined)"));
    out << fn("noise2", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "2D noise (implementation-defined)"));
    out << fn("noise3", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "3D noise (implementation-defined)"));
    out << fn("noise4", "float", "(genType x)", QT_TRANSLATE_NOOP("GlslLanguageData", "4D noise (implementation-defined)"));

    return out;
}

/// Builtin variable names, shared with the highlighter.
QStringList makeBuiltinVariables() {
    return QStringList{
        QStringLiteral("gl_Position"),       QStringLiteral("gl_PointSize"),
        QStringLiteral("gl_ClipDistance"),   QStringLiteral("gl_CullDistance"),
        QStringLiteral("gl_VertexIndex"),    QStringLiteral("gl_InstanceIndex"),
        QStringLiteral("gl_VertexID"),       QStringLiteral("gl_InstanceID"),
        QStringLiteral("gl_BaseVertex"),     QStringLiteral("gl_BaseInstance"),
        QStringLiteral("gl_DrawID"),         QStringLiteral("gl_PrimitiveID"),
        QStringLiteral("gl_PrimitiveIDIn"),  QStringLiteral("gl_InvocationID"),
        QStringLiteral("gl_Layer"),          QStringLiteral("gl_ViewportIndex"),
        QStringLiteral("gl_TessCoord"),      QStringLiteral("gl_TessLevelOuter"),
        QStringLiteral("gl_TessLevelInner"), QStringLiteral("gl_PatchVerticesIn"),
        QStringLiteral("gl_FragCoord"),      QStringLiteral("gl_FrontFacing"),
        QStringLiteral("gl_PointCoord"),     QStringLiteral("gl_SampleID"),
        QStringLiteral("gl_SamplePosition"), QStringLiteral("gl_SampleMask"),
        QStringLiteral("gl_SampleMaskIn"),   QStringLiteral("gl_FragDepth"),
        QStringLiteral("gl_FragColor"),      QStringLiteral("gl_FragData"),
        QStringLiteral("gl_HelperInvocation"),
        QStringLiteral("gl_NumWorkGroups"),  QStringLiteral("gl_WorkGroupSize"),
        QStringLiteral("gl_WorkGroupID"),    QStringLiteral("gl_LocalInvocationID"),
        QStringLiteral("gl_GlobalInvocationID"),
        QStringLiteral("gl_LocalInvocationIndex"),
        QStringLiteral("gl_SubgroupSize"),   QStringLiteral("gl_SubgroupInvocationID"),
        QStringLiteral("gl_SubgroupEqMask"), QStringLiteral("gl_SubgroupGeMask"),
        QStringLiteral("gl_SubgroupGtMask"), QStringLiteral("gl_SubgroupLeMask"),
        QStringLiteral("gl_SubgroupLtMask"),
        QStringLiteral("gl_NumSamples"),
        QStringLiteral("gl_ViewIndex"),
        QStringLiteral("gl_DeviceIndex"),
        QStringLiteral("gl_RayTmax"),        QStringLiteral("gl_RayOrigin"),
        QStringLiteral("gl_HitT"),           QStringLiteral("gl_InstanceCustomIndex"),
        QStringLiteral("gl_WorldRayOrigin"), QStringLiteral("gl_WorldRayDirection"),
    };
}

}  // namespace

QString CompletionItem::displaySignature() const {
    if (signature.isEmpty()) {
        return name;
    }
    return name + signature;
}

// --- keyword groups ----------------------------------------------------------

const QStringList& GlslLanguageData::scalarTypes() {
    static const QStringList list{
        QStringLiteral("void"),   QStringLiteral("bool"),  QStringLiteral("int"),
        QStringLiteral("uint"),   QStringLiteral("float"), QStringLiteral("double"),
        QStringLiteral("int8_t"), QStringLiteral("int16_t"), QStringLiteral("int32_t"),
        QStringLiteral("int64_t"), QStringLiteral("uint8_t"), QStringLiteral("uint16_t"),
        QStringLiteral("uint32_t"), QStringLiteral("uint64_t"),
        QStringLiteral("float16_t"), QStringLiteral("float32_t"), QStringLiteral("float64_t"),
        QStringLiteral("atomic_uint"),
    };
    return list;
}

const QStringList& GlslLanguageData::vectorTypes() {
    static const QStringList list{
        QStringLiteral("vec2"),  QStringLiteral("vec3"),  QStringLiteral("vec4"),
        QStringLiteral("ivec2"), QStringLiteral("ivec3"), QStringLiteral("ivec4"),
        QStringLiteral("uvec2"), QStringLiteral("uvec3"), QStringLiteral("uvec4"),
        QStringLiteral("bvec2"), QStringLiteral("bvec3"), QStringLiteral("bvec4"),
        QStringLiteral("dvec2"), QStringLiteral("dvec3"), QStringLiteral("dvec4"),
        QStringLiteral("f16vec2"), QStringLiteral("f16vec3"), QStringLiteral("f16vec4"),
        QStringLiteral("i8vec2"), QStringLiteral("i8vec3"), QStringLiteral("i8vec4"),
        QStringLiteral("u8vec2"), QStringLiteral("u8vec3"), QStringLiteral("u8vec4"),
        QStringLiteral("i16vec2"), QStringLiteral("i16vec3"), QStringLiteral("i16vec4"),
        QStringLiteral("u16vec2"), QStringLiteral("u16vec3"), QStringLiteral("u16vec4"),
        QStringLiteral("i64vec2"), QStringLiteral("i64vec3"), QStringLiteral("i64vec4"),
        QStringLiteral("u64vec2"), QStringLiteral("u64vec3"), QStringLiteral("u64vec4"),
    };
    return list;
}

const QStringList& GlslLanguageData::matrixTypes() {
    static const QStringList list{
        QStringLiteral("mat2"),    QStringLiteral("mat3"),    QStringLiteral("mat4"),
        QStringLiteral("mat2x2"),  QStringLiteral("mat2x3"),  QStringLiteral("mat2x4"),
        QStringLiteral("mat3x2"),  QStringLiteral("mat3x3"),  QStringLiteral("mat3x4"),
        QStringLiteral("mat4x2"),  QStringLiteral("mat4x3"),  QStringLiteral("mat4x4"),
        QStringLiteral("dmat2"),   QStringLiteral("dmat3"),   QStringLiteral("dmat4"),
        QStringLiteral("dmat2x2"), QStringLiteral("dmat2x3"), QStringLiteral("dmat2x4"),
        QStringLiteral("dmat3x2"), QStringLiteral("dmat3x3"), QStringLiteral("dmat3x4"),
        QStringLiteral("dmat4x2"), QStringLiteral("dmat4x3"), QStringLiteral("dmat4x4"),
        QStringLiteral("f16mat2"), QStringLiteral("f16mat3"), QStringLiteral("f16mat4"),
    };
    return list;
}

const QStringList& GlslLanguageData::opaqueTypes() {
    static const QStringList list{
        // shadow samplers
        QStringLiteral("sampler1DShadow"),      QStringLiteral("sampler2DShadow"),
        QStringLiteral("samplerCubeShadow"),    QStringLiteral("sampler1DArrayShadow"),
        QStringLiteral("sampler2DArrayShadow"), QStringLiteral("samplerCubeArrayShadow"),
        QStringLiteral("sampler2DRectShadow"),
        // plain samplers
        QStringLiteral("sampler1D"),       QStringLiteral("sampler2D"),
        QStringLiteral("sampler3D"),       QStringLiteral("samplerCube"),
        QStringLiteral("sampler2DRect"),   QStringLiteral("sampler1DArray"),
        QStringLiteral("sampler2DArray"),  QStringLiteral("samplerCubeArray"),
        QStringLiteral("samplerBuffer"),   QStringLiteral("sampler2DMS"),
        QStringLiteral("sampler2DMSArray"),
        // integer samplers
        QStringLiteral("isampler1D"),      QStringLiteral("isampler2D"),
        QStringLiteral("isampler3D"),      QStringLiteral("isamplerCube"),
        QStringLiteral("isampler2DRect"),  QStringLiteral("isampler1DArray"),
        QStringLiteral("isampler2DArray"), QStringLiteral("isamplerCubeArray"),
        QStringLiteral("isamplerBuffer"),  QStringLiteral("isampler2DMS"),
        QStringLiteral("isampler2DMSArray"),
        // unsigned samplers
        QStringLiteral("usampler1D"),      QStringLiteral("usampler2D"),
        QStringLiteral("usampler3D"),      QStringLiteral("usamplerCube"),
        QStringLiteral("usampler2DRect"),  QStringLiteral("usampler1DArray"),
        QStringLiteral("usampler2DArray"), QStringLiteral("usamplerCubeArray"),
        QStringLiteral("usamplerBuffer"),  QStringLiteral("usampler2DMS"),
        QStringLiteral("usampler2DMSArray"),
        // images
        QStringLiteral("image1D"),       QStringLiteral("image2D"),
        QStringLiteral("image3D"),       QStringLiteral("imageCube"),
        QStringLiteral("image2DRect"),   QStringLiteral("image1DArray"),
        QStringLiteral("image2DArray"),  QStringLiteral("imageCubeArray"),
        QStringLiteral("imageBuffer"),   QStringLiteral("image2DMS"),
        QStringLiteral("image2DMSArray"),
        QStringLiteral("iimage1D"),      QStringLiteral("iimage2D"),
        QStringLiteral("iimage3D"),      QStringLiteral("iimageCube"),
        QStringLiteral("iimage2DRect"),  QStringLiteral("iimage1DArray"),
        QStringLiteral("iimage2DArray"), QStringLiteral("iimageCubeArray"),
        QStringLiteral("iimageBuffer"),  QStringLiteral("iimage2DMS"),
        QStringLiteral("iimage2DMSArray"),
        QStringLiteral("uimage1D"),      QStringLiteral("uimage2D"),
        QStringLiteral("uimage3D"),      QStringLiteral("uimageCube"),
        QStringLiteral("uimage2DRect"),  QStringLiteral("uimage1DArray"),
        QStringLiteral("uimage2DArray"), QStringLiteral("uimageCubeArray"),
        QStringLiteral("uimageBuffer"),  QStringLiteral("uimage2DMS"),
        QStringLiteral("uimage2DMSArray"),
        // Vulkan-specific
        QStringLiteral("subpassInput"),    QStringLiteral("subpassInputMS"),
        QStringLiteral("isubpassInput"),   QStringLiteral("isubpassInputMS"),
        QStringLiteral("usubpassInput"),   QStringLiteral("usubpassInputMS"),
        QStringLiteral("texture1D"),       QStringLiteral("texture2D"),
        QStringLiteral("texture3D"),       QStringLiteral("textureCube"),
        QStringLiteral("texture2DRect"),   QStringLiteral("texture1DArray"),
        QStringLiteral("texture2DArray"),  QStringLiteral("textureCubeArray"),
        QStringLiteral("textureBuffer"),   QStringLiteral("texture2DMS"),
        QStringLiteral("texture2DMSArray"),
        QStringLiteral("sampler"),         QStringLiteral("samplerShadow"),
        QStringLiteral("accelerationStructureEXT"),
        QStringLiteral("rayQueryEXT"),
    };
    return list;
}

const QStringList& GlslLanguageData::qualifiers() {
    static const QStringList list{
        QStringLiteral("const"),        QStringLiteral("in"),          QStringLiteral("out"),
        QStringLiteral("inout"),        QStringLiteral("uniform"),     QStringLiteral("buffer"),
        QStringLiteral("shared"),       QStringLiteral("attribute"),   QStringLiteral("varying"),
        QStringLiteral("centroid"),     QStringLiteral("flat"),        QStringLiteral("smooth"),
        QStringLiteral("noperspective"),QStringLiteral("patch"),       QStringLiteral("sample"),
        QStringLiteral("coherent"),     QStringLiteral("volatile"),    QStringLiteral("restrict"),
        QStringLiteral("readonly"),     QStringLiteral("writeonly"),   QStringLiteral("layout"),
        QStringLiteral("precise"),      QStringLiteral("invariant"),   QStringLiteral("lowp"),
        QStringLiteral("mediump"),      QStringLiteral("highp"),       QStringLiteral("precision"),
        QStringLiteral("subroutine"),   QStringLiteral("nonuniformEXT"),
        QStringLiteral("perprimitiveEXT"), QStringLiteral("perviewNV"),
        QStringLiteral("pervertexNV"),  QStringLiteral("perprimitiveNV"),
        QStringLiteral("taskPayloadSharedEXT"),
    };
    return list;
}

const QStringList& GlslLanguageData::controlFlow() {
    static const QStringList list{
        QStringLiteral("if"),       QStringLiteral("else"),     QStringLiteral("for"),
        QStringLiteral("while"),    QStringLiteral("do"),       QStringLiteral("switch"),
        QStringLiteral("case"),     QStringLiteral("default"),  QStringLiteral("break"),
        QStringLiteral("continue"), QStringLiteral("return"),   QStringLiteral("discard"),
        QStringLiteral("struct"),   QStringLiteral("true"),     QStringLiteral("false"),
        QStringLiteral("terminateRayEXT"), QStringLiteral("ignoreIntersectionEXT"),
    };
    return list;
}

QStringList GlslLanguageData::allTypes() {
    QStringList types;
    types << scalarTypes() << vectorTypes() << matrixTypes() << opaqueTypes();
    return types;
}

const QVector<LayoutQualifier>& GlslLanguageData::layoutQualifiers() {
    static const QVector<LayoutQualifier> list{
        {"location", "Position of a vertex attribute / shader IO"},
        {"component", "Component index, used to pack several small types"},
        {"binding", "Descriptor binding point"},
        {"set", "Descriptor set number"},
        {"offset", "Byte offset of a block member in std140/std430"},
        {"align", "Alignment in bytes of a block member"},
        {"std140", "The uniform block uses the std140 layout"},
        {"std430", "The storage block uses the std430 layout"},
        {"scalar", "Scalar block layout (VK_EXT_scalar_block_layout)"},
        {"push_constant", "Declares a push constant block"},
        {"input_attachment_index", "Input attachment index of a subpassInput"},
        {"constant_id", "Specialization constant id"},
        {"index", "Output index for dual-source blending"},
        {"local_size_x", "Compute work-group size in X"},
        {"local_size_y", "Compute work-group size in Y"},
        {"local_size_z", "Compute work-group size in Z"},
        {"local_size_x_id", "Work-group size in X, given as a specialization constant"},
        {"local_size_y_id", "Work-group size in Y, given as a specialization constant"},
        {"local_size_z_id", "Work-group size in Z, given as a specialization constant"},
        {"vertices", "Number of output vertices of a geometry shader"},
        {"invocations", "Number of invocations of a geometry/tessellation shader"},
        {"max_vertices", "Maximum number of output vertices of a mesh shader"},
        {"max_primitives", "Maximum number of output primitives of a mesh shader"},
        {"triangles", "Input/output primitive type: triangles"},
        {"quads", "Tessellation input primitive type: quads"},
        {"isolines", "Tessellation input primitive type: isolines"},
        {"equal_spacing", "Tessellation spacing: equal"},
        {"fractional_even_spacing", "Tessellation spacing: fractional even"},
        {"fractional_odd_spacing", "Tessellation spacing: fractional odd"},
        {"cw", "Clockwise is front-facing"},
        {"ccw", "Counter-clockwise is front-facing"},
        {"point_mode", "Tessellation output point mode"},
        {"points", "Input/output primitive type: points"},
        {"lines", "Input/output primitive type: lines"},
        {"lines_adjacency", "Input primitive type: lines with adjacency"},
        {"triangles_adjacency", "Input primitive type: triangles with adjacency"},
        {"line_strip", "Output primitive type: line strip"},
        {"triangle_strip", "Output primitive type: triangle strip"},
        {"early_fragment_tests", "Runs the depth test early"},
        {"depth_greater", "Depth comparison direction"},
        {"depth_less", "Depth comparison direction"},
        {"origin_upper_left", "The fragment coordinate origin is the upper left"},
        {"pixel_center_integer", "Shifts fragment coordinates onto pixel centres"},
        {"xfb_buffer", "Transform feedback buffer index"},
        {"xfb_stride", "Transform feedback stride"},
        {"xfb_offset", "Transform feedback offset"},
        {"stream", "Transform feedback stream number"},
        {"shared", "Shared variable of a compute shader"},
        {"packed", "Packed layout (for compatibility)"},
        {"row_major", "Row-major matrix layout"},
        {"column_major", "Column-major matrix layout"},
        {"rgba32f", "Image format qualifier"},
        {"rgba16f", "Image format qualifier"},
        {"rg32f", "Image format qualifier"},
        {"r32f", "Image format qualifier"},
        {"rgba8", "Image format qualifier"},
        {"rgba8_snorm", "Image format qualifier"},
        {"r32ui", "Image format qualifier"},
        {"r32i", "Image format qualifier"},
        {"rayPayloadEXT", "Ray tracing payload qualifier"},
        {"rayPayloadInEXT", "Ray tracing payload input qualifier"},
        {"hitAttributeEXT", "Ray tracing hit attribute qualifier"},
        {"shaderRecordEXT", "Shader record qualifier"},
    };
    return list;
}

const QStringList& GlslLanguageData::builtinVariables() {
    static const QStringList list = makeBuiltinVariables();
    return list;
}

const QStringList& GlslLanguageData::preprocessorDirectives() {
    static const QStringList list{
        QStringLiteral("version"), QStringLiteral("define"),   QStringLiteral("undef"),
        QStringLiteral("include"), QStringLiteral("if"),       QStringLiteral("ifdef"),
        QStringLiteral("ifndef"),  QStringLiteral("else"),     QStringLiteral("elif"),
        QStringLiteral("endif"),   QStringLiteral("error"),    QStringLiteral("pragma"),
        QStringLiteral("extension"), QStringLiteral("line"),
    };
    return list;
}

// --- completion --------------------------------------------------------------

const QVector<CompletionItem>& GlslLanguageData::builtinFunctions() {
    static const QVector<CompletionItem> list = makeBuiltinFunctions();
    return list;
}

QVector<CompletionItem> GlslLanguageData::typeCompletions() {
    QVector<CompletionItem> out;
    const auto add = [&out](const QStringList& names, const QString& description) {
        for (const QString& name : names) {
            CompletionItem item;
            item.name = name;
            item.description = description;
            out.append(item);
        }
    };
    add(scalarTypes(), trs(QT_TRANSLATE_NOOP("GlslLanguageData", "Scalar type")));
    add(vectorTypes(), trs(QT_TRANSLATE_NOOP("GlslLanguageData", "Vector type")));
    add(matrixTypes(), trs(QT_TRANSLATE_NOOP("GlslLanguageData", "Matrix type")));
    add(opaqueTypes(), trs(QT_TRANSLATE_NOOP("GlslLanguageData", "Opaque type (sampler / image)")));
    for (const LayoutQualifier& qualifier : layoutQualifiers()) {
        CompletionItem item;
        item.name = QString::fromLatin1(qualifier.name);
        item.description = trs(QT_TRANSLATE_NOOP("GlslLanguageData", qualifier.description));
        out.append(item);
    }
    return out;
}

QVector<CompletionItem> GlslLanguageData::keywordCompletions() {
    QVector<CompletionItem> out;
    const auto add = [&out](const QStringList& names, const QString& description) {
        for (const QString& name : names) {
            CompletionItem item;
            item.name = name;
            item.description = description;
            out.append(item);
        }
    };
    add(qualifiers(), trs(QT_TRANSLATE_NOOP("GlslLanguageData", "Qualifier / storage class")));
    add(controlFlow(), trs(QT_TRANSLATE_NOOP("GlslLanguageData", "Control-flow keyword")));
    add(builtinVariables(), trs(QT_TRANSLATE_NOOP("GlslLanguageData", "Built-in variable")));
    for (const QString& directive : preprocessorDirectives()) {
        CompletionItem item;
        item.name = QLatin1Char('#') + directive;
        item.description = trs(QT_TRANSLATE_NOOP("GlslLanguageData", "Preprocessor directive"));
        out.append(item);
    }
    return out;
}

const CompletionItem* GlslLanguageData::findBuiltinFunction(const QString& name) {
    const QVector<CompletionItem>& functions = builtinFunctions();
    for (const CompletionItem& item : functions) {
        if (item.name == name) {
            return &item;
        }
    }
    return nullptr;
}

}  // namespace spall
