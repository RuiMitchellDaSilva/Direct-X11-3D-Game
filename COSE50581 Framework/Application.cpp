#include "Application.h"

float angler = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBufferCube = nullptr;
	_pVertexBufferPlane = nullptr;
	_pIndexBufferCube = nullptr;
	_pIndexBufferPlane = nullptr;
	_pConstantBuffer = nullptr;
	_stride[0] = sizeof(SimpleVertex); _stride[1] = sizeof(InstanceData);
	_offset[0] = 0; _offset[1] = 0;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
	if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
		return E_FAIL;
	}

	RECT rc;
	GetClientRect(_hWnd, &rc);

	_WindowWidth = rc.right - rc.left;
	_WindowHeight = rc.bottom - rc.top;

	if (FAILED(InitDevice()))
	{
		Cleanup();

		return E_FAIL;
	}

	// Initialise Input
	if (FAILED(InitDirectInput(hInstance)))
	{
		MessageBox(0, L"Direct Initialisation - Failed",
			L"Error", MB_OK);
		return 0;
	}

	// Initialise the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

	if (FAILED(InitCamera()))
	{
		MessageBox(0, L"Camera Initialization - Failed",
			L"Error", MB_OK);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT Application::InitDirectInput(HINSTANCE hInstance)
{
	HRESULT hr;

	hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&_directInput,
		NULL);

	hr = _directInput->CreateDevice(GUID_SysMouse,
		&_dIMouse,
		NULL);

	hr = _dIMouse->SetDataFormat(&c_dfDIMouse);

	hr = _dIMouse->SetCooperativeLevel(_hWnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
		
	return S_OK;
}

HRESULT Application::InitCamera()
{
	_eyeX = -150.0f;
	_eyeY = 60.0f;
	_eyeZ = -150.0f;

	// Initialise the view matrix

	// Eye for the first person camera
	XMFLOAT4 firstPersonEye = { _eyeX, _eyeY, _eyeZ, 1.0f };

	// front of orange car
	XMFLOAT4 frontViewEye = { _eyeX, _eyeY, _eyeZ, 1.0f };

	// Eye for the top-down camera
	XMFLOAT4 topDownEye = { _eyeX, 900.0f, _eyeZ, 1.0f };
	XMFLOAT4 At = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 Up = { 0.0f, 1.0f, 0.0f, 0.0f };

	// Third Person

	// Initialise cameras

	// Default camera initialised
	_firstPersonCamera = new Camera(firstPersonEye, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 4000.0f);
	_firstPersonCamera->RotateYaw(XM_PI/2);
	_firstPersonCamera->RotatePitch(-XM_PI/4);
	_firstPersonCamera->CalculateViewProjection();

	_frontViewCamera = new Camera({ -100.0f, 0.0f, 0.0f, 0.0f }, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 4000.0f);
	_frontViewCamera->CalculateViewProjection();

	_rearViewCamera = new Camera({ -100.0f, 0.0f, 0.0f, 0.0f }, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 4000.0f);
	_rearViewCamera->CalculateViewProjection();

	_thirdPersonCamera = new Camera({ 0.0f, 25.0f, 0.0f, 0.0f }, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 4000.0f);
	_thirdPersonCamera->CalculateViewProjection();

	_topDownCamera = new Camera(topDownEye, At, Up, (float)_WindowWidth, (float)_WindowHeight, 0.01f, 4000.0f);
	_topDownCamera->CalculateViewProjection();
	_topDownCamera->SetTopDown(true);
	_topDownCamera->RotatePitch(-XM_PI);

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(L"Lighting.fx", "VS", "vs_4_0", &pVSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Lighting.fx", "PS", "ps_4_0", &pPSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

	if (FAILED(hr))
		return hr;


	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
		return hr;

	// Set the input layout
	_pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBufferCube()
{
	HRESULT hr;

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		// Front Face
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)},
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },

		// Back Face
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f),XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		// Top Face
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },

		// Bottom Face
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		// Left Face
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) }, 
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },

		// Right Face
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferCube);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitVertexBufferPlane()
{
	HRESULT hr;

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.0f),  XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f),XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },

	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferPlane);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitIndexBufferCube()
{
	HRESULT hr;

	// Create index buffer of cube
	WORD indices[] =
	{
		// Front Face
		0, 1, 2,
		0, 2, 3,

		// Back Face
		4, 5, 6,
		4, 6, 7,

		// Top Face
		8, 9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBufferCube);

	if (FAILED(hr))
		return hr;

	return S_OK;
}


HRESULT Application::InitIndexBufferPlane()
{
	HRESULT hr;

	// Create index buffer of plane
	WORD indices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBufferPlane);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

void Application::InitInstanceBuffer()
{

}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	_hInst = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	_hWnd = CreateWindow(L"TutorialWindowClass", L"Lighting", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!_hWnd)
		return E_FAIL;

	ShowWindow(_hWnd, nCmdShow);

	return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob != nullptr)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

		if (pErrorBlob) pErrorBlob->Release();

		return hr;
	}

	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT Application::InitDevice()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = _WindowWidth;
	sd.BufferDesc.Height = _WindowHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	// Set depth/stencil buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	if (FAILED(hr))
		return hr;

	// Create the depth/stencil buffer
	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	if (FAILED(hr))
		return hr;

	hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
	pBackBuffer->Release();

	if (FAILED(hr))
		return hr;

	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)_WindowWidth;
	vp.Height = (FLOAT)_WindowHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	// Initialize all scene objects
	// Setup plane
	InitVertexBufferPlane();
	_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferPlane, _stride, _offset);
	InitIndexBufferPlane();
	_pImmediateContext->IASetIndexBuffer(_pIndexBufferPlane, DXGI_FORMAT_R16_UINT, 0);


	_sphereMesh = OBJLoader::Load("sphere.obj", _pd3dDevice);

	_skybox = new GameObject(_sphereMesh);
	_skybox->Initialise();

	MeshData planeMesh = { _pVertexBufferPlane, _pIndexBufferPlane, { _stride[0], _stride[1] }, 0, 6 };

	_plane = new GameObject(planeMesh);
	_plane->Initialise();

	for (int i = 0; i < 5; i++)
	{
		_tree[i] = new GameObject(planeMesh);
		_tree[i]->Initialise();
	}
	
	_stoneMesh = OBJLoader::Load("barrier.obj", _pd3dDevice);

	for (int i = 0; i < 5; i++)
	{
		_barrier[i] = new GameObject(_stoneMesh);
		_barrier[i]->Initialise();
	}

	// Setup cube
	InitVertexBufferCube();
	_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferCube, _stride, _offset);
	InitIndexBufferCube();
	_pImmediateContext->IASetIndexBuffer(_pIndexBufferCube, DXGI_FORMAT_R16_UINT, 0);

	MeshData cubeMesh = { _pVertexBufferCube, _pIndexBufferCube, { _stride[0], _stride[1] }, 0, 36 };

	_objMeshData = OBJLoader::Load("sphere.obj", _pd3dDevice);

	// Setup cars
	_carMeshData = OBJLoader::Load("car.obj", _pd3dDevice);
	_carOrange = new GameObject(_carMeshData);
	_carOrange->Initialise();
	_carOrange->position = { -800.0f, 0.0f, 0.0f };
	_carBlue = new GameObject(_carMeshData);
	_carBlue->Initialise();

	// Setup Barriers

	// Set primitive topology
	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

	if (FAILED(hr))
		return hr;

	// Setup lights
	plX = 10.0f;
	plY = 10.0f;
	plZ = 10.0f;

	// Create wireframe rasterizer
	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

	if (FAILED(hr))
		return hr;

	// Create solid rasterizer
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_SOLID;
	wfdesc.CullMode = D3D11_CULL_BACK;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_solidRender);

	if (FAILED(hr))
		return hr;

	// Create non-culling rasterizer
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_noCulling);
	
	if (FAILED(hr))
		return hr;
	
	// Load textures
	hr = CreateDDSTextureFromFile(_pd3dDevice, L"Crate_COLOR.dds", nullptr, &_pCrateColouredTextureRV);
	hr = CreateDDSTextureFromFile(_pd3dDevice, L"Water2.dds", nullptr, &_pWaterTextureRV);
	hr = CreateDDSTextureFromFile(_pd3dDevice, L"DraftTrack.dds", nullptr, &_pTrackTextureRV);
	hr = CreateDDSTextureFromFile(_pd3dDevice, L"Pine Tree.dds", nullptr, &_pPineTreeTextureRV);
	hr = CreateDDSTextureFromFile(_pd3dDevice, L"Stone.dds", nullptr, &_pStoneTextureRV);
	hr = CreateDDSTextureFromFile(_pd3dDevice, L"A8B8G8R8MipMap.dds", nullptr, &_pOrangeCarTextureRV);
	hr = CreateDDSTextureFromFile(_pd3dDevice, L"BlueCar.dds", nullptr, &_pBlueCarTextureRV);

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);
           

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	
	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));
	
	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
	
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;
	
	_pd3dDevice->CreateBlendState(&blendDesc, &_transparency);


	return S_OK;
}

void Application::Cleanup()
{
	if (_pImmediateContext) _pImmediateContext->ClearState();
	if (_pConstantBuffer) _pConstantBuffer->Release();
	if (_pVertexBufferCube) _pVertexBufferCube->Release();
	if (_pVertexBufferPlane) _pVertexBufferPlane->Release();
	if (_pIndexBufferCube) _pIndexBufferCube->Release();
	if (_pIndexBufferPlane) _pIndexBufferPlane->Release();
	if (_pVertexLayout) _pVertexLayout->Release();
	if (_pVertexShader) _pVertexShader->Release();
	if (_pPixelShader) _pPixelShader->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();
	if (_wireFrame) _wireFrame->Release();
	if (_solidRender) _solidRender->Release();
	if (_noCulling) _noCulling->Release();
	if (_pRenderTargetView) _pRenderTargetView->Release();
	if (_pSwapChain) _pSwapChain->Release();
	if (_pImmediateContext) _pImmediateContext->Release();
	if (_pd3dDevice) _pd3dDevice->Release();

	if (_firstPersonCamera) _firstPersonCamera->~Camera();
	if (_frontViewCamera) _frontViewCamera->~Camera();
	if (_rearViewCamera) _rearViewCamera->~Camera();
	if (_topDownCamera) _topDownCamera->~Camera();


	if (_pCrateColouredTextureRV) _pCrateColouredTextureRV->Release();
	if (_pWaterTextureRV) _pWaterTextureRV->Release();
	if (_pTrackTextureRV) _pTrackTextureRV->Release();
	if (_pPineTreeTextureRV) _pPineTreeTextureRV->Release();
	if (_pStoneTextureRV) _pStoneTextureRV->Release();
	if (_pOrangeCarTextureRV) _pOrangeCarTextureRV->Release();
	if (_pBlueCarTextureRV) _pBlueCarTextureRV->Release();
	if (_pSamplerLinear) _pSamplerLinear->Release();

	if (_skybox) _skybox->~GameObject();

	if (_plane) _plane->~GameObject();

	for (int i = 0; i < 5; i++)
		_tree[i]->~GameObject();

	for (int i = 0; i < 5; i++)
		_barrier[i]->~GameObject();

	if (_carOrange) _carOrange->~GameObject();
	if (_carBlue) _carBlue->~GameObject();

	if (_transparency) _transparency->Release();

	if (_dIMouse) _dIMouse->Unacquire();
}

void Application::DetectInput()
{
	DIMOUSESTATE mouseCurrentState;
	_dIMouse->Acquire();
	_dIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrentState);

	if (_currentCamera == 1)
	{
		_firstPersonCamera->RotatePitch(mouseCurrentState.lY * -0.01f);
		_firstPersonCamera->RotateYaw(mouseCurrentState.lX * -0.01f);
	}
	else if (_currentCamera == 2)
			_frontViewCamera->RotateYaw(mouseCurrentState.lX * -0.01f);

	//if (mouseCurrentState.rgbButtons)

	// Exit program
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		exit(0);
	}

	// Move Foward
	if (GetAsyncKeyState(0x57))
	{
		if (_currentCamera != 1)
		{
			_topDownCamera->Move(0.1f);
			_carOrange->position = { _carOrange->position.x + (0.335f * (cos(_frontViewCamera->GetYaw()) * cos(_frontViewCamera->GetPitch()))),
				0.0f,
				_carOrange->position.z + (0.335f * (sin(_frontViewCamera->GetYaw()) * cos(_frontViewCamera->GetPitch()))) };

		}
		else
			_firstPersonCamera->Move(0.1f);
	}

	// Move Backwards
	if (GetAsyncKeyState(0x53))
	{
		if (_currentCamera != 1)
		{
			_topDownCamera->Move(-0.1f);
			_carOrange->position = { _carOrange->position.x - (0.335f * (cos(_frontViewCamera->GetYaw()) * cos(_frontViewCamera->GetPitch()))),
				0.0f,
				_carOrange->position.z - (0.335f *(sin(_frontViewCamera->GetYaw()) * cos(_frontViewCamera->GetPitch()))) };
		}
		else			
			_firstPersonCamera->Move(-0.1f);
	}

	// Strafe Left
	if (GetAsyncKeyState(0x41))
	{
		if (_currentCamera != 1)
		{
			_topDownCamera->Strafe(-0.1f);
			//_carOrange->position = { _carOrange->position.x - 0.1f * cos(_firstPersonCamera->GetYaw() - XM_PIDIV2),
			//	0.0f,
			//	_carOrange->position.z + 0.1f * sin(_firstPersonCamera->GetYaw() - XM_PIDIV2) };
		}
		else
			_firstPersonCamera->Strafe(-0.1f);
	}

	// Strafe right
	if (GetAsyncKeyState(0x44))
	{
		if (_currentCamera != 1)
		{
			_topDownCamera->Strafe(0.1f);
			//_carOrange->position = { _carOrange->position.x + 0.1f * cos(_firstPersonCamera->GetYaw() - XM_PIDIV2),
			//	0.0f,
			//	_carOrange->position.z - 0.1f * sin(_firstPersonCamera->GetYaw() - XM_PIDIV2) };
		}
		else
			_firstPersonCamera->Strafe(0.1f);
	}

	// Fly Up
	if (GetAsyncKeyState(0x44))
	{
		_firstPersonCamera->Fly(0.1f);
		_topDownCamera->Fly(0.1f);
	}

	// Fly Down
	if (GetAsyncKeyState(0x44))
	{
		_firstPersonCamera->Fly(-0.1f);
		_topDownCamera->Fly(-0.1f);
	}



	if (GetAsyncKeyState(0x31))
	{
		_currentCamera = 1;
	}
	if (GetAsyncKeyState(0x32))
	{
		_currentCamera = 2;
	}
	if (GetAsyncKeyState(0x33))
	{
		_currentCamera = 3;
	}
	if (GetAsyncKeyState(0x34))
	{
		_currentCamera = 4;
	}
	if (GetAsyncKeyState(0x35))
	{
		_currentCamera = 5;
	}


	// Set Rasterizer (Q key)
	if (GetAsyncKeyState(0x51))
	{
		if (_render == solid)
			_render = wireFrame;
		else
			_render = solid;
	}

	if (GetAsyncKeyState(0x49))
	{
		plX += 0.1f;
	}
	if (GetAsyncKeyState(0x4F))
	{
		plY += 0.1f;
	}
	if (GetAsyncKeyState(0x50))
	{
		plZ += 0.1f;
	}
}

void Application::Update()
{
	// Update our time
	static float t = 0.0f;

	if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();

		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;

		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}

	// Keyboard input
	DetectInput();

	_frontViewCamera->SetEye({ _carOrange->position.x * 0.3f, 10.0f, _carOrange->position.z + 25.0f, 0.0f });
	_frontViewCamera->CalculateViewProjection();

	_rearViewCamera->SetEye({ _carOrange->position.x, 60.0f, _carOrange->position.z, 0.0f });
	_rearViewCamera->SetAt({ -_frontViewCamera->GetEye().x, -_frontViewCamera->GetEye().y, -_frontViewCamera->GetEye().z, 0.0f });
	_rearViewCamera->CalculateViewProjection();

	_firstPersonCamera->CalculateViewProjection();

	_thirdPersonCamera->SetAt({ _carOrange->position.x,
		_carOrange->position.y,
		_carOrange->position.z, 0.0f });
	_thirdPersonCamera->CalculateViewProjection();

	_topDownCamera->CalculateViewProjection();

	// 
	// Animate the cubes (worlds)
	// 


	// Regular Objects

	// Skybox
	_skybox->Update(t, { 3000.0f, 3000.0f, 3000.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
	_skybox->UpdateWorld();

	// Plane
	_plane->Update(t, { 2000.0f, 2000.0f, 2000.0f }, { -4.7f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });
	_plane->UpdateWorld();

	// Trees
	// Billboarding
	XMFLOAT4 cameraPosition = { 0.0f, 0.0f, 0.0f, 0.0f };
	double angle = 0.0;

	switch (_currentCamera)
	{
		case 1:
			cameraPosition = _firstPersonCamera->GetEye();
			break;
		case 2:
			cameraPosition = _frontViewCamera->GetEye();
			break;
		case 3:
			cameraPosition = _rearViewCamera->GetEye();
			break;
		case 4:
			cameraPosition = _thirdPersonCamera->GetEye();
			break;
		default:
			cameraPosition = _topDownCamera->GetEye();
			break;
	}
	
	float scale = 50.0f;

	// Trees
	for (int i = 0; i < 5; i++)
	{
		angle = atan2(_tree[i]->position.x - cameraPosition.x, _tree[i]->position.z - cameraPosition.z) * (180 / XM_PI);
		_tree[i]->Update(t, { scale, scale, scale }, { -(float)angle * 0.0174532925f, 0.0f, 4.65f }, { -5.0f + (i * 2.0f), 0.0f, 20.0f});
		_tree[i]->UpdateWorld();
	}

	// Barriers
	for (int i = 0; i < 5; i++)
	{
		_barrier[i]->Update(t, { 1.0f, 1.0f, 1.0f }, { 0.0f, XM_PI/2, 0.0f }, { 110.0f, 0.0f, -200 + (i * 200.0f) });
		_barrier[i]->UpdateWorld();
	}

	//cos(_firstPersonCamera->GetYaw())
	// Cars
	_carOrange->Update(t, { 0.3f, 0.3f, 0.3f }, { 0.0f, 0.0f, 0.0f }, { _carOrange->position.x,
		_carOrange->position.y,
		_carOrange->position.z });
	_carOrange->UpdateWorld();
	_carBlue->Update(t, { 0.3f, 0.3f, 0.3f }, { 0.0f, 0.0f, 0.0f }, { -100.0f, 0.0f, 0.0f });
	_carBlue->UpdateWorld();
} 

void Application::Draw()
{
	// 
	// Clear the back buffer
	// 
	float ClearColor[4] = { 0.5f, 0.5f, 1.0f, 1.0f }; // red,green,blue,alpha
	_pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ConstantBuffer cb;

	cb.diffuseMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cb.diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cb.lightVecW = XMFLOAT3(plX/100, plY/100, -plZ/100);

	cb.ambientMtrl = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
	cb.ambientLight = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);

	cb.specularMtrl = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cb.specularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cb.specularPower = 10.0f;

	cb.eyePosW = XMFLOAT3(0.0f, 0.0f, -2.5f);
	cb.gLightPos = XMFLOAT3(0.0f, 0.0f, -2.0f);
	cb.gLightRange = 100.0f;
	cb.gLightAtten = XMFLOAT3(0.0f, 0.2f, 0.0f);

	cb.spotCone = 20.0f;
	//cb.pad;


	// Render opaque objects

	XMFLOAT4X4 cameraView;
	XMFLOAT4X4 cameraProj;

	switch (_currentCamera)
	{
		case 1:
			cameraView = _firstPersonCamera->GetView();
			cameraProj = _firstPersonCamera->GetProjection();
			break;
		case 2:
			cameraView = _frontViewCamera->GetView();
			cameraProj = _frontViewCamera->GetProjection();
			break;
		case 3:
			cameraView = _rearViewCamera->GetView();
			cameraProj = _rearViewCamera->GetProjection();
			break;
		case 4:
			cameraView = _thirdPersonCamera->GetView();
			cameraProj = _thirdPersonCamera->GetProjection();
			break;
		default:
			cameraView = _topDownCamera->GetView();
			cameraProj = _topDownCamera->GetProjection();
			break;
	}


	XMMATRIX view = XMLoadFloat4x4(&cameraView);
	XMMATRIX projection = XMLoadFloat4x4(&cameraProj);


	// "fine-tune" the blending equation
	float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };

	// Set the default blend state (no blending) for opaque objects
	_pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	// 
	// Update variables
	// 


	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShaderResources(0, 1, &_pCrateColouredTextureRV);
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

	XMFLOAT4X4 world;

	// Skybox
	_pImmediateContext->RSSetState(_noCulling);
	_pImmediateContext->PSSetShaderResources(0, 1, &_pWaterTextureRV);
	world = _skybox->GetWorld();
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&world));
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_skybox->Draw(_pd3dDevice, _pImmediateContext);

	// Rasterizer
	if (_render == solid)
		_pImmediateContext->RSSetState(_solidRender);	// Set rasterizer to solid rendering
	else
		_pImmediateContext->RSSetState(_wireFrame); // Set rasterizer to wireframe rendering


	// Draw Cars
	_pImmediateContext->PSSetShaderResources(0, 1, &_pOrangeCarTextureRV);
	world = _carOrange->GetWorld();
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&world));
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_carOrange->Draw(_pd3dDevice, _pImmediateContext);

	_pImmediateContext->PSSetShaderResources(0, 1, &_pBlueCarTextureRV);
	world = _carBlue->GetWorld();
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&world));
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_carBlue->Draw(_pd3dDevice, _pImmediateContext);
	
	// Draw Trees
	_pImmediateContext->PSSetShaderResources(0, 1, &_pPineTreeTextureRV);

	for (int i = 0; i < 5; i++)
	{
		world = _tree[i]->GetWorld();
		cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&world));
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		_tree[i]->Draw(_pd3dDevice, _pImmediateContext);
	}

	// Draw Barriers
	_pImmediateContext->PSSetShaderResources(0, 1, &_pStoneTextureRV);

	for (int i = 0; i < 5; i++)
	{
		world = _barrier[i]->GetWorld();
		cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&world));
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		_barrier[i]->Draw(_pd3dDevice, _pImmediateContext);
	}

	//Set the blend state for transparent objects
	//_pImmediateContext->OMSetBlendState(_transparency, blendFactor, 0xffffffff);

	_pImmediateContext->PSSetShaderResources(0, 1, &_pTrackTextureRV);
	world = _plane->GetWorld();
	cb.mWorld = XMMatrixTranspose(XMLoadFloat4x4(&world));
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_plane->Draw(_pd3dDevice, _pImmediateContext);

	// 
	// Present our back buffer to our front buffer
	// 
	_pSwapChain->Present(0, 0);
}
