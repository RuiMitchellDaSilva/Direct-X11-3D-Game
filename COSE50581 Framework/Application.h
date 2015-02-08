#pragma once

//Input includes
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <dinput.h>

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"

#include "GameObject.h"
#include "Camera.h"
#include "DDSTextureLoader.h"
#include "OBJLoader.h"

#include "Structures.h"
#include "LightStructures.h"

using namespace DirectX;

const float kPI = 3.1415926539;
const int kCUBES = 10;

// Enum for the Rasterizer
enum Rasterizer { solid, wireFrame };

class Application
{
private:
	HINSTANCE					_hInst;
	HWND						_hWnd;
	D3D_DRIVER_TYPE				_driverType;
	D3D_FEATURE_LEVEL			_featureLevel;
	ID3D11Device*				_pd3dDevice;
	ID3D11DeviceContext*		_pImmediateContext;
	IDXGISwapChain*				_pSwapChain;
	ID3D11RasterizerState*		_wireFrame;
	ID3D11RasterizerState*		_solidRender;
	ID3D11RasterizerState*		_noCulling;
	ID3D11RenderTargetView*		_pRenderTargetView;
	ID3D11VertexShader*			_pVertexShader;
	ID3D11PixelShader*			_pPixelShader;
	ID3D11GeometryShader*		_pGeometryShader;
	ID3D11InputLayout*			_pVertexLayout;
	ID3D11Buffer*				_pVertexBufferCube;
	ID3D11Buffer*				_pVertexBufferPlane;  
	ID3D11Buffer*				_pIndexBufferCube;
	ID3D11Buffer*				_pIndexBufferPlane;
	ID3D11Buffer*				_pConstantBuffer;
	ID3D11Buffer*				_instanceBuffer;
	ID3D11DepthStencilView*		_depthStencilView;
	ID3D11Texture2D*			_depthStencilBuffer;
	XMFLOAT4X4					_world;
	cbPerObject					_cbPerObj;

	IDirectInputDevice8*		_dIMouse;
	DIMOUSESTATE				_mouseLastState;
	LPDIRECTINPUT8				_directInput;

	UINT						_WindowHeight;
	UINT						_WindowWidth;

	Rasterizer					_render = solid;

	Camera*						_firstPersonCamera;

	Camera*						_frontViewCamera;
	Camera*						_rearViewCamera;
	Camera*						_thirdPersonCamera;
	Camera*						_topDownCamera;

	float						_eyeX, _eyeY, _eyeZ;
	float						_carRotation = 0.0f;
	int							_currentCamera = 1;
	XMFLOAT4					_mLastMousePos;
						

	MeshData					_objMeshData;
	MeshData					_carMeshData;
	MeshData					_stoneMesh;
	MeshData					_sphereMesh;

	GameObject*					_plane;
	GameObject*					_tree[5];
	GameObject*					_barrier[5];
	GameObject*					_carOrange;
	GameObject*					_carBlue;
	GameObject*					_skybox;
	UINT						_stride[2];
	UINT						_offset[2];

	// Textures
	ID3D11ShaderResourceView*	_pCrateColouredTextureRV = nullptr;
	ID3D11ShaderResourceView*	_pWaterTextureRV = nullptr;
	ID3D11ShaderResourceView*	_pTrackTextureRV = nullptr;
	ID3D11ShaderResourceView*	_pPineTreeTextureRV = nullptr;
	ID3D11ShaderResourceView*	_pStoneTextureRV = nullptr;
	ID3D11ShaderResourceView*	_pOrangeCarTextureRV = nullptr;
	ID3D11ShaderResourceView*	_pBlueCarTextureRV = nullptr;

	ID3D11SamplerState*			_pSamplerLinear = nullptr;

	ID3D11BlendState*			_transparency;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitCamera();
	HRESULT InitDevice();
	HRESULT InitDirectInput(HINSTANCE hInstance);
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	//Input
	void DetectInput(double time);

	HRESULT InitVertexBufferCube();
	HRESULT InitIndexBufferCube();
	HRESULT InitVertexBufferPlane();
	HRESULT InitIndexBufferPlane();

	void InitInstanceBuffer();

	void RecalculateCameras();
	void DetectInput();
	void SaveTexture();
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);

	XMMATRIX SetupMatrixIdentities(XMFLOAT3 scale,
		XMFLOAT3 translate1, XMFLOAT3 translate2,
		float rotate);
public:
	// m = member variable
	// i = integer variable
	int	miCameraNo = 0;

	float plX = 0;
	float plY = 0; 
	float plZ = 0;


public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

