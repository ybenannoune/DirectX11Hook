#include "DxgiHook.h"

namespace HookHandler
{
	typedef HRESULT( __stdcall *DXGIPresentHook )( IDXGISwapChain* pSwapChain, UINT SyncInterval , UINT Flags );
	DXGIPresentHook g_oPresent = nullptr;

	std::function<void( void )> g_pPresentCallBack = nullptr;
	bool g_canEject = false;
	bool g_Initialized = false;
	ID3D11Device* g_pDevice = NULL;
	ID3D11DeviceContext* g_pContext = NULL;
	IDXGISwapChain* g_pSwapChain = NULL;

	HRESULT __stdcall hkDXGIPresent( IDXGISwapChain* pSwapChain, UINT SyncInterval , UINT Flags )
	{
		g_canEject = false;

		if(g_Initialized == false)
		{
			g_pSwapChain = pSwapChain;
			pSwapChain->GetDevice(__uuidof(g_pDevice), (void**)&g_pDevice);
			g_pDevice->GetImmediateContext(&g_pContext);
			g_Initialized = true;
		}

		if (g_pPresentCallBack)
		{
			g_pPresentCallBack();
		}			
	
		HRESULT ret = g_oPresent(pSwapChain, SyncInterval , Flags );

		g_canEject = true;
		return ret;
	}	
}

DXGIHook::DXGIHook( )
{		
	m_bInitialized = false;
}

DXGIHook::~DXGIHook(  )
{
	m_bInitialized = false;	
	m_pD11Device = nullptr;
	m_pContext = nullptr;
	m_pSwapChain = nullptr;

	while ( HookHandler::g_canEject == false );
}

ID3D11Device* DXGIHook::GetD11Device( ) const
{ 
	return m_pD11Device;
}

ID3D11DeviceContext* DXGIHook::GetContext( ) const
{ 
	return m_pContext;
}

IDXGISwapChain* DXGIHook::GetSwapChain( ) const
{ 
	return m_pSwapChain;
}

void DXGIHook::SetCallBack( std::function<void( void )> _callback )
{
	HookHandler::g_pPresentCallBack = _callback;
}

bool DXGIHook::HookDXGI( )
{
	if ( this->Initialized( ) )
		return false;

	HWND hWnd = GetForegroundWindow( );
	if ( hWnd == nullptr )
		return false;

	ID3D11Device* pDevice = nullptr;
	ID3D11DeviceContext* pContext = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;//((GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? FALSE : TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext)))
	{
		MessageBox(hWnd, "Failed to create directX device and swapchain!", "", MB_ICONERROR);
		return NULL;
	}

	uintptr_t CreatedSwapChainAddress = reinterpret_cast< uintptr_t >( pSwapChain );
	uintptr_t* m_pSwapChainVtable = reinterpret_cast< uintptr_t* >( pSwapChain );
	m_pSwapChainVtable = reinterpret_cast< uintptr_t* >( m_pSwapChainVtable[0] );
		
	HookHandler::g_oPresent = reinterpret_cast<HookHandler::DXGIPresentHook>(HookLib::DetourCreate((BYTE*)m_pSwapChainVtable[SC_PRESENT], (BYTE*)HookHandler::hkDXGIPresent, 5));

	DWORD dwOld;
	VirtualProtect(HookHandler::g_oPresent, 2, PAGE_EXECUTE_READWRITE, &dwOld);

	//Unsafe
	while (HookHandler::g_Initialized == false)
		Sleep(100);

	m_pSwapChain = HookHandler::g_pSwapChain;
	m_pD11Device = HookHandler::g_pDevice;
	m_pContext = HookHandler::g_pContext;

	SAFE_RELEASE( pSwapChain );
	SAFE_RELEASE( pDevice );
	SAFE_RELEASE( pContext );		
	
	this->m_bInitialized = true;

	return true;
}

bool DXGIHook::Initialized( ) const
{
	return m_bInitialized;
}
