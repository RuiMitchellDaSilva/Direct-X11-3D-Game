#pragma once

#include <directxmath.h>

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};
};

struct InstanceData
{
	XMFLOAT4X4 World;
	XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;

	XMFLOAT4 ambientMtrl;
	XMFLOAT4 ambientLight;

	XMFLOAT4 specularMtrl;
	XMFLOAT4 specularLight;
	float	 specularPower;

	XMFLOAT3 eyePosW;
	XMFLOAT3 lightVecW;
	XMFLOAT3 gLightPos;
	float	 gLightRange;
	XMFLOAT3 gLightAtten;
	float	 pad;

	float spotCone;

	float imageWidth;
	float imageHeight;
	// Pads
	float pad1;
	float pad2;
	float camPos;
};

struct cbPerObject
{
	XMMATRIX WVP;
};

struct Pair
{
	float x;
	float z;
};

//For encapsulating 3 float variables
struct Triplet
{
	float x;
	float y;
	float z;
};
