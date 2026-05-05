#version 330 core
out vec4 FragColor;
in vec2 v_texCoord;

uniform sampler2D u_scene;
uniform sampler2D u_bloomBlur;
uniform float u_bloomIntensity;
uniform int u_isUnderwater;

// ============================================================
//  post_final.frag - Final composite with Bloom and Tone Mapping
// ============================================================
void main() {             
    const float gamma = 2.2;
    vec3 hdrColor = texture(u_scene, v_texCoord).rgb;      
    vec3 bloomColor = texture(u_bloomBlur, v_texCoord).rgb;
    
    if (u_isUnderwater == 1) {
        // Apply a deep blue-cyan tint and reduce brightness to simulate being underwater
        hdrColor *= vec3(0.1, 0.4, 0.6);
        bloomColor *= vec3(0.5, 0.8, 1.0);
    }
    
    // Additive blending of bloom
    hdrColor += bloomColor * u_bloomIntensity; 
    
    // Simple Exposure Tone Mapping
    vec3 result = vec3(1.0) - exp(-hdrColor);
    
    // Gamma correction
    result = pow(result, vec3(1.0 / gamma));
    
    FragColor = vec4(result, 1.0);
}
