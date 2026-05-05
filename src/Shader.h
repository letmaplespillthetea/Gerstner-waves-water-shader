#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// ============================================================
//  Shader - Compiles and links vertex + fragment shader
//  Provides typed uniform setters for clean usage at call site
// ============================================================
class Shader {
public:
    unsigned int id; // OpenGL program handle

    // Build shader program from file paths
    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vertCode   = readFile(vertexPath);
        std::string fragCode   = readFile(fragmentPath);
        const char* vShaderSrc = vertCode.c_str();
        const char* fShaderSrc = fragCode.c_str();

        // Compile vertex shader
        unsigned int vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &vShaderSrc, nullptr);
        glCompileShader(vertShader);
        checkCompileErrors(vertShader, "VERTEX");

        // Compile fragment shader
        unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &fShaderSrc, nullptr);
        glCompileShader(fragShader);
        checkCompileErrors(fragShader, "FRAGMENT");

        // Link to program
        id = glCreateProgram();
        glAttachShader(id, vertShader);
        glAttachShader(id, fragShader);
        glLinkProgram(id);
        checkCompileErrors(id, "PROGRAM");

        // Shaders are now part of the program; individual objects can be freed
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
    }

    ~Shader() { glDeleteProgram(id); }

    void use() const { glUseProgram(id); }

    // ---------- Uniform setters ----------
    void setBool (const std::string& name, bool value)  const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
    }
    void setInt  (const std::string& name, int value)   const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }
    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(id, name.c_str()), value);
    }
    void setVec2 (const std::string& name, const glm::vec2& v) const {
        glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(v));
    }
    void setVec3 (const std::string& name, const glm::vec3& v) const {
        glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(v));
    }
    void setVec4 (const std::string& name, const glm::vec4& v) const {
        glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(v));
    }
    void setMat4 (const std::string& name, const glm::mat4& m) const {
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
    }

private:
    // Read entire text file into a string
    static std::string readFile(const char* path) {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            file.open(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        } catch (std::ifstream::failure& e) {
            std::cerr << "[Shader] ERROR: Cannot read file: " << path << "\n";
            return "";
        }
    }

    // Print compile/link errors with context
    static void checkCompileErrors(unsigned int object, const std::string& type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(object, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(object, 1024, nullptr, infoLog);
                std::cerr << "[Shader] COMPILE ERROR (" << type << "):\n" << infoLog << "\n";
            }
        } else {
            glGetProgramiv(object, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(object, 1024, nullptr, infoLog);
                std::cerr << "[Shader] LINK ERROR:\n" << infoLog << "\n";
            }
        }
    }
};
