#version 330 core
out vec4 fragColor;

uniform vec3 u_sunColor;

// ============================================================
//  sun.frag - Fragment shader to render a bright sun disc
// ============================================================
void main() {
    // Output a solid, bright emissive color
    fragColor = vec4(u_sunColor, 1.0);
}
