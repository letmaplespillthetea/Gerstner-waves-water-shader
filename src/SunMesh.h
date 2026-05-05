#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

// ============================================================
//  SunMesh - Generates a simple circular disc to represent the sun.
// ============================================================
class SunMesh {
public:
    unsigned int VAO = 0;
    int vertexCount = 0;

    SunMesh(float radius = 5.0f, int segments = 32) {
        std::vector<float> vertices;
        // Center point
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);

        // Circular points
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * 3.14159f * (float)i / (float)segments;
            vertices.push_back(radius * cos(angle));
            vertices.push_back(radius * sin(angle));
            vertices.push_back(0.0f);
        }

        vertexCount = segments + 2;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    ~SunMesh() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void draw() const {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
        glBindVertexArray(0);
    }

private:
    unsigned int VBO = 0;
};
