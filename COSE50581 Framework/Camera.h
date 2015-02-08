#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <directxmath.h>

using namespace DirectX;

class Camera
{
private:
	XMFLOAT4 _eye, _at, _up;

	FLOAT _pitch, _yaw;

	FLOAT _strafeLookX, _strafeLookZ;

	bool _topDown = false;


	// For reshaping the camera when the program window reshapes
	FLOAT _windowWidth, _windowHeight, _nearDepth, _farDepth;

	XMFLOAT4X4 _rotate;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

public:
	Camera(XMFLOAT4 eye, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	~Camera();

	void CalculateViewProjection();

	void RotatePitch(float angle);
	void RotateYaw(float angle);

	XMFLOAT4X4 GetView() const { return _view; }
	XMFLOAT4X4 GetProjection() const { return _projection; }

	XMFLOAT4X4 GetViewProjection() const;

	XMFLOAT4 GetEye() { return _eye; }
	XMFLOAT4 GetAt() { return _at; }
	XMFLOAT4 GetUp() { return _up; }

	float GetPitch() { return _pitch; }
	float GetYaw() { return _yaw; }

	void Move(float increment);
	void Strafe(float increment);
	void Fly(float increment);

	void SetRotation(float x, float y, float z);

	void SetEye(XMFLOAT4 eye) { _eye = eye; }
	void SetAt(XMFLOAT4 at) { _at = at; }
	void SetUp(XMFLOAT4 up) { _up = up; }

	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);

	void SetTopDown(bool topDown) { _topDown = topDown; }
};