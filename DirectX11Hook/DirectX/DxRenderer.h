#ifndef __DXRENDERER_H__
#define __DXRENDERER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "..\Util\Common.h"
#include <vector>

#include "DxColors.h"

#include <d3d9types.h>
#include <d3dx9math.h>
#include <d3d9.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx11effect.h>
#include <D3DX11async.h> 
#include <D3D11Shader.h> 
#include "xnamath.h"

#pragma comment(lib, "Effects11.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3d11.lib")

#include "Gdiplus.h"
#pragma comment (lib, "Gdiplus.lib")

enum DXVersion
{
	DXVersion_Unknow,	
	DXVersion_D3D9,
	DXVersion_D3D10,
	DXVersion_D3D11
};

static HMODULE GetD3DCompiler( )
{
	char buf[32];

	for ( int i = 50; i >= 30; i-- )
	{
		sprintf_s( buf , "D3DCompiler_%d.dll" , i );
		HMODULE mod = LoadLibrary( buf );
		if ( mod )
			return mod;
	}

	return nullptr;
}

class DXRenderer 
{
public:	
	virtual bool IsInitialized( ) = 0;
	virtual void DrawLine( int x1, int y1, int x2, int y2, D3DCOLOR color ) = 0;
	virtual void DrawText( int x, int y, int scale, D3DCOLOR color, char *szText ) = 0;
	virtual void DrawCircle( int x, int y, int radius, D3DCOLOR color ) = 0;
	virtual void DrawFilledRect( int x, int y, int w, int h, D3DCOLOR color ) = 0;	
	virtual int  GetWidth( ) = 0;
	virtual int  GetHeight( ) = 0;
};

#endif