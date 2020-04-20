#ifndef DX11_RENDERER_H 
#define DX11_RENDERER_H 

#ifdef _MSC_VER
#pragma once
#endif

#include <assert.h>
#include <windows.h>

#include "..\DxRenderer.h"

struct COLOR_VERTEX 
{ 
    D3DXVECTOR3 Position; 
    D3DXCOLOR    Color; 
};

#define CIRCLE_NUMPOINTS 30 
#define MAX_VERTEX_COUNT ( CIRCLE_NUMPOINTS + 1 ) 

class DxFont;
class DxSprite;
class Dx11Renderer;

class DxSprite
{
public:

	ID3D11Device *            DxDev;
	ID3D11DeviceContext *    DevCtx;

	DxSprite( );
	~DxSprite( );

	bool    Initialize(ID3D11Device * DxDev2, ID3D11DeviceContext * DevCtx2);
	void    BeginBatch(ID3D11ShaderResourceView * texSRV);
	void    EndBatch( );
	void	Draw(const CD3D11_RECT & destinationRect, const CD3D11_RECT & sourceRect, XMCOLOR color, float rotation = 0.0f, float scale = 1.0f);
	void    DrawString(int X, int Y, const std::wstring & text, int R, int G, int B, int A, DxFont  &F);

private:

	struct Sprite
	{
		Sprite( ) :
		Color(1.0f, 1.0f, 1.0f, 1.0f),
		Z(0.0f),
		Angle(0.0f),
		Scale(1.0f)
		{
		}

		CD3D11_RECT SrcRect;
		CD3D11_RECT DestRect;
		XMCOLOR        Color;
		float        Z;
		float        Angle;
		float        Scale;
	};

	struct SpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
		XMCOLOR  Color;
	};

	void        DrawBatch(UINT startSpriteIndex, UINT spriteCount);
	XMFLOAT3    PointToNdc(int x, int y, float z);
	void        BuildSpriteQuad(const Sprite & sprite, SpriteVertex v[4]);

	bool                                    Initialized;
	ID3D11BlendState *                        TransparentBS;
	ID3DX11EffectTechnique *                SpriteTech;
	ID3DX11EffectShaderResourceVariable *    SpriteMap;
	ID3D11ShaderResourceView *                BatchTexSRV;
	ID3D11InputLayout *                        InputLayout;
	ID3D11Buffer *                            VB, *IB;
	UINT                                    TexWidth, TexHeight;
	std::vector<Sprite>                            SpriteList;
	float                                    ScreenWidth, ScreenHeight;
};


class DxFont
{
public:

	DxSprite               D3DSprite;
	ID3D11Device *            DxDev;
	ID3D11DeviceContext *    DevCtx;

	bool Initialize(ID3D11Device * Device, ID3D11DeviceContext * Context);
	void DrawString(int X, int Y, const std::wstring & text, int R, int G, int B, int A);
	void DrawString(int X, int Y, char * txt, int R, int G, int B, int A);

	DxFont( );
	~DxFont( );

	bool						IsInitialized( );
	bool                        InitializeFont(const std::wstring & FontName, float FontSize, WORD FontStyle, bool AntiAliased);
	ID3D11ShaderResourceView *    GetFontSheetSRV( );
	const CD3D11_RECT &            GetCharRect(WCHAR c);
	int                            GetSpaceWidth( );
	int                            GetCharHeight( );
	int                            GetStrLen(char* str);

	enum
	{
		STYLE_NORMAL = 0,
		STYLE_BOLD = 1,
		STYLE_ITALIC = 2,
		STYLE_BOLD_ITALIC = 3,
		STYLE_UNDERLINE = 4,
		STYLE_STRIKEOUT = 8
	};

private:

	void    MeasureChars(Gdiplus::Font & font, Gdiplus::Graphics & charGraphics);
	void    BuildFontSheetBitmap(Gdiplus::Font & font, Gdiplus::Graphics & charGraphics, Gdiplus::Bitmap & charBitmap, Gdiplus::Graphics & fontSheetGraphics);
	bool    BuildFontSheetTexture(Gdiplus::Bitmap & fontSheetBitmap);
	int        GetCharMinX(Gdiplus::Bitmap & charBitmap);
	int        GetCharMaxX(Gdiplus::Bitmap & charBitmap);
	int        GetEncoderClsid(const WCHAR * format, CLSID * pClsid);

	bool                        Initialized;
	static const WCHAR            StartChar = 33;
	static const WCHAR            EndChar = 127;
	static const UINT            NumChars = EndChar - StartChar;
	ID3D11ShaderResourceView *    FontSheetSRV;
	ID3D11Texture2D *            FontSheetTex;
	UINT                        TexWidth, TexHeight;
	CD3D11_RECT                    CharRects[NumChars];
	int                            SpaceWidth, CharHeight;
};


class Dx11Renderer : public DXRenderer
{ 

public: 
    Dx11Renderer( ); 
    ~Dx11Renderer( ); 
	
	bool IsInitialized( );
    bool InitializeRenderClass( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, char *DefaultFont = "Arial"); 
	void DrawFilledRect( int x, int y, int w, int h, DWORD color/*ARGB*/ );
	void DrawLine( int x1, int y1, int x2, int y2, D3DCOLOR color );
    void DrawCircle( int x, int y, int radius, DWORD color/*ARGB*/ ); 
	void DrawText( int x, int y, int scale, D3DCOLOR color, char *szText );

	void DrawTriangle( float x0 , float y0 , float x1 , float y1 , float x2 , float y2 , D3DCOLOR color );

	void GradientRectangle(int x, int y, int width, int height, DWORD startCol, DWORD endCol, bool vertical = true);   	
	void RenderSprite(ID3D11ShaderResourceView * texSRV, const CD3D11_RECT destinationRect, const CD3D11_RECT sourceRect, XMCOLOR c);
		
	int GetWidth( );
	int GetHeight( );


private: 
    void DrawLineInternal( int x0, int y0, int x1, int y1, int r, int g, int b, int a ); 
    void DrawCircleInternal( int x0, int y0, int radius, int r, int g, int b, int a); 
	void DrawFilledCircleInternal( int x0, int y0, int radius, int r, int g, int b, int a );
	void DrawRectInternal( int x0 , int y0 , int x1 , int y1 , int r , int g , int b , int a );
	

    ID3D11Device*            m_pDevice; 
    ID3D11DeviceContext*    m_pDeviceContext; 
    ID3DX11Effect*            m_pEffect; 
    ID3DX11EffectTechnique* m_pTechnique; 
    ID3D11InputLayout*        m_pInputLayout; 
    ID3D11Buffer*            m_pVertexBuffer; 
    bool                    bIsInitialized; 
	char*					fontName;

	DxFont m_pDxFont[64]; //size to 0 from 64
	DxSprite* m_pSprite = nullptr;
}; 


#endif 

