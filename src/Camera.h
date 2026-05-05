#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================================================
//  Camera - FPS-style free camera
//  Supports WASD movement + mouse look
// ============================================================
class Camera {
public:
    // Camera position and orientation
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler angles (degrees)
    float yaw;    // Horizontal rotation
    float pitch;  // Vertical rotation

    // Movement settings
    float movementSpeed;
    float mouseSensitivity;
    float fov; // Field of view (degrees)

    // Constructor with default position looking over the water
    Camera(glm::vec3 startPos = glm::vec3(0.0f, 5.0f, 20.0f),
           glm::vec3 worldUpVec = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw   = -90.0f,
           float startPitch = -10.0f)
        : position(startPos),
          worldUp(worldUpVec),
          yaw(startYaw),
          pitch(startPitch),
          movementSpeed(10.0f),
          mouseSensitivity(0.1f),
          fov(45.0f)
    {
        updateCameraVectors();
    }

    // Returns the view matrix (world -> camera space)
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

    // Keyboard movement - called every frame
    // Direction: 0=forward, 1=backward, 2=left, 3=right, 4=up, 5=down
    void processKeyboard(int direction, float deltaTime) {
        float velocity = movementSpeed * deltaTime;
        if (direction == 0) position += front  * velocity; // W
        if (direction == 1) position -= front  * velocity; // S
        if (direction == 2) position -= right  * velocity; // A
        if (direction == 3) position += right  * velocity; // D
        if (direction == 4) position += worldUp * velocity; // Space
        if (direction == 5) position -= worldUp * velocity; // Ctrl
    }

    // Mouse look - called on mouse move event
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true) {
        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;

        yaw   += xOffset;
        pitch += yOffset;

        // Prevent camera from flipping over
        if (constrainPitch) {
            if (pitch >  89.0f) pitch =  89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }

        updateCameraVectors();
    }

    // Mouse scroll to zoom
    void processMouseScroll(float yOffset) {
        fov -= yOffset;
        if (fov < 10.0f)  fov = 10.0f;
        if (fov > 90.0f)  fov = 90.0f;
    }

private:
    // Recalculate front/right/up from yaw and pitch
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(newFront);
        right = glm::normalize(glm::cross(front, worldUp));
        up    = glm::normalize(glm::cross(right, front));
    }
};
