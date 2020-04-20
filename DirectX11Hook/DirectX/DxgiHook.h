#ifndef _DXGIHOOK_H_
#define _DXGIHOOK_H_

#ifdef _MSC_VER
#pragma once
#endif

// Standard Includes
#include "..\Util\Common.h"

#include <iostream>
#include <functional> 
#include <vector>
#include "DxRenderer.h"

#include "..\Util\Memory\HookLib.h"
#include "Dx11/D3D11VtableIndex.h"


class DXGIHook
{
public:	
	DXGIHook( );	
	~DXGIHook( );
		
	ID3D11Device* GetD11Device( ) const;
	ID3D11DeviceContext* GetContext( ) const;
	IDXGISwapChain* GetSwapChain( ) const;
		
	void SetCallBack( std::function<void( void )> _callback );
	bool HookDXGI( );
	bool Initialized( ) const;
	
private:
	bool							m_bInitialized;		
	ID3D11Device*					m_pD11Device = nullptr;
	ID3D11DeviceContext*			m_pContext = nullptr;
	IDXGISwapChain*					m_pSwapChain = nullptr;	
};

#endif