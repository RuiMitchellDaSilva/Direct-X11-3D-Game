#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <directxmath.h>

using namespace DirectX;

struct MeshData
{
	ID3D11Buffer * VertexBuffer;
	ID3D11Buffer * IndexBuffer;
	UINT VBStride[2];
	UINT VBOffset[2];
	UINT IndexCount;
};

class GameObject
{
private:
	MeshData		_meshData;
	XMFLOAT3		_origin;

	XMFLOAT4X4		_world;

	XMFLOAT4X4		_scale;
	XMFLOAT4X4		_rotate;
	XMFLOAT4X4		_translate;

	ID3D11Buffer*	_instanceBuffer;
	int				_instanceCount = 100;

public:
	GameObject(MeshData meshData);
	~GameObject();

	XMFLOAT3  position = { 0.0f, 0.0f, 0.0f };

	XMFLOAT4X4 GetWorld() const { return _world; };
	XMFLOAT4X4 GetTranslation() const { return _translate; };
	int GetInstanceCount() const { return _instanceCount; }
	void UpdateWorld();

	void SetScale(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetTranslation(float x, float y, float z);

	void ControlTranslation(float x, float y, float z);

	void Initialise();
	void Update(float elapsedTime, XMFLOAT3 scale, XMFLOAT3 rotation, XMFLOAT3 translation);
	void Draw(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pImmediateContext);
};

