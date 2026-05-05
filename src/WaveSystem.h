#pragma once

#include "Shader.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

// ============================================================
//  GerstnerWave - Parameters for a single Gerstner wave.
//
//  Gerstner (trochoidal) wave formula:
//    P(x,z,t) = (x + Q*A * D.x * cos(w * dot(D,xz) + phi*t),
//               A * sin(w * dot(D,xz) + phi*t),
//               z + Q*A * D.z * cos(w * dot(D,xz) + phi*t))
//
//  Where:
//    D     = normalized 2D direction vector
//    A     = amplitude
//    w     = frequency (angular, = 2*PI / wavelength)
//    phi   = phase speed (= speed * w)
//    Q     = steepness in [0, 1] - horizontal displacement factor
// ============================================================
struct GerstnerWave {
  glm::vec2 direction; // 2D direction (will be normalised on upload)
  float amplitude;     // Height of wave crest above rest surface
  float frequency;     // Angular frequency (controls wavelength)
  float speed;         // Phase speed (distance per second)
  float steepness;     // Horizontal sharpening [0 = sine, 1 = pointed crest]

  GerstnerWave(glm::vec2 dir = glm::vec2(1.0f, 0.0f), float amp = 0.1f,
               float freq = 0.2f, float spd = 1.0f, float steep = 0.5f)
      : direction(glm::normalize(dir)), amplitude(amp), frequency(freq),
        speed(spd), steepness(steep) {}
};

// ============================================================
//  WaveSystem - Manages a collection of GerstnerWaves and
//               uploads their data to the shader as uniform arrays.
//
//  The shader supports up to MAX_WAVES waves. Additional waves
//  can be added at runtime up to that limit.
// ============================================================
class WaveSystem {
public:
  static constexpr int MAX_WAVES = 16; // Shader array capacity

  std::vector<GerstnerWave> waves;

  WaveSystem() {
    // ---- Default 16 waves for simulation ----

    // Layer 1: Large Swells (Low frequency, Medium amplitude)
    addWave(GerstnerWave(glm::vec2(1.0f, 0.15f), 0.20f, 0.22f, 1.6f, 0.15f));
    addWave(GerstnerWave(glm::vec2(0.8f, 0.30f), 0.25f, 0.25f, 1.4f, 0.12f));
    addWave(GerstnerWave(glm::vec2(0.9f, -0.10f), 0.22f, 0.28f, 1.8f, 0.15f));
    addWave(GerstnerWave(glm::vec2(0.1f, 0.95f), 0.21f, 0.35f, 1.5f, 0.10f));

    // Layer 2: Medium Smooth Waves (Turbulence & Variation)
    addWave(GerstnerWave(glm::vec2(-0.5f, 0.75f), 0.1f, 0.50f, 2.0f, 0.10f));
    addWave(GerstnerWave(glm::vec2(0.35f, 0.85f), 0.18f, 0.60f, 1.8f, 0.12f));
    addWave(GerstnerWave(glm::vec2(-0.75f, -0.25f), 0.15f, 0.70f, 2.3f, 0.08f));
    addWave(GerstnerWave(glm::vec2(0.25f, -0.85f), 0.13f, 0.80f, 2.1f, 0.10f));
    addWave(GerstnerWave(glm::vec2(0.95f, 0.70f), 0.14f, 0.90f, 1.9f, 0.08f));
    addWave(GerstnerWave(glm::vec2(-0.10f, -0.98f), 0.09f, 0.75f, 2.2f, 0.10f));

    // Layer 3: Surface Ripples (Fine details & High-frequency glints)
    addWave(GerstnerWave(glm::vec2(0.45f, -0.55f), 0.06f, 1.50f, 3.5f, 0.05f));
    addWave(GerstnerWave(glm::vec2(-0.15f, 0.75f), 0.05f, 2.20f, 4.0f, 0.05f));
    addWave(GerstnerWave(glm::vec2(0.85f, 0.15f), 0.04f, 3.00f, 3.8f, 0.05f));
    addWave(GerstnerWave(glm::vec2(-0.65f, -0.75f), 0.03f, 3.50f, 4.5f, 0.05f));
    addWave(GerstnerWave(glm::vec2(0.98f, 0.10f), 0.03f, 2.80f, 4.2f, 0.05f));
    addWave(GerstnerWave(glm::vec2(-0.25f, 0.92f), 0.02f, 4.50f, 5.0f, 0.05f));
  }

  // Add a new wave (up to MAX_WAVES)
  bool addWave(const GerstnerWave &wave) {
    if ((int)waves.size() >= MAX_WAVES) {
      return false; // Capacity reached
    }
    waves.push_back(wave);
    return true;
  }

  // Remove a wave at specific index
  void removeWave(int index) {
    if (index >= 0 && index < (int)waves.size()) {
      waves.erase(waves.begin() + index);
    }
  }

  // Upload all wave parameters to shader as uniform arrays.
  // Naming convention mirrors the shader: waves[i].direction, etc.
  void uploadToShader(const Shader &shader) const {
    shader.setInt("u_waveCount", (int)waves.size());

    for (int i = 0; i < (int)waves.size(); ++i) {
      std::string base = "u_waves[" + std::to_string(i) + "]";
      const GerstnerWave &w = waves[i];

      glm::vec2 dir = glm::normalize(w.direction);
      glUniform2f(
          glGetUniformLocation(shader.id, (base + ".direction").c_str()), dir.x,
          dir.y);
      glUniform1f(
          glGetUniformLocation(shader.id, (base + ".amplitude").c_str()),
          w.amplitude);
      glUniform1f(
          glGetUniformLocation(shader.id, (base + ".frequency").c_str()),
          w.frequency);
      glUniform1f(glGetUniformLocation(shader.id, (base + ".speed").c_str()),
                  w.speed);
      glUniform1f(
          glGetUniformLocation(shader.id, (base + ".steepness").c_str()),
          w.steepness);
    }
  }
};
