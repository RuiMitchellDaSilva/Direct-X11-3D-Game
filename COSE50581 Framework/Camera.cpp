#include "Camera.h"

Camera::Camera(XMFLOAT4 eye, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
	: _eye(eye), _at(at), _up(up), _windowWidth(windowWidth), _windowHeight(windowHeight), _nearDepth(nearDepth), _farDepth(farDepth)
{
	_yaw = 0.0f;
	_pitch = 0.0f;

	// Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight,
		_nearDepth, _farDepth));
}

Camera::~Camera()
{
}

void Camera::CalculateViewProjection()
{
	// Camera Parameters
	_at.x = cos(_yaw) * cos(_pitch);
	_at.y = sin(_pitch);
	_at.z = sin(_yaw) * cos(_pitch);

	_strafeLookX = cos(_yaw - XM_PIDIV2);
	_strafeLookZ = sin(_yaw - XM_PIDIV2);

    // Initialize the view matrix
	XMVECTOR Eye = XMLoadFloat4(&_eye);
	XMVECTOR At = XMLoadFloat4(&_at); 
	XMVECTOR Up = XMLoadFloat4(&_up);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, Eye + At, Up));

	// Recalculate projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, _nearDepth, _farDepth));
}

void Camera::Move(float increment)
{
	float lookX = cos(_yaw) * cos(_pitch);
	float lookY = sin(_pitch);
	float lookZ = sin(_yaw) * cos(_pitch);

	if (!_topDown)
	{
		_eye.x = _eye.x + (lookX * increment);
		_eye.y = _eye.y + (lookY * increment);
		_eye.z = _eye.z + (lookZ * increment);
	}
	else
	{
		_eye.x = _eye.x + (cos(_yaw) * increment);
		_eye.z = _eye.z + (sin(_yaw) * increment);
	}

	CalculateViewProjection();
}

void Camera::Strafe(float increment)
{
	_eye.x = _eye.x + (_strafeLookX * increment);

	_eye.z = _eye.z + (_strafeLookZ * increment);

	CalculateViewProjection();
}

void Camera::Fly(float increment)
{
	if (!_topDown)
		_eye.y = _eye.y + increment;

	CalculateViewProjection();
}

void Camera::RotatePitch(float angle)
{
	const float limit = 89.0 * XM_PI / 180.0;

	_pitch += angle;

	if (_pitch < -limit)
		_pitch = -limit;

	if (_pitch > limit)
		_pitch = limit;

	CalculateViewProjection();
}

void Camera::RotateYaw(float angle)
{
	_yaw += angle;

	CalculateViewProjection();
}

void Camera::SetRotation(float x, float y, float z)
{
	XMStoreFloat4x4(&_rotate, XMMatrixRotationX(x) * XMMatrixRotationX(y) * XMMatrixRotationX(z));
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
}

XMFLOAT4X4 Camera::GetViewProjection() const 
{ 
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);

	XMFLOAT4X4 viewProj;

	XMStoreFloat4x4(&viewProj, view * projection);

	return viewProj;
}