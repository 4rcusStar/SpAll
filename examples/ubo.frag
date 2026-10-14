#version 450

// ubo.frag — exercises every descriptor kind the reflection panel reports:
// a UBO, a combined image sampler, and a push constant block.
//
// Expected reflection output:
//   set=0 binding=0  uniform buffer          Material  (16 bytes, member "tint" at offset 0)
//   set=0 binding=1  combined image sampler  albedo
//   push constant    PushConstants          offset=0 size=16

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform Material
{
    vec4 tint;   // offset 0, 16 bytes
} material;

layout(set = 0, binding = 1) uniform sampler2D albedo;

layout(push_constant) uniform PushConstants
{
    vec4 uvScaleBias;   // xy = scale, zw = bias; offset 0, 16 bytes
} push;

void main()
{
    // gl_FragCoord is a builtin input; the reflection panel should list it as one.
    vec2 uv = gl_FragCoord.xy * push.uvScaleBias.xy + push.uvScaleBias.zw;
    fragColor = texture(albedo, uv) * material.tint;
}
