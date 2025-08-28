#include <learnGL/camera.h>

 Camera::Camera(const glm::vec3& position, const glm::vec3& up, float yaw, float pitch)
	: m_position{ position }, m_worldUp{ up }, m_front{glm::vec3(0.0f,0.0f,-1.0f)},
		m_yaw{ yaw }, m_pitch{ pitch }
{
	updateCameraParams();
}
 
 Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
	:	m_position{ glm::vec3(posX,posY,posZ) }, m_worldUp{ glm::vec3(upX,upY,upZ) }, m_front{ glm::vec3(0.0f,0.0f,-1.0f) },
		m_yaw{ yaw }, m_pitch{ pitch }
{
	updateCameraParams();
}

glm::mat4 Camera::getViewMatrix() const
{
	//return glm::lookAt(m_position, m_position + m_front, m_up);
	return getLookAt(m_position, m_position + m_front, m_up);
	// remember the middle parameter is pos + front(real heading)!
}

glm::mat4 Camera::getReverseViewMatrix() const
{
	// to get a rear view, we just simply reverse the front
	return getLookAt(m_position, m_position - m_front, m_up);
}

float Camera::getFov() const
{
	return m_zoom; // degrees here, radians is needed
}

const glm::vec3& Camera::getPosition() const
{
	return m_position;
}

const glm::vec3& Camera::getFront() const
{
	return m_front;
}

void Camera::processKeyboard(Data::CameraMovements movement, float deltaTime)
{
	float velocity{ m_movementSpeed * deltaTime };
	switch (movement)
	{
	case Data::forward:
		m_position += m_front * velocity;
		break;
	case Data::backward:
		m_position -= m_front * velocity;
		break;
	case Data::left:
		m_position -= m_right * velocity;
		break;
	case Data::right:
		m_position += m_right * velocity;
		break;
	default:
		break;
	}
	//m_position.y = 0; // FPS viewer can not fly, so component y should always be 0
}

void Camera::processMouseMove(float xOffset, float yOffset, GLboolean constrainPitch)
{
	xOffset *= m_mouseSensitivity;
	yOffset *= m_mouseSensitivity;

	m_yaw += xOffset;
	m_pitch -= yOffset; // reverse is needed

	if (constrainPitch)
	{
		if (m_pitch > 89.0f) // dir can't be parallel with the World UP
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;
	}

	updateCameraParams();
}

void Camera::processMouseScroll(float yOffset)
{
	m_zoom -= yOffset; // fov
	if (m_zoom < 1.0f)
		m_zoom = 1.0f;
	if (m_zoom > 45.0f)
		m_zoom = 45.0f;
}

void Camera::updateCameraParams()
{
	// create a movement vector and excute
	auto direction{ glm::vec3(1.0f) }; // unit vector
	direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	direction.y = sin(glm::radians(m_pitch));
	direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	direction = glm::normalize(direction); // mouse take control
	m_front = direction; // get the new front based on the latest yaw and pitch
	m_right = glm::normalize(glm::cross(m_front, m_worldUp)); // get the right and the up of the camera from that
	m_up = glm::normalize(glm::cross(m_right, m_front));
}

glm::mat4 Camera::getLookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) const
{
	// reverse the front (to +Z)
	glm::vec3 direction{ glm::normalize(position - target) }; // (pos-targ) -> (pos-(pos+front)) == (-front) == +Z

	glm::vec3 temp_up{ up }; // we set up here (world's absolute up, Not this obj's up) (this will influence the object's UP)
	glm::vec3 right{ glm::normalize(glm::cross(temp_up,direction)) };
	glm::vec3 real_up{ glm::normalize(glm::cross(direction,right)) };
	// CAUTION: mat4 is column major, we need to transpose the matrix
	glm::mat4 resultLeft{glm::mat4(1.0f)};
	resultLeft[0] = glm::vec4(right, 0);
	resultLeft[1] = glm::vec4(real_up, 0);
	resultLeft[2] = glm::vec4(direction, 0);
	resultLeft = glm::transpose(resultLeft); // transpose, To column Major
	glm::mat4 resultRight{ glm::mat4(1.0f) };
	resultRight[3][0] = -position.x; // Notice: col Major here!, you need index this way -> [col][row]
	resultRight[3][1] = -position.y;
	resultRight[3][2] = -position.z;
	return resultLeft * resultRight;
}