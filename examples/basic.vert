#version 450

// basic.vert — smallest useful Vulkan vertex shader.
//
// Generates a full-screen triangle from gl_VertexIndex alone, so the shader needs
// no vertex buffer and the reflection output stays trivial to read.

void main()
{
    // Three vertices, drawn with vkCmdDraw(cmd, 3, 1, 0, 0).
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
