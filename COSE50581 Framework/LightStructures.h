#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <directxmath.h>

using namespace DirectX;

struct DirectionalLight
{
	//DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Diffuse;
	XMFLOAT4 Ambient;
	XMFLOAT4 Specular;
	XMFLOAT3 Vector;
	float pad; //Pad the last float so an array of lights can be set.
};

struct PointLight
{
	//PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Diffuse;
	XMFLOAT4 Ambient;
	XMFLOAT4 Specular;

	// Position and Range are packed into a 4D vector.
	XMFLOAT3 position;
	float range;

	// Attentuation 1, 2 and 3, and Pad are packed into a 4D vector
	XMFLOAT3 att; // Stores 3 attenuation constants in the format (a0, a1, a2) that controls how light intensity
	// falls of with distance.
	float pad; // Pad the last float so an array of lights can be set.
};

struct SpotLight
{
	//SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Diffuse;
	XMFLOAT4 Ambient;
	XMFLOAT4 Specular;

	// Position and Range are packed into a 4D vector.
	XMFLOAT3 position;
	float range;

	// Direction and Spot are packed into a 4D vector.
	XMFLOAT3 direction;
	float spot;

	// Attentuation 1, 2 and 3, and Pad are packed into a 4D vector.
	XMFLOAT3 att;
	float pad; // Pad the last float so an array of lights can be set.
};
