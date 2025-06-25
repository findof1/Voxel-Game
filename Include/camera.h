#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 20.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 70.0f;

class Camera
{
public:
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;

  float Yaw;
  float Pitch;

  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
  {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
  }

  void move(const glm::vec3 &offset)
  {
    Position += offset;
  }

  void moveForwards(float amount)
  {
    Position += Front * amount;
  }

  void moveRight(float amount)
  {
    Position += Right * amount;
  }

  void rotate(float yawOffset, float pitchOffset, bool constrainPitch = true)
  {
    Yaw += yawOffset;
    Pitch += pitchOffset;

    if (constrainPitch)
    {
      if (Pitch > 89.0f)
        Pitch = 89.0f;
      if (Pitch < -89.0f)
        Pitch = -89.0f;
    }

    updateCameraVectors();
  }

  void setZoom(float zoom)
  {
    Zoom = glm::clamp(zoom, 1.0f, 120.0f);
  }

  glm::mat4 getViewMatrix() { return glm::lookAt(Position, Position + Front, Up); }
  glm::vec3 getForward() const { return Front; }
  glm::vec3 getRight() const { return Right; }
  glm::vec3 getUp() const { return Up; }

private:
  void updateCameraVectors()
  {

    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
  }
};
#endif