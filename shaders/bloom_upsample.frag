#version 330 core
out vec4 FragColor;
in vec2 v_texCoord;

uniform sampler2D u_srcTexture;
uniform vec2 u_srcResolution;

// ============================================================
//  bloom_upsample.frag - Progressive Box Filter Upsample
// ============================================================
void main() {
    vec2 texelSize = 1.0 / u_srcResolution;
    // Box filter: 4 samples offset by 0.5 texels to overlap 3x3 footprint
    vec2 o1 = vec2(-0.5, 0.5) * texelSize;
    vec2 o2 = vec2(0.5, 0.5) * texelSize;
    vec2 o3 = vec2(0.5, -0.5) * texelSize;
    vec2 o4 = vec2(-0.5, -0.5) * texelSize;
    
    vec3 s = texture(u_srcTexture, v_texCoord + o1).rgb +
             texture(u_srcTexture, v_texCoord + o2).rgb +
             texture(u_srcTexture, v_texCoord + o3).rgb +
             texture(u_srcTexture, v_texCoord + o4).rgb;
             
    FragColor = vec4(s * 0.25, 1.0);
}
