#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texCoords;

out vec2 v_texCoord;

// ============================================================
//  post.vert - Full-screen quad vertex shader
// ============================================================
void main() {
    v_texCoord = a_texCoords;
    gl_Position = vec4(a_pos.x, a_pos.y, 0.0, 1.0);
}
