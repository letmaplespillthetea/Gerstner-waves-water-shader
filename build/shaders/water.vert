// ============================================================
//  water.vert - Vertex Shader with Distance-based Attenuation
// ============================================================
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform float u_time;
uniform vec3 u_cameraPos;

#define MAX_WAVES 16
struct Wave {
    vec2  direction;
    float amplitude;
    float frequency;
    float speed;
    float steepness;
};

uniform Wave u_waves[MAX_WAVES];
uniform int u_waveCount;

out vec3 v_worldPos;
out vec3 v_normal;
out vec2 v_texCoord;

void main() {
    vec3 gridPos = a_position;
    
    // Calculate distance from camera to the vertex in world space
    vec3 worldPosBase = (u_model * vec4(gridPos, 1.0)).xyz;
    float dist = length(worldPosBase - u_cameraPos);
    
    // Attenuation factor: 1.0 near camera, fades to 0.0 at 450 units
    // This makes the horizon look calm while keeping nearby waves active
    float att = clamp(1.0 - (dist / 450.0), 0.0, 1.0);
    att = pow(att, 2.5); // Faster falloff for a more distinct horizon line

    vec3 displacement = vec3(0.0);
    float dX = 0.0;
    float dZ = 0.0;

    for (int i = 0; i < u_waveCount; ++i) {
        Wave w = u_waves[i];
        float freq = w.frequency;
        float amp = w.amplitude;
        float steep = w.steepness;
        float phase = w.speed * freq;

        // Wave phase calculation
        float d = dot(w.direction, gridPos.xz);
        float theta = freq * d + phase * u_time;

        float sinT = sin(theta);
        float cosT = cos(theta);

        // Accumulate Gerstner horizontal and vertical displacement
        displacement.x += w.direction.x * (steep * amp * cosT);
        displacement.y += amp * sinT;
        displacement.z += w.direction.y * (steep * amp * cosT);

        // Partial derivatives for analytical normal calculation
        float wa = freq * amp;
        dX += w.direction.x * wa * cosT;
        dZ += w.direction.y * wa * cosT;
    }

    // Apply distance attenuation to both height and horizontal shift
    vec3 finalPos = gridPos + (displacement * att);
    
    // Smooth out the normal back to (0, 1, 0) as distance increases
    vec3 normal = normalize(vec3(-dX * att, 1.0, -dZ * att));

    v_worldPos = (u_model * vec4(finalPos, 1.0)).xyz;
    v_normal = normalize(mat3(transpose(inverse(u_model))) * normal);
    v_texCoord = a_texCoord;

    gl_Position = u_projection * u_view * vec4(v_worldPos, 1.0);
}
