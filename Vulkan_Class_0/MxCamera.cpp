#include "MxCamera.h"

namespace Mix
{
	//void MxVkCamera::updateCamera()
	//{
	//	// update front vector
	//	glm::vec3 front;
	//	front.x = std::cos(glm::radians(mEulerAngle.pitch))*std::cos(glm::radians(mEulerAngle.yaw));
	//	front.y = std::cos(glm::radians(mEulerAngle.pitch))*std::sin(glm::radians(mEulerAngle.yaw));
	//	front.z = std::sin(glm::radians(mEulerAngle.yaw));
	//	mFront = glm::normalize(front);

	//	// update right vector
	//	if (std::abs(glm::dot(mFront, mWorldUp)) > (1 - 0.001))
	//		mRight = glm::cross(mFront, mWorldUp);
	//		// update up vector
	//	mUp = glm::cross(mRight, mFront);
	//}

	//MxVkCamera::MxVkCamera() :
	//	mPosition(0.0f, 0.0f, 0.0f),
	//	mFront(1.0f, 0.0f, 0.0f),
	//	mRight(0.0f, -1.0f, 0.0f),
	//	mUp(0.0f, 0.0f, 1.0f),
	//	mWorldUp(0.0f, 0.0f, 1.0f),
	//	mEulerAngle(0.0f, 0.0f, 0.0f),
	//	mMoveSpeed(1.0f),
	//	mMouseSensitivity(1.0f)
	//{
	//	updateCamera();
	//}

	//MxVkCamera::MxVkCamera(const glm::vec3 pos,
	//					   const MxEulerAngle<float>& angle,
	//					   const glm::vec3 worldUp) :MxVkCamera()
	//{
	//	mPosition = pos;
	//	mEulerAngle = angle;
	//	mWorldUp = worldUp;
	//}

	//glm::mat4 MxVkCamera::getViewMatrix() const
	//{
	//	return glm::lookAt(mPosition, mPosition + mFront, mWorldUp);
	//}

	//void MxVkCamera::moveCamera(const float deltaTime, glm::vec3 direction, bool normalized)
	//{
	//	if (!normalized)
	//		direction = glm::normalize(direction);

	//	mPosition += (direction * deltaTime * mMoveSpeed);
	//	updateCamera();
	//}

	//void MxVkCamera::moveCamera(const float deltaTime, CameraDirection direction)
	//{
	//	glm::vec3 dirVec;
	//	switch (direction)
	//	{
	//	case Mix::MxVkCamera::FORWARD:
	//		dirVec = mFront;
	//		break;
	//	case Mix::MxVkCamera::BACK:
	//		dirVec = -mFront;
	//		break;
	//	case Mix::MxVkCamera::LEFT:
	//		dirVec = -mRight;
	//		break;
	//	case Mix::MxVkCamera::RIGHT:
	//		dirVec = mRight;
	//		break;
	//	case Mix::MxVkCamera::UP:
	//		dirVec = mUp;
	//		break;
	//	case Mix::MxVkCamera::DOWN:
	//		dirVec = -mUp;
	//		break;
	//	case Mix::MxVkCamera::WORLD_UP:
	//		dirVec = mWorldUp;
	//		break;
	//	case Mix::MxVkCamera::WORLD_DOWN:
	//		break;
	//		dirVec = -mWorldUp;
	//	default:
	//		break;
	//	}

	//	moveCamera(deltaTime, dirVec);
	//}

	//void MxVkCamera::rotateCamera(float yawOffset, float pitchOffset, float rollOffset, bool constrainPitch)
	//{
	//	mEulerAngle.yaw += (yawOffset * mMouseSensitivity);
	//	mEulerAngle.pitch += (pitchOffset * mMouseSensitivity);

	//	if (constrainPitch)
	//	{
	//		mEulerAngle.pitch = mEulerAngle.pitch >= 89.0f ? 89.0f : mEulerAngle.pitch;
	//		mEulerAngle.pitch = mEulerAngle.pitch <= -89.0f ? -89.0f : mEulerAngle.pitch;
	//	}

	//	updateCamera();
	//}

	//glm::vec3 MxVkCamera::setPosition(glm::vec3 pos)
	//{
	//	glm::vec3 old = mPosition;
	//	mPosition = pos;
	//	return old;
	//}

	//MxEulerAngle<float> MxVkCamera::setEulerAngle(MxEulerAngle<float>& eulerAngle)
	//{
	//	MxEulerAngle old = mEulerAngle;
	//	mEulerAngle = eulerAngle;
	//	return old;
	//}

	//float MxVkCamera::setMoveSpeed(float speed)
	//{
	//	float old = mMoveSpeed;
	//	if (speed >= 0.0f)
	//	{
	//		mMoveSpeed = speed;
	//	}
	//	return old;
	//}

	//float MxVkCamera::setMouseSensitivity(float sensitivity)
	//{
	//	float old = mMouseSensitivity;
	//	mMouseSensitivity = sensitivity;
	//	return old;
	//}

	MxCamera::MxCamera() :mTransform(), mMoveSpeed(1.0f), mMouseSensitivity(1.0f)
	{
	}

	glm::mat4 MxCamera::getViewMatrix() const
	{
		return glm::lookAt(mTransform.getPosition(),
						   mTransform.getPosition() + mTransform.getForward(),
						   mTransform.getUp());
	}

	void MxCamera::moveCamera(const float deltaTime, glm::vec3 direction, bool normalized)
	{
		if (!normalized)
			direction = glm::normalize(direction);

		mTransform.setPosition(mTransform.getPosition() + direction * deltaTime * mMoveSpeed);
	}

	void MxCamera::moveCamera(const float deltaTime, const CameraDirection direction)
	{
		glm::vec3 dir;

		switch (direction)
		{
		case Mix::MxCamera::FORWARD:
			dir = mTransform.getForward();
			break;
		case Mix::MxCamera::BACK:
			dir = -mTransform.getForward();
			break;
		case Mix::MxCamera::LEFT:
			dir = -mTransform.getRight();
			break;
		case Mix::MxCamera::RIGHT:
			dir = mTransform.getRight();
			break;
		case Mix::MxCamera::UP:
			dir = mTransform.getUp();
			break;
		case Mix::MxCamera::DOWN:
			dir = -mTransform.getUp();
			break;
		case Mix::MxCamera::WORLD_UP:
			dir = MxAxis::initZ;
			break;
		case Mix::MxCamera::WORLD_DOWN:
			dir = -MxAxis::initZ;
			break;
		}

		moveCamera(deltaTime, dir);
	}

	void MxCamera::rotateCamera(const float yawOffset, const float pitchOffset, const float rollOffset)
	{
		mTransform.rotate(rollOffset, pitchOffset, yawOffset);
	}

	void MxCamera::setPosition(const glm::vec3& pos)
	{
		mTransform.setPosition(pos);
	}

	void MxCamera::setMoveSpeed(const float speed)
	{
		if (speed < 0.0f)
			return;
		mMoveSpeed = speed;
	}

	void MxCamera::lookAt(const glm::vec3& target)
	{
		mTransform.lookAt(target);
	}

	void MxCamera::lookAt(const float x, const float y, const float z)
	{
		lookAt(glm::vec3(x, y, z));
	}

	void MxCamera::setMouseSensitivity(const float sensitivity)
	{
		if (sensitivity < 0.0f)
			return;
		mMouseSensitivity = sensitivity;
	}

}
