#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// ============================================================
//  WaterMesh - Generates a subdivided flat grid in XZ plane.
//  The vertex shader will displace Y via Gerstner Waves.
//
//  Layout (per vertex):
//    location 0 -> position  (vec3) - flat XZ grid, Y = 0
//    location 1 -> texCoord  (vec2) - UV for future texturing
// ============================================================
class WaterMesh {
public:
    unsigned int VAO = 0;
    int indexCount   = 0; // Total indices for glDrawElements

    // size     : world-space side length of the water plane
    // segments : number of grid cells per side (higher = smoother waves)
    WaterMesh(float size = 100.0f, int segments = 200) {
        buildMesh(size, segments);
    }

    ~WaterMesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

private:
    unsigned int VBO = 0, EBO = 0;

    void buildMesh(float size, int segments) {
        // Number of vertices per side
        int vCount = segments + 1;
        float step = size / (float)segments;
        float half = size * 0.5f;

        // Each vertex: 3 floats position + 2 floats UV
        std::vector<float> vertices;
        vertices.reserve(vCount * vCount * 5);

        for (int z = 0; z < vCount; ++z) {
            for (int x = 0; x < vCount; ++x) {
                float px = -half + x * step;
                float pz = -half + z * step;

                // Flat Y - displacement happens in vertex shader
                vertices.push_back(px);
                vertices.push_back(0.0f);
                vertices.push_back(pz);

                // UV coordinates normalised [0, 1]
                vertices.push_back((float)x / segments);
                vertices.push_back((float)z / segments);
            }
        }

        // Two triangles per grid cell
        std::vector<unsigned int> indices;
        indices.reserve(segments * segments * 6);

        for (int z = 0; z < segments; ++z) {
            for (int x = 0; x < segments; ++x) {
                unsigned int topLeft     = z * vCount + x;
                unsigned int topRight    = topLeft + 1;
                unsigned int bottomLeft  = (z + 1) * vCount + x;
                unsigned int bottomRight = bottomLeft + 1;

                // Triangle 1
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // Triangle 2
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        indexCount = (int)indices.size();

        // Upload to GPU
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // attribute 0: position (xyz)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // attribute 1: UV
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
};
