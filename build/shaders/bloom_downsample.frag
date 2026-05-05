#version 330 core
out vec4 FragColor;
in vec2 v_texCoord;

uniform sampler2D u_srcTexture;
uniform vec2 u_srcResolution;
uniform int u_mipLevel;
uniform vec4 u_filter; // x: threshold, y: threshold - knee, z: 2 * knee, w: 0.25 / knee

// ============================================================
//  bloom_downsample.frag - Progressive Box Filter Downsample
// ============================================================
void main() {
    vec2 texelSize = 1.0 / u_srcResolution;
    // Box filter: 4 samples offset by half a texel (1.0 for downsampling to half-size)
    vec2 o1 = vec2(-1.0, 1.0) * texelSize;
    vec2 o2 = vec2(1.0, 1.0) * texelSize;
    vec2 o3 = vec2(1.0, -1.0) * texelSize;
    vec2 o4 = vec2(-1.0, -1.0) * texelSize;
    
    vec3 s = texture(u_srcTexture, v_texCoord + o1).rgb +
             texture(u_srcTexture, v_texCoord + o2).rgb +
             texture(u_srcTexture, v_texCoord + o3).rgb +
             texture(u_srcTexture, v_texCoord + o4).rgb;
    vec3 result = s * 0.25;
    
    // Apply soft threshold prefilter on the first mip (mipLevel == 0)
    if (u_mipLevel == 0) {
        float brightness = max(result.r, max(result.g, result.b));
        float soft = brightness - u_filter.y;
        soft = clamp(soft, 0.0, u_filter.z);
        soft = soft * soft * u_filter.w;
        float contribution = max(soft, brightness - u_filter.x);
        contribution /= max(brightness, 0.00001);
        result *= contribution;
    }
    
    FragColor = vec4(result, 1.0);
}
