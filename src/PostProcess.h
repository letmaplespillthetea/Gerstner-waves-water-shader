#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

// ============================================================
//  BloomMip - Represents a downsampled texture level
// ============================================================
struct BloomMip {
    glm::vec2 size;
    glm::ivec2 intSize;
    unsigned int texture;
};

// ============================================================
//  PostProcess - Manages Catlike Coding Bloom Pipeline
// ============================================================
class PostProcess {
public:
    unsigned int hdrFBO;
    unsigned int colorBuffer;
    unsigned int bloomFBO;
    std::vector<BloomMip> mipChain;
    unsigned int quadVAO, quadVBO;
    int m_width, m_height;

    PostProcess(int width, int height) : m_width(width), m_height(height) {
        // ---- HDR Framebuffer ----
        glGenFramebuffers(1, &hdrFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

        glGenTextures(1, &colorBuffer);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

        unsigned int rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ---- Bloom Progressive FBO ----
        glGenFramebuffers(1, &bloomFBO);
        
        glm::ivec2 mipSize(width / 2, height / 2);
        // Create mip chain (down to at least 2 pixels)
        for (int i = 0; i < 6; i++) {
            BloomMip mip;
            mip.intSize = mipSize;
            mip.size = glm::vec2((float)mipSize.x, (float)mipSize.y);
            
            glGenTextures(1, &mip.texture);
            glBindTexture(GL_TEXTURE_2D, mip.texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mipSize.x, mipSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            mipChain.push_back(mip);
            
            mipSize /= 2;
            if (mipSize.y < 2) break;
        }

        // ---- Full-screen Quad ----
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    void renderQuad() {
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void renderBloom(Shader& downsample, Shader& upsample, float threshold, float softThreshold) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO);
        
        // --- 1. Downsample (with Prefilter on mip 0) ---
        downsample.use();
        glDisable(GL_BLEND); // Ensure no blending during downsample

        float knee = threshold * softThreshold;
        glm::vec4 filter(threshold, threshold - knee, 2.0f * knee, 0.25f / (knee + 0.00001f));
        downsample.setVec4("u_filter", filter);

        unsigned int currentSource = colorBuffer;
        glm::vec2 currentSourceSize((float)m_width, (float)m_height);

        for (int i = 0; i < mipChain.size(); i++) {
            const BloomMip& mip = mipChain[i];
            glViewport(0, 0, mip.intSize.x, mip.intSize.y);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);
            
            downsample.setInt("u_mipLevel", i);
            downsample.setVec2("u_srcResolution", currentSourceSize);
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, currentSource);
            renderQuad();
            
            currentSource = mip.texture;
            currentSourceSize = mip.size;
        }

        // --- 2. Upsample (with Additive Blending) ---
        upsample.use();
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        for (int i = (int)mipChain.size() - 2; i >= 0; i--) {
            const BloomMip& mip = mipChain[i];
            const BloomMip& prevMip = mipChain[i+1];
            
            glViewport(0, 0, mip.intSize.x, mip.intSize.y);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);
            
            upsample.setVec2("u_srcResolution", prevMip.size);
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, prevMip.texture);
            renderQuad();
        }

        // Cleanup state
        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_width, m_height);
    }
};
