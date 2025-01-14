#define GLM_ENABLE_EXPERIMENTAL
#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT };

    class Camera {

    public:
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        glm::mat4 getViewMatrix();
        void move(MOVE_DIRECTION direction, float deltaTime);
        void rotate(float pitch, float yaw);
        void mouseCallback(GLFWwindow* window, double xPos, double yPos);
        void keyboardCallback(float deltaTime, GLFWwindow* window, int key, int scancode, int action, int mode);
        void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
        glm::mat4 getProjectionMatrix(float aspectRatio);

    private:
        bool firstMouse{ true };
        float zoom{ 70.0f };
        float lastX{ 400.0f };
        float lastY{ 300.0f };
        float pitch{ 0.0f };
        float yaw{ -90.0f };
        const float sensivity{ 0.1f };
        const float cameraSpeed{ 10.0f };
        const float nearPlane{ 0.1f };
        const float farPlane{ 1000.0f };
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
    };
}

#endif