#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Data
{
	enum CameraMovements
	{
		forward,
		backward,
		left,
		right,
		max_type
	};

	constexpr float YAW{ -90.0f };
	constexpr float PITCH{ 0.0f };
	constexpr float SPEED{ 2.5f };
	constexpr float SENSI_MOUSE{ 0.1f };
	constexpr float ZOOM{ 45.0f };
}

class Camera
{
public:
	explicit Camera(const glm::vec3& position = glm::vec3(0.0f,0.0f,0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = Data::YAW, float pitch = Data::PITCH);
	explicit Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw = Data::YAW, float pitch = Data::PITCH);
	glm::mat4 getViewMatrix() const;
	glm::mat4 getReverseViewMatrix() const;
	float getFov() const;
	const glm::vec3& getPosition() const;
	const glm::vec3& getFront() const;
	void processKeyboard(Data::CameraMovements movement, float deltaTime);
	void processMouseMove(float xOffset, float yOffset, GLboolean constrainPitch = true);
	void processMouseScroll(float yOffset);

private:
	glm::vec3 m_position{};
	glm::vec3 m_front{};
	glm::vec3 m_up{}; // camera's up
	glm::vec3 m_right{};
	glm::vec3 m_worldUp{}; // global up
	float m_yaw{};
	float m_pitch{};
	float m_movementSpeed{Data::SPEED};
	float m_mouseSensitivity{Data::SENSI_MOUSE};
	float m_zoom{Data::ZOOM};

	void updateCameraParams();
	glm::mat4 getLookAt(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f), const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)) const;

};