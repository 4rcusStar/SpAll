#version 450

// basic.frag — matches basic.vert: a flat colour, no descriptors at all.

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
