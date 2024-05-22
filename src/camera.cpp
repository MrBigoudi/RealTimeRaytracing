#include "camera.hpp"
#include <glm/ext.hpp>

Camera::Camera(
    const glm::vec3& position,
    float aspectRatio,
    float fov,
    float near,
    float far,
    const glm::vec3& worldUp
){
    _AspectRatio = aspectRatio;
    _Fov = fov;
    _Near = near;
    _Far = far;
    _WorldUp = worldUp;
    _Eye = position;
    updateCameraVectors();
}

CameraGPU Camera::getGpuData() const {
    CameraGPU camera{};
    camera._View = getView();
    camera._Proj = getPerspective();
    camera._Eye = glm::vec4(getPosition(), 1.f);
    camera._PlaneHeight = getPlaneHeight();
    camera._PlaneWidth = getPlaneWidth(camera._PlaneHeight);
    return camera;
}

glm::vec3 Camera::getAt() const {
    return _At;
}

glm::vec3 Camera::getPosition() const {
    return _Eye;
}

glm::mat4 Camera::getView() const {
    return glm::lookAt(_Eye, _Eye + _At, _Up);
}

glm::mat4 Camera::getPerspective() const {
    return glm::perspective(glm::radians(_Fov), _AspectRatio, _Near, _Far);
}

float Camera::getPlaneHeight() const {
    return 2.f * _Near * glm::tan(0.5f * glm::radians(_Fov));
}

float Camera::getPlaneHeight(float planeWidth) const {
    return planeWidth / _AspectRatio;
}

float Camera::getPlaneWidth() const {
    return 2.f * _Near * _AspectRatio * glm::tan(0.5f * glm::radians(_Fov));
}

float Camera::getPlaneWidth(float planeHeight) const {
    return planeHeight * _AspectRatio;
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime){
    float velocity = _MovementSpeed * deltaTime;
    if(_Accelerate) velocity *= 5.f;

    switch(direction){
        case FORWARD:
            _Eye += _At * velocity;
            break;
        case BACKWARD:
            _Eye -= _At * velocity;
            break;
        case LEFT:
            _Eye -= _Right * velocity;
            break;
        case RIGHT:
            _Eye += _Right * velocity;
            break;
        case UP:
            _Eye += _WorldUp * velocity;
            break;
        case DOWN:
            _Eye -= _WorldUp * velocity;
            break;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch){
    xoffset *= _MouseSensitivity;
    yoffset *= _MouseSensitivity;
    _Yaw   += xoffset;
    _Pitch += yoffset;
    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch){
        if (_Pitch > 89.0f)
            _Pitch = 89.0f;
        if (_Pitch < -89.0f)
            _Pitch = -89.0f;
    }
    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

void Camera::updateCameraVectors(){
    // calculate the new at vector
    glm::vec3 front;
    front.x = cos(glm::radians(_Yaw)) * cos(glm::radians(_Pitch));
    front.y = sin(glm::radians(_Pitch));
    front.z = sin(glm::radians(_Yaw)) * cos(glm::radians(_Pitch));
    _At = glm::normalize(front);
    // also re-calculate the Right and Up vector
    _Right = glm::normalize(glm::cross(_At, _WorldUp));
    _Up    = glm::normalize(glm::cross(_Right, _At));
}