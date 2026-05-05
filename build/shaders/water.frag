// ============================================================
//  water.frag - Fragment Shader
//
//  Implements:
//    1. Blinn-Phong lighting (ambient + diffuse + specular)
//    2. Fresnel effect: mix between deep-water colour and
//       sky reflection colour based on view angle.
//    3. Depth-based colour darkening for realism.
// ============================================================
#version 330 core

// ---- Inputs from vertex shader ----
in vec3 v_worldPos;
in vec3 v_normal;
in vec2 v_texCoord;

// ---- Camera & light ----
uniform vec3 u_cameraPos;
uniform vec3 u_lightDir;   // Direction TOWARDS the light (normalised)
uniform vec3 u_lightColor;

// ---- Water appearance tunables ----
uniform vec3  u_shallowColor; // Water colour near the surface
uniform vec3  u_deepColor;    // Water colour in deep/shadowed areas
uniform float u_time;
uniform float u_ambientStrength; // New uniform for UI control

out vec4 fragColor;

// ============================================================
//  Schlick approximation for Fresnel reflectance.
//  F(theta) = F0 + (1 - F0) * (1 - cos(theta))^5
//  For water, F0 ~ 0.02 (2 % base reflectance).
// ============================================================
float fresnelSchlick(float cosTheta, float F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // ---- Normalise interpolated vectors ----
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_lightDir);
    vec3 V = normalize(u_cameraPos - v_worldPos); // View direction

    // Ensure normal faces viewer (two-sided water)
    if (dot(N, V) < 0.0) N = -N;

    // ---- Blinn-Phong lighting ----
    // Ambient - base sky light scattered into the water
    vec3  ambient = u_ambientStrength * u_lightColor;

    // Diffuse - Lambert's cosine law
    float diff   = max(dot(N, L), 0.0);
    vec3  diffuse = diff * u_lightColor;

    // Specular - Blinn-Phong half-vector
    vec3  H        = normalize(L + V);           // Half-vector
    float shininess = 256.0;                     // Higher = sharper sun glint
    float spec     = pow(max(dot(N, H), 0.0), shininess);
    vec3  specular  = spec * u_lightColor * 4.0; // Stronger sun reflection boost

    // ---- Depth-based colour mixing ----
    // Use world Y displacement as a cheap "depth" proxy:
    // high crests -> shallow colour; troughs -> deep colour
    float depthFactor = clamp((v_worldPos.y + 1.5) / 3.0, 0.0, 1.0);
    vec3  waterColor  = mix(u_deepColor, u_shallowColor, depthFactor);

    // ---- Fresnel effect ----
    float cosTheta   = max(dot(N, V), 0.0);
    float F0         = 0.02; // Water-air boundary base reflectance
    float fresnel    = fresnelSchlick(cosTheta, F0);

    // Sky / reflection colour (simplified sky gradient as stand-in)
    // At low angles the water reflects the sky; at high angles deep water
    vec3 skyColor = mix(vec3(0.53f, 0.81f, 0.92f),   // Horizon haze
                        vec3(0.10f, 0.25f, 0.60f),    // Zenith blue
                        clamp(V.y * 2.0, 0.0, 1.0));

    // Blend water colour with sky reflection via Fresnel
    vec3 baseColor = mix(waterColor, skyColor, fresnel * 0.9);

    // ---- Combine lighting ----
    vec3 litColor = (ambient + diffuse) * baseColor + specular;

    // ---- Atmospheric Fog & Sun Glow ----
    // Calculate distance from camera to the water surface
    float dist = length(v_worldPos - u_cameraPos);

    // Calculate how much the viewer is looking towards the sun direction
    float sunAlignment = max(dot(V, L), 0.0);

    // Fog intensity increases with distance - tweaked for a more ethereal look
    // Higher power (2.0) creates a softer, more "dreamy" transition at the horizon
    float fogFactor = clamp(dist / 420.0, 0.0, 1.0);
    fogFactor = pow(fogFactor, 2.0); 

    // Horizon color: Very bright near the sun disc to simulate scattering/glow
    vec3 skyBlue = vec3(0.5, 0.7, 0.9);
    vec3 sunGlow = vec3(1.0, 1.0, 0.95);
    // Lowered power to 3.0 for a wider glow matching the larger sun
    vec3 horizonColor = mix(skyBlue, sunGlow, pow(sunAlignment, 3.0));

    // Blend the lit water color into the thick horizon fog
    litColor = mix(litColor, horizonColor, fogFactor);

    fragColor = vec4(litColor, 0.92);
}
