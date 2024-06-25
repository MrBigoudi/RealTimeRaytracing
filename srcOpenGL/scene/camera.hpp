#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>

#include <vector>

namespace glr{

// Defines several possible options for camera movement
// Used as abstraction to stay away from window-system specific input methods
enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

struct CameraGPU{
    glm::mat4 _View;
    glm::mat4 _Proj;
    glm::mat4 _InvView;
    glm::mat4 _InvProj;
    glm::vec4 _Eye;
    float _PlaneWidth;
    float _PlaneHeight;
    float _PlaneNear;
};

class Camera;
using CameraPtr = std::shared_ptr<Camera>;

class Camera{
    private:
        // camera Attributes
        glm::vec3 _Eye;
        glm::vec3 _At;
        glm::vec3 _WorldUp;

        glm::vec3 _Up;
        glm::vec3 _Right;

        float _Fov = 0.f;
        float _AspectRatio = 0.f;
        float _Near = 0.f;
        float _Far = 0.f;

        const float _MOVEMENT_ACCELERATION = 5.f;
        const float _MOVEMENT_SPEED = 20.f;
        const float _MOUSE_SENSITIVITY = 0.1f;

        float _Yaw = -90.f;
        float _Pitch = 0.f;
    
    public:
        bool _Accelerate = false;

    public:
        // constructor with vectors
        Camera(
            const glm::vec3& position,
            float aspectRatio,
            float fov = 45.f,
            float near = 0.1f,
            float far = 200.f,
            const glm::vec3& worldUp = glm::vec3(0.f, 1.f, 0.f)
        );

        CameraGPU getGpuData() const;

        glm::vec3 getAt() const;

        glm::vec3 getPosition() const;

        glm::mat4 getView() const;

        glm::mat4 getPerspective() const;

        float getPlaneHeight() const;

        float getPlaneHeight(float planeWidth) const;

        float getPlaneWidth() const;

        float getPlaneWidth(float planeHeight) const;

        void processKeyboard(CameraMovement direction, float deltaTime);

        void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    private:
        void updateCameraVectors();
};

}