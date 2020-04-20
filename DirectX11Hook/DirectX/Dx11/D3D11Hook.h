#ifndef _DXGIHOOK_H_
#define _DXGIHOOK_H_

#ifdef _MSC_VER
#pragma once
#endif

// Standard Includes
#include <iostream>
#include <functional> //callback

// DirectX Includes
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "..\..\Util\Memory\HookLib.h"
#include "D3D11VtableIndex.h"
#include "..\..\Util\Log.h"

typedef HRESULT( __stdcall *D3D11PresentHook )( IDXGISwapChain* This, UINT SyncInterval, UINT Flags );
D3D11PresentHook oPresent = nullptr;

class DXGIHook
{
public:
	static std::function<void( void )> g_pCallBack;

	static HRESULT __stdcall hkDXGIPresent( IDXGISwapChain* This, UINT SyncInterval, UINT Flags )
	{
		if ( g_pCallBack )
		{
			__try
			{
				g_pCallBack( );
			}
			__except ( 1 )
			{

			}
		}
		return oPresent( This, SyncInterval, Flags );
	}

	DXGIHook( )
	{
		m_pIDXGISwapChainHook = new HookLib::CVMTHook( );
		m_bInitialized = false;
	}

	~DXGIHook( );	
	
	ID3D11Device* Device( ) const { return m_pDevice; }
	ID3D11DeviceContext* Context( ) const { return m_pContext; }
	IDXGISwapChain* SwapChain( ) const { return m_pSwapChain; }
		
	bool HookDXGI( )
	{
		HWND hWnd = GetForegroundWindow( );
		if ( hWnd == nullptr )
			return false;

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.Windowed = ( GetWindowLong( hWnd, GWL_STYLE ) & WS_POPUP ) != 0 ? FALSE : TRUE;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		D3D_DRIVER_TYPE driverTypes[ ] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE
		};
		UINT numDriverTypes = ARRAYSIZE( driverTypes );

		D3D_FEATURE_LEVEL featureLevels[ ] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};
		UINT numFeatureLevels = ARRAYSIZE( featureLevels );

		ID3D11Device* pDevice = nullptr;
		ID3D11DeviceContext* pContext = nullptr;
		IDXGISwapChain* pSwapChain = nullptr;

		HRESULT result;
		for ( unsigned int i = 0; i < numDriverTypes; i++ )
		{
			result = D3D11CreateDeviceAndSwapChain( nullptr, driverTypes[ i ], nullptr, 0,
				featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, nullptr, &pContext );

			if ( SUCCEEDED( result ) )
			{
				break;
			}
		}

		if ( FAILED( result ) )
		{
			printf("[!] D3D11CreateDeviceAndSwapChain Error Code: 0x%X", result);
			return false;
		}

	DWORD_PTR* m_pSwapChainVtable = reinterpret_cast< DWORD_PTR* >( pSwapChain );
	m_pSwapChainVtable = reinterpret_cast< DWORD_PTR* >( m_pSwapChainVtable[ 0 ] );
	DWORD_PTR dwVTableAddress = 0;

	#ifdef _AMD64_
	#define _PTR_MAX_VALUE 0x7FFFFFFEFFFF
		MEMORY_BASIC_INFORMATION64 mbi = { 0 };
	#else
	#define _PTR_MAX_VALUE 0xFFE00000
		MEMORY_BASIC_INFORMATION32 mbi = { 0 };
	#endif

		for ( DWORD_PTR memptr = 0x10000; memptr < _PTR_MAX_VALUE; memptr = mbi.BaseAddress + mbi.RegionSize )
		{
			if ( !VirtualQuery( reinterpret_cast< LPCVOID >( memptr ), reinterpret_cast< PMEMORY_BASIC_INFORMATION >( &mbi ), sizeof( MEMORY_BASIC_INFORMATION ) ) )
				continue;

			if ( mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS || ( mbi.Protect & PAGE_GUARD ) )
				continue;

			DWORD_PTR len = mbi.BaseAddress + mbi.RegionSize;

			for ( DWORD_PTR current = static_cast< DWORD_PTR >( mbi.BaseAddress ); current < len; ++current )
			{
				__try
				{
					dwVTableAddress = *reinterpret_cast< DWORD_PTR* >( current );
				}
				__except ( 1 )
				{
					continue;
				}

				if ( dwVTableAddress == reinterpret_cast< DWORD_PTR >( m_pSwapChainVtable ) )
				{
					this->m_pSwapChain = reinterpret_cast< IDXGISwapChain* >( dwVTableAddress );

					m_pIDXGISwapChainHook->Initialize( reinterpret_cast<PDWORD_PTR*>(m_pSwapChain) );
					oPresent = reinterpret_cast< D3D11PresentHook >( m_pIDXGISwapChainHook->HookMethod( reinterpret_cast< DWORD_PTR >( hkDXGIPresent ), SC_PRESENT ) );
													
					if ( HRESULT( m_pSwapChain->GetDevice( __uuidof( ID3D11Device ), ( LPVOID* )&m_pDevice )) < 0 || !m_pDevice )
					{
						printf( "[!] GetDevice Failed \n" );
						break;
					}						

					m_pDevice->GetImmediateContext( &m_pContext );
					if ( !m_pContext )
					{
						break;						
					}	

					m_bInitialized = true;
					break;
				}
			}
		}

		if ( pSwapChain )
		{
			pSwapChain->Release( );
			pSwapChain = nullptr;
		}
		if ( pDevice )
		{
			pDevice->Release( );
			pDevice = nullptr;
		}
		if ( pContext )
		{
			pContext->Release( );
			pContext = nullptr;
		}	

		return m_bInitialized;
	}

	void ReHook( )
	{
		if ( m_bInitialized == true && m_pIDXGISwapChainHook )
		{
			if ( m_pIDXGISwapChainHook->GetMethodAddress( SC_PRESENT ) != reinterpret_cast< DWORD_PTR >( hkDXGIPresent ) )
				m_pIDXGISwapChainHook->ReHook( );
		}
	}

	bool UnHook( )
	{
		SAFE_DELETE( m_pIDXGISwapChainHook );
	}

	bool Initialized( ) const 
	{
		return m_bInitialized;
	}

	void Log(  )
	{
		Log::NewLine( );
		Log::Add( "IDXGISwapChain:      0x%I64x", this->m_pSwapChain );
		Log::Add( "ID3D11Device:        0x%I64x", this->m_pDevice );
		Log::Add( "ID3D11DeviceContext: 0x%I64x", this->m_pContext );
		Log::NewLine( );
	}

	private:
		bool m_bInitialized;		
	
		HookLib::CVMTHook* m_pIDXGISwapChainHook = nullptr;

		ID3D11Device* m_pDevice = nullptr;
		ID3D11DeviceContext* m_pContext = nullptr;		
		IDXGISwapChain* m_pSwapChain = nullptr;	
};

#endif