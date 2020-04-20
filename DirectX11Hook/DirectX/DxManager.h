#ifndef _DXMGR_H_
#define _DXMGR_H_

#ifdef _MSC_VER
#pragma once
#endif

// Standard Includes
#include <iostream>
#include "DXRenderer.h"

//D3D11
#include "DX11\D3D11Rendering.h"
#include "DxgiHook.h"

class DXManager 
{
public:
	DXManager( );
	~DXManager( );		
	
	void DrawLine( int x1 , int y1 , int x2 , int y2 , D3DCOLOR color );
	void DrawText( int x , int y , int scale , bool outLined , D3DCOLOR color , char *szText , ... );
	void DrawCircle( int x , int y , int radius , D3DCOLOR color );
	void DrawFilledRect( int x , int y , int w , int h , D3DCOLOR color );
	void DrawRect( int x , int y , int w , int h , int px , D3DCOLOR color );
	void BorderedBox( int x , int y , int w , int h , D3DCOLOR fillColor , D3DCOLOR borderColor );
	void DrawCornerBox( int x , int y , int w , int h , bool outLined , int indicatorH , int indicatorW , D3DCOLOR color );
	int GetWidth( );
	int GetHeight( );

	DXRenderer* GetRenderer( ) const;

	bool IsInitialized( );
	bool PlaceHook( std::function<void(void)> callBack );

private:		
	DXGIHook*					m_pDXGIHook;
	DXRenderer*					m_pRenderer;
	//DXVersion					m_DXVersion;
	bool						m_Initialized;

};

#endif