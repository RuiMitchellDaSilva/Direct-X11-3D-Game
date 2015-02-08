#include "GameObject.h"

GameObject::GameObject(MeshData meshData) : _meshData(meshData)
{
}

GameObject::~GameObject(void)
{
}

void GameObject::Initialise()
{
	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	XMStoreFloat4x4(&_translate, XMMatrixIdentity());
	XMStoreFloat4x4(&_scale, XMMatrixIdentity());
	XMStoreFloat4x4(&_rotate, XMMatrixIdentity());
}

void GameObject::SetScale(float x, float y, float z)
{
	XMStoreFloat4x4(&_scale, XMMatrixScaling(x, y, z));
}

void GameObject::SetRotation(float x, float y, float z)
{
	XMStoreFloat4x4(&_rotate, XMMatrixRotationX(x) * XMMatrixRotationY(y) * XMMatrixRotationZ(z));
}

void GameObject::SetTranslation(float x, float y, float z)
{
	XMStoreFloat4x4(&_translate, XMMatrixTranslation(x, y, z));
}

void GameObject::UpdateWorld()
{
	XMMATRIX rotate = XMLoadFloat4x4(&_rotate);
	XMMATRIX translate = XMLoadFloat4x4(&_translate);
	XMMATRIX scale = XMLoadFloat4x4(&_scale);		

	XMStoreFloat4x4(&_world, rotate * translate * scale);
}

void GameObject::Update(float elapsedTime, XMFLOAT3 scale, XMFLOAT3 rotation, XMFLOAT3 translation)
{
	SetScale(scale.x, scale.y, scale.z);
	position = translation;
	SetTranslation(translation.x, translation.y, translation.z);
	SetRotation(rotation.x, rotation.y, rotation.z);
}

void GameObject::Draw(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pImmediateContext)
{
	// NOTE: We are assuming that the constant buffers and all other draw setup has already taken place

	// Set vertex and index buffers
	pImmediateContext->IASetVertexBuffers(0, 1, &_meshData.VertexBuffer, _meshData.VBStride, _meshData.VBOffset);
	pImmediateContext->IASetIndexBuffer(_meshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	if (GetInstanceCount() < 2)
		pImmediateContext->DrawIndexedInstanced(_meshData.IndexCount, GetInstanceCount(), 0, 0, 0);
	else
		pImmediateContext->DrawIndexed(_meshData.IndexCount, 0, 0);
}