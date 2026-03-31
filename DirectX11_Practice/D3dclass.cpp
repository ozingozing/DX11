////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "d3dclass.h"

D3DClass::D3DClass()
{
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;

	// 클래스 생성자에서 새 깊이 스텐실 상태를 null로 초기화합니다.
	m_depthDisabledStencilState = 0;
}


D3DClass::D3DClass(const D3DClass& other)
{
}


D3DClass::~D3DClass()
{
}

bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	float fieldOfView, screenAspect;

	// 새로운 깊이 스텐실 설정을 위한 새로운 깊이 스텐실 설명 변수가 있습니다.
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;

	// vsync(Vertical Synchronization 수직동기화) 설정을 저장합니다.
	m_vsync_enabled = vsync;

	// Direct3D를 초기화하기 전에, 비디오 카드/모니터의 주사율(Refresh Rate)을 먼저 얻어와야 합니다.
	// 컴퓨터마다 환경이 조금씩 다르므로, 해당 정보를 직접 조회(Query)해야 합니다.
	// 우리는 주사율을 분수 형태로 표현하는 numerator(분자)와 denominator(분모) 값을 구한 뒤,
	// 초기화 과정에서 DirectX에 전달합니다. 그러면 DirectX가 이를 기반으로 올바른 주사율을 계산합니다.
	//
	// 만약 이 과정을 생략하고 모든 PC에 존재하지 않을 수도 있는 “기본값” 주사율을 임의로 지정하면,
	// DirectX는 버퍼 플립(buffer flip) 대신 블릿(blit) 방식으로 동작하게 되어 성능이 저하되고,
	// 디버그 출력에 성가신 오류 메시지가 발생할 수 있습니다.

	// DirectX 그래픽 인터페이스 팩토리를 생성합니다.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	// 팩토리를 사용하여 기본 그래픽 인터페이스(비디오 카드)용 어댑터를 생성합니다.
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// 기본 어댑터 출력(모니터)을 열거합니다.
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// 어댑터 출력(모니터)에 대해 DXGI_FORMAT_R8G8B8A8_UNORM 디스플레이 형식에 맞는 모드 수를 가져옵니다.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 이 모니터/비디오 카드 조합에 사용할 수 있는 모든 디스플레이 모드를 저장할 목록을 생성합니다.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// 이제 표시 모드 목록 구조체를 채웁니다.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// 이제 모든 디스플레이 모드를 살펴보고 화면 너비와 높이에 맞는 모드를 찾습니다.
	// 일치하는 모드를 찾으면 해당 모니터의 새로 고침률의 분자와 분모를 저장합니다.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// 이제 주사율(Refresh Rate)에 대한 numerator(분자)와 denominator(분모) 값을 확보했습니다.
	// 어댑터(adapter)를 통해 마지막으로 가져올 정보는 비디오 카드의 이름과 비디오 메모리 용량입니다.

	// 어댑터(비디오 카드) 설명을 가져옵니다.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// 전용 비디오 카드 메모리를 메가바이트 단위로 저장합니다.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// 비디오 카드 이름을 문자 배열로 변환하여 저장합니다.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// 주사율(Refresh Rate)의 numerator(분자)와 denominator(분모) 값, 그리고 비디오 카드 정보를 저장했으므로,
	// 해당 정보를 가져오는 데 사용했던 구조체와 인터페이스는 이제 해제(release)해도 됩니다.

	// 디스플레이 모드 목록을 해제합니다.
	delete[] displayModeList;
	displayModeList = 0;

	// 어댑터 출력을 해제합니다.
	adapterOutput->Release();
	adapterOutput = 0;

	// 어댑터를 해제합니다.
	adapter->Release();
	adapter = 0;

	// factory도 해제합니다.
	factory->Release();
	factory = 0;

	// 시스템에서 주사율(Refresh Rate)을 얻었으므로, 이제 DirectX 초기화를 시작할 수 있습니다.
	// 가장 먼저 스왑 체인(Swap Chain)의 설명(Description)을 채웁니다.
	//
	// 스왑 체인은 그래픽이 그려질 프론트 버퍼(front buffer)와 백 버퍼(back buffer)를 의미합니다.
	// 일반적으로는 백 버퍼 1개를 사용하여 모든 렌더링을 그 버퍼에 수행한 뒤,
	// 렌더링이 끝나면 백 버퍼를 프론트 버퍼와 교체(swap)하여 화면에 표시합니다.
	// 이러한 “교체” 과정 때문에 swap chain이라는 이름이 붙었습니다.

	// 스왑 체인 설명을 초기화합니다.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// 백 버퍼를 하나로 설정합니다.
	swapChainDesc.BufferCount = 1;

	// 백 버퍼의 너비와 높이를 설정합니다.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// 백 버퍼에 일반 32비트 서페이스를 설정합니다.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// 스왑 체인 설명에서 다음으로 설정할 항목은 주사율(Refresh Rate)입니다.
	// 주사율은 1초에 몇 번 백 버퍼(back buffer)의 내용을 프론트 버퍼(front buffer)로 그려(전달해) 화면에 표시하는지를 의미합니다.
	//
	// 만약 applicationclass.h에서 vsync를 true로 설정했다면,
	// 주사율이 시스템 설정(예: 60Hz)에 맞춰 고정됩니다.
	// 즉, 화면은 1초에 60번만 갱신되며(시스템 주사율이 60Hz보다 높다면 그에 맞춰 더 높아질 수 있음),
	// 프레임 출력이 모니터 주사율과 동기화됩니다.
	//
	// 반대로 vsync를 false로 설정하면,
	// 가능한 한 많이 화면을 그리게 되지만(프레임 제한 없이 최대치로 출력),
	// 이 경우 화면 찢어짐(티어링) 같은 시각적 아티팩트가 발생할 수 있습니다.

	// 백 버퍼의 새로 고침 빈도를 설정합니다.
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// 백 버퍼의 사용 방식을 설정합니다.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// 렌더링할 창의 핸들을 설정합니다.
	swapChainDesc.OutputWindow = hwnd;

	// 멀티샘플링을 끄세요.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// 전체 화면 모드 또는 창 모드로 설정하세요.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// 스캔 라인 순서 및 배율을 지정하지 않음으로 설정합니다.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 화면 표시 후 백 버퍼의 내용을 버립니다.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// 고급 플래그를 설정하지 마십시오.
	swapChainDesc.Flags = 0;

	// 스왑 체인 설명을 설정한 뒤에는, feature level이라는 변수를 하나 더 설정해야 합니다.
	// 이 값은 DirectX에서 어떤 기능 수준(= 어떤 버전/세대의 기능 집합)을 사용할지 알려줍니다.
	//
	// 여기서는 feature level을 11.0으로 설정하여 DirectX 11 기능을 사용하도록 합니다.
	// 만약 여러 버전을 지원하거나 사양이 낮은 하드웨어까지 고려해야 한다면,
	// 10 또는 9로 설정하여 더 낮은 수준의 DirectX 기능을 사용하도록 구성할 수도 있습니다.

	// 기능 수준을 DirectX 11로 설정합니다.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// 스왑 체인 설명(Swap Chain Description)과 feature level 설정을 모두 마쳤으므로,
	// 이제 스왑 체인, Direct3D 디바이스(Direct3D device), 그리고 디바이스 컨텍스트(Direct3D device context)를 생성할 수 있습니다.
	//
	// Direct3D 디바이스와 디바이스 컨텍스트는 매우 중요한 객체로,
	// Direct3D의 거의 모든 기능에 접근하기 위한 인터페이스 역할을 합니다.
	// 이 시점부터 대부분의 작업에서 디바이스와 디바이스 컨텍스트를 계속 사용하게 됩니다.
	//
	// 이전 버전의 DirectX에 익숙한 분이라면 Direct3D 디바이스는 익숙하겠지만,
	// Direct3D 디바이스 컨텍스트는 새롭게 느껴질 수 있습니다.
	// 간단히 말해, 기존 Direct3D 디바이스가 담당하던 기능을 두 개로 분리했기 때문에,
	// 이제는 디바이스와 컨텍스트를 함께 사용해야 합니다.
	//
	// 참고로 사용자의 그래픽 카드가 DirectX 11을 지원하지 않으면,
	// 이 함수 호출은 디바이스 및 디바이스 컨텍스트 생성에 실패합니다.
	// 또한 DirectX 11 기능을 테스트하고 싶은데 DirectX 11 그래픽 카드가 없다면,
	// D3D_DRIVER_TYPE_HARDWARE 대신 D3D_DRIVER_TYPE_REFERENCE를 사용하여
	// GPU 대신 CPU로 렌더링하도록 할 수 있습니다.
	// 다만 이 방식은 속도가 약 1/1000 수준으로 매우 느리지만,
	// DirectX 11 GPU가 없는 환경에서도 테스트를 진행할 수 있다는 장점이 있습니다.

	// 스왑 체인, Direct3D 장치 및 Direct3D 장치 컨텍스트를 생성합니다.
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(result))
	{
		return false;
	}

	// 백 버퍼에 대한 포인터를 가져옵니다.
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// 경우에 따라 디바이스 생성 호출이 실패할 수 있는데, 그 이유는 기본(Primary) 그래픽 카드가 DirectX 11과 호환되지 않기 때문입니다.
	// 어떤 PC는 기본 그래픽 카드가 DirectX 10이고, 보조(Secondary) 그래픽 카드가 DirectX 11인 구성일 수도 있습니다.
	// 또한 하이브리드 그래픽 환경에서는 기본이 저전력 Intel 내장 그래픽이고,
	// 보조가 고성능 Nvidia 그래픽인 형태로 동작하는 경우도 있습니다.
	//
	// 이러한 문제를 피하려면 “기본 디바이스(default device)”를 그대로 사용하지 말고,
	// 시스템에 장착된 모든 그래픽 카드를 열거(enumerate)한 뒤 사용자가 사용할 카드를 선택하게 하고,
	// 디바이스 생성 시 선택된 그래픽 카드를 명시적으로 지정해야 합니다.
	//
	// 이제 스왑 체인(swap chain)을 생성했으므로,
	// 백 버퍼(back buffer)에 대한 포인터를 얻어온 다음 이를 스왑 체인에 연결(attach)해야 합니다.
	// 이를 위해 CreateRenderTargetView 함수를 사용하여 백 버퍼를 스왑 체인에 연결합니다.

	// 백 버퍼 포인터를 사용하여 렌더링 대상 뷰를 생성합니다.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// 더 이상 필요하지 않으므로 백 버퍼에 대한 포인터를 해제합니다.
	backBufferPtr->Release();
	backBufferPtr = 0;

	// 또한 깊이 버퍼(depth buffer) 설명(Description)도 설정해야 합니다.
	// 이를 통해 깊이 버퍼를 생성하면, 폴리곤이 3D 공간에서 올바르게 렌더링될 수 있습니다.
	// 동시에 스텐실 버퍼(stencil buffer)도 깊이 버퍼에 함께 붙여서 사용합니다.
	// 스텐실 버퍼는 모션 블러, 볼류메트릭 섀도우(체적 그림자) 등 다양한 효과를 구현하는 데 활용될 수 있습니다.

	// 깊이 버퍼의 설명을 초기화합니다.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// 깊이 버퍼에 대한 설명을 설정합니다.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// 이제 위에서 설정한 설명(Description)을 사용하여 depth/stencil 버퍼를 생성합니다.
	// 여기서 CreateTexture2D 함수를 사용해 버퍼를 만들기 때문에,
	// depth/stencil 버퍼는 결국 “2D 텍스처” 형태로 생성된다는 점을 확인할 수 있습니다.
	//
	// 그 이유는 폴리곤이 정렬되고 래스터라이즈(rasterize)되는 과정 이후에는,
	// 결국 이 2D 버퍼 위에 색이 칠해진 픽셀들로 변환되기 때문입니다.
	// 그리고 최종적으로 이 2D 버퍼가 화면에 그려져 출력됩니다.

	// 래스터라이즈(rasterize)
	// 이 삼각형이 포함하는 모든 픽셀마다 Pixel Shader(fragment shader)가 실행된다.
	// 삼각형의 세 정점에 할당 되었던 여러 데이터(pos, uv, normal, color)가 보간되어 삼각형 내부에 각 픽셀셰이더로 넘어옵니다.
	// DirectX에서 이러한 일련의 과정을 레스터라이제이션이라고 부르고 
	// 고정 파이프라인 단계로 프로그래머가 이러한 로직들을 임의 바꿀수 없는 파이프라인 단계이다.
	// 자체 알고리즘으로 알아서 동작을 합니다.

	// 입력된 설명을 사용하여 깊이 버퍼용 텍스처를 생성합니다.
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 이제 Depth-Stencil 상태(Depth Stencil State)에 대한 설명(Description)을 설정해야 합니다.
	// 이를 통해 Direct3D가 각 픽셀에 대해 어떤 방식의 깊이 테스트(depth test)를 수행할지 제어할 수 있습니다.

	// 스텐실 상태에 대한 설명을 초기화합니다.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// 스텐실 상태에 대한 설명을 설정합니다.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// 픽셀이 정면을 향하고 있는 경우 스텐실 작업.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// 픽셀이 뒷면을 향하고 있는 경우 스텐실 작업.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// 설명(Description)을 모두 채웠으므로,
	// 이제 Depth-Stencil 상태(Depth Stencil State)를 생성할 수 있습니다.
	// 깊이 스텐실 상태를 생성합니다.
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// 생성한 Depth-Stencil 상태(Depth Stencil State)를 이제 적용(설정)하여 실제로 동작하게 합니다.
	// 이 설정은 디바이스 컨텍스트(device context)를 통해 적용된다는 점에 유의하세요.
	// 깊이 스텐실 상태를 설정합니다.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// 다음으로 생성해야 할 것은 depth/stencil 버퍼의 “뷰(view)”에 대한 설명(Description)입니다.
	// 이 설정을 통해 Direct3D가 깊이 버퍼를 depth-stencil 텍스처로 사용해야 한다는 것을 알 수 있습니다.
	// 설명을 모두 채운 뒤에는 CreateDepthStencilView 함수를 호출하여 실제 뷰를 생성합니다.
	// 깊이 스텐실 뷰를 초기화합니다.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// 깊이 스텐실 뷰 설명을 설정합니다.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// 깊이 스텐실 뷰를 생성합니다.
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	// 이제 필요한 것들이 생성되었으므로 OMSetRenderTargets를 호출할 수 있습니다.
	// 이 함수는 렌더 타깃 뷰(Render Target View)와 Depth-Stencil 버퍼를
	// 출력 머지 단계(Output Merger, OM) 파이프라인에 바인딩합니다.
	//
	// 이렇게 바인딩해두면 렌더링 파이프라인이 출력하는 그래픽 결과가
	// 이전에 생성해둔 백 버퍼(back buffer)에 그려지게 됩니다.
	// 백 버퍼에 그래픽이 기록되면, 이후 이를 프론트 버퍼(front buffer)로 스왑하여
	// 사용자의 화면에 최종 결과를 표시할 수 있습니다.
	// 렌더링 대상 뷰와 깊이 스텐실 버퍼를 출력 렌더링 파이프라인에 바인딩합니다.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// 렌더 타깃 설정이 끝났으므로, 이후 튜토리얼에서 씬을 더 세밀하게 제어하기 위한 추가 설정들을 진행합니다.
	// 그 첫 번째로 래스터라이저 상태(Rasterizer State)를 생성합니다.
	// 래스터라이저 상태를 통해 폴리곤이 어떤 방식으로 렌더링될지 제어할 수 있습니다.
	//
	// 예를 들어 씬을 와이어프레임(wireframe) 모드로 렌더링하거나,
	// 폴리곤의 앞면(front face)과 뒷면(back face)을 모두 그리도록 설정할 수도 있습니다.
	//
	// 기본적으로 DirectX는 아래와 동일한 동작을 하는 기본 래스터라이저 상태를 이미 설정해 두고 사용하고 있습니다.
	// 하지만 직접 래스터라이저 상태를 만들어 적용하지 않으면, 그 기본 설정을 변경하거나 제어할 방법이 없습니다.
	// 폴리곤이 어떻게 그리고 어떤 폴리곤이 그려질지를 결정하는 래스터 설명을 설정합니다.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// 방금 입력한 설명을 기반으로 래스터라이저 상태를 생성합니다.
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	// 이제 래스터라이저 상태를 설정합니다.
	m_deviceContext->RSSetState(m_rasterState);

	// 또한 뷰포트(viewport)도 설정해야 Direct3D가 클립 공간(clip space) 좌표를
	// 렌더 타깃(render target) 공간으로 올바르게 매핑할 수 있습니다.
	// 뷰포트는 창(window) 전체 크기를 사용하도록 설정합니다.
	// 렌더링을 위한 뷰포트를 설정합니다.
	m_viewport.Width = (float)screenWidth;
	m_viewport.Height = (float)screenHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	// 뷰포트를 생성합니다.
	m_deviceContext->RSSetViewports(1, &m_viewport);

	// 투영 행렬을 설정합니다.
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	// 3D 렌더링을 위한 투영 행렬을 생성합니다.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	// 또한 월드 행렬(world matrix)이라는 또 다른 행렬을 생성합니다.
	// 월드 행렬은 오브젝트의 정점(vertices)을 3D 씬 공간상의 정점으로 변환하는 데 사용됩니다.
	// 이 행렬은 3D 공간에서 오브젝트를 회전(rotate), 이동(translate), 스케일(scale)하는 데에도 활용됩니다.
	//
	// 처음에는 월드 행렬을 단위 행렬(identity matrix)로 초기화하고,
	// 이 객체 안에 그 사본(copy)을 저장해 둡니다.
	// 이 사본은 렌더링 시 셰이더(shaders)로 전달해야 하므로 필요합니다.
	// 월드 행렬을 단위 행렬로 초기화합니다.
	m_worldMatrix = XMMatrixIdentity();

	// 보통 이 지점에서 뷰 행렬(view matrix)을 생성합니다.
	// 뷰 행렬은 우리가 씬을 “어디에서 바라보고 있는지”를 계산하는 데 사용됩니다.
	// 카메라처럼 생각하면 이해하기 쉽습니다. 즉, 우리는 이 카메라를 통해서만 씬을 바라보게 됩니다.
	//
	// 다만 뷰 행렬은 역할상 카메라 클래스에 두는 것이 더 자연스럽기 때문에,
	// 이후 튜토리얼에서 카메라 클래스를 만들 때 그쪽에서 생성하도록 하고,
	// 여기서는 일단 생성을 건너뜁니다.
	//
	// 그리고 Initialize 함수에서 마지막으로 설정할 것은 직교 투영 행렬(orthographic projection matrix)입니다.
	// 이 행렬은 화면에 UI 같은 2D 요소를 렌더링할 때 사용되며,
	// 3D 렌더링 과정을 생략하고 2D 렌더링을 할 수 있게 해줍니다.
	// 이후 튜토리얼에서 2D 그래픽과 폰트를 화면에 그릴 때 이 행렬이 사용되는 것을 보게 될 것입니다.
	// 2D 렌더링을 위한 직교 투영 행렬을 생성합니다.
	m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);


	// 여기서는 깊이 스텐실에 대한 설명을 설정합니다.
	// 이 새로운 깊이 스텐실과 이전 스텐실의 유일한 차이점은
	// 2D 도면의 경우 DepthEnable이 false로 설정되어 있다는 점입니다.
	// 
	// 매개변수를 설정하기 전에 두 번째 깊이 스텐실 상태를 지우십시오.
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	// 이제 2D 렌더링 시 Z 버퍼를 끄는 두 번째 깊이 스텐실 상태를 생성합니다.
	// 유일한 차이점은 DepthEnable이 false로 설정된다는 점이며,
	// 다른 모든 매개변수는 다른 깊이 스텐실 상태와 동일합니다.
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// 이제 새로운 깊이 스텐실을 만드세요.
	// device를 사용하여 상태를 생성합니다.
	result = m_device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	// Here we release the new depth stencil during the Shutdown function.
	if (m_depthDisabledStencilState)
	{
		m_depthDisabledStencilState->Release();
		m_depthDisabledStencilState = 0;
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}

void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}

void D3DClass::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if (m_vsync_enabled)
	{
		// Lock to screen refresh rate.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_swapChain->Present(0, 0);
	}

	return;
}

ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}


void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}


void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}

void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}

void D3DClass::SetBackBufferRenderTarget()
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	return;
}

void D3DClass::ResetViewport()
{
	// Set the viewport.
	m_deviceContext->RSSetViewports(1, &m_viewport);

	return;
}

// 다음은 Z 버퍼를 활성화 및 비활성화하는 새로운 기능입니다.
// Z 버퍼링을 켜려면 원래 깊이 스텐실을 설정합니다.
// Z 버퍼링을 끄려면 depthEnable이 false로 설정된 새 깊이 스텐실을 설정합니다.
// 일반적으로 이러한 기능을 사용하는 가장 좋은 방법은 먼저 모든 3D 렌더링을 완료한 다음
// Z 버퍼를 끄고 2D 렌더링을 수행하고 마지막으로 Z 버퍼를 다시 켜는 것입니다.
void D3DClass::TurnZBufferOn()
{
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
	return;
}

void D3DClass::TurnZBufferOff()
{
	m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
	return;
}