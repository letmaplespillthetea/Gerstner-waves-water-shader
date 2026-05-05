#version 330 core
layout (location = 0) out vec4 FragColor;
in vec2 v_texCoord;

uniform sampler2D u_scene;
uniform float u_bloomThreshold;

// ============================================================
//  bloom_extract.frag - Extract bright parts of the HDR scene
// ============================================================
void main() {
    vec3 color = texture(u_scene, v_texCoord).rgb;
    // Calculate relative luminance
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // Extract only fragments that are brighter than the UI threshold
    if(brightness > u_bloomThreshold)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
