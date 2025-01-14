#include "Camera.hpp"

namespace gps {

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, this->cameraFrontDirection));
        this->cameraUpDirection = glm::cross(cameraFrontDirection, cameraRightDirection);
        rotate(pitch, yaw);
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    void Camera::move(MOVE_DIRECTION direction, float deltaTime) {
        const float speed = cameraSpeed * deltaTime;
        if (direction == MOVE_FORWARD)
            cameraPosition += speed * cameraFrontDirection;
        if (direction == MOVE_BACKWARD)
            cameraPosition -= speed * cameraFrontDirection;
        if (direction == MOVE_LEFT)
            cameraPosition -= glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
        if (direction == MOVE_RIGHT)
            cameraPosition += glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
    }

    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 direction(0.0f, 0.0f, 0.0f);
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(direction);
    }

    void Camera::mouseCallback(GLFWwindow* window, double xPos, double yPos)
    {
        if (firstMouse) {
            lastX = static_cast<float>(xPos);
            lastY = static_cast<float>(yPos);
            firstMouse = false;
        }
        float xOffset = static_cast<float>(xPos) - lastX;
        float yOffset = lastY - static_cast<float>(yPos);
        lastX = static_cast<float>(xPos);
        lastY = static_cast<float>(yPos);
        xOffset *= sensivity;
        yOffset *= sensivity;
        yaw += xOffset;
        pitch += yOffset;
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
        rotate(pitch, yaw);
    }

    void Camera::keyboardCallback(float deltaTime, GLFWwindow* window, int key, int scancode, int action, int mode)
    {
        if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
            move(gps::MOVE_FORWARD, deltaTime);
        if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
            move(gps::MOVE_BACKWARD, deltaTime);
        if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
            move(gps::MOVE_LEFT, deltaTime);
        if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
            move(gps::MOVE_RIGHT, deltaTime);
    }

    void Camera::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
    {
        const float zoomSensitivity = 0.01f;
        zoom -= static_cast<float>(yOffset) * zoomSensitivity;
        if (zoom < 1.0f)
            zoom = 1.0f;
        if (zoom > 70.0f)
            zoom = 70.0f;
    }

    glm::mat4 Camera::getProjectionMatrix(float aspectRatio)
    {
        return glm::perspective(zoom, aspectRatio, nearPlane, farPlane);
    }
}