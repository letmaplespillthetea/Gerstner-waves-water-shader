#version 330 core
out vec4 FragColor;
in vec2 v_texCoord;

uniform sampler2D u_image;
uniform bool u_horizontal;
uniform float u_weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

// ============================================================
//  blur.frag - Gaussian Blur shader for Bloom effect
// ============================================================
void main() {             
    vec2 tex_offset = 1.0 / textureSize(u_image, 0); 
    vec3 result = texture(u_image, v_texCoord).rgb * u_weight[0]; 
    
    if(u_horizontal) {
        for(int i = 1; i < 5; ++i) {
            result += texture(u_image, v_texCoord + vec2(tex_offset.x * i, 0.0)).rgb * u_weight[i];
            result += texture(u_image, v_texCoord - vec2(tex_offset.x * i, 0.0)).rgb * u_weight[i];
        }
    } else {
        for(int i = 1; i < 5; ++i) {
            result += texture(u_image, v_texCoord + vec2(0.0, tex_offset.y * i)).rgb * u_weight[i];
            result += texture(u_image, v_texCoord - vec2(0.0, tex_offset.y * i)).rgb * u_weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
