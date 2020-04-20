
//Credits: @ 2011  s0beit NeoIII 
#include "D3D11Rendering.h" 
#include "D3D11ShaderFx.h"

#pragma warning ( disable: 4102 ) 
#pragma warning ( disable: 4311 ) 
#pragma warning ( disable: 4312 ) 

#pragma warning ( disable: 4244 ) 
#pragma warning ( disable: 4996 ) 

Dx11Renderer::Dx11Renderer( ) 
{ 
	bIsInitialized = false;
    m_pDevice        = nullptr; 
    m_pDeviceContext= nullptr;    
    m_pEffect        = nullptr; 
    m_pTechnique    = nullptr; 
    m_pInputLayout    = nullptr; 
    m_pVertexBuffer = nullptr;     
} 

Dx11Renderer::~Dx11Renderer( ) 
{ 
	bIsInitialized = false;
    m_pDevice        = nullptr; 
    m_pDeviceContext= nullptr;    
    m_pEffect        = nullptr; 
    m_pTechnique    = nullptr; 
    m_pInputLayout    = nullptr; 
    m_pVertexBuffer = nullptr; 
} 

int Dx11Renderer::GetWidth( )
{
	D3D11_VIEWPORT vp;
	UINT nvp = 1;
	this->m_pDeviceContext->RSGetViewports( &nvp, &vp );
	return vp.Width;
}

int Dx11Renderer::GetHeight( )
{
	D3D11_VIEWPORT vp;
	UINT nvp = 1;
	this->m_pDeviceContext->RSGetViewports( &nvp, &vp );
	return vp.Height;
}

bool Dx11Renderer::IsInitialized( ) 
{ 
    return this->bIsInitialized; 
} 

bool Dx11Renderer::InitializeRenderClass( ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, char *DefaultFont ) 
{ 
    if( !pDevice || !pDeviceContext ) 
        return false; 

    if( this->bIsInitialized == true ) 
        return this->bIsInitialized; 
	
	this->m_pDevice                = pDevice; 
    this->m_pDeviceContext        = pDeviceContext; 

	typedef HRESULT(__stdcall* D3DCompile_t)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName, const D3D_SHADER_MACRO *pDefines, ID3DInclude *pInclude, LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob **ppCode, ID3DBlob *ppErrorMsgs);
	D3DCompile_t myD3DCompile = ( D3DCompile_t )GetProcAddress( GetD3DCompiler( ), "D3DCompile" );
	if (!myD3DCompile)
	{
		DEBUGOUT("Fail to get D3DCompile");	
		return false;
	}

	ID3D10Blob *compiledFX = nullptr, *errorMsgs = nullptr;
	HRESULT hr = myD3DCompile(shaderRaw, strlen(shaderRaw), "FillTechFx", nullptr, nullptr, "FillTech", "fx_5_0", NULL, NULL, &compiledFX, errorMsgs);
	if (FAILED(hr))
	{
		DEBUGOUT( "Fail to D3DX11CompileFromMemory" );
		return false;
	}
        
	hr = D3DX11CreateEffectFromMemory(compiledFX->GetBufferPointer( ), compiledFX->GetBufferSize( ), 0, this->m_pDevice, &m_pEffect);
	if (FAILED(hr))
	{		
		DEBUGOUT( "Fail to D3DX11CreateEffectFromMemory" );
		return false;
	}

	SAFE_RELEASE(compiledFX);
    m_pTechnique = m_pEffect->GetTechniqueByName( "FillTech" ); 

	if (m_pTechnique == nullptr)
	{
		DEBUGOUT( "m_pTechnique is nullptr" );
		return false;
	}

    D3D11_INPUT_ELEMENT_DESC lineRectLayout[] = 
    { 
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },   
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 } 
    }; 

    D3DX11_PASS_DESC passDesc; 

	if (FAILED(m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc)))
	{
		DEBUGOUT( "FAILED to GetDesc(&passDesc)" );
		return false;
	}
        

	if (FAILED(this->m_pDevice->CreateInputLayout(lineRectLayout, sizeof(lineRectLayout) / sizeof(lineRectLayout[0]), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout)))
	{
		DEBUGOUT( "FAILED to CreateInputLayout" );
		return false;
	}      

    D3D11_BUFFER_DESC bufferDesc; 

    bufferDesc.Usage            = D3D11_USAGE_DYNAMIC; 
    bufferDesc.ByteWidth        = MAX_VERTEX_COUNT * sizeof( COLOR_VERTEX ); 
    bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER; 
    bufferDesc.CPUAccessFlags    = D3D11_CPU_ACCESS_WRITE; 
    bufferDesc.MiscFlags        = 0; 

	if (FAILED(this->m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pVertexBuffer)))
	{
		DEBUGOUT( "FAILED to CreateBuffer" );
		return false;
	}      

	/*m_pSprite = new DxSprite( );
	if (!m_pSprite->Initialize(pDevice, pDeviceContext))
	{
		DEBUGOUT( "FAILED to Initialize DxSprite" );
		return false;
	}*/

	this->fontName = DefaultFont;
    this->bIsInitialized = true; 

    return true; 
} 

void Dx11Renderer::DrawFilledRect( int x, int y, int w, int h, DWORD color )
{ 
    int a = (color >> 24) & 0xff; 
    int r = (color >> 16) & 0xff; 
    int g = (color >> 8) & 0xff; 
    int b = (color) & 0xff; 
    this->DrawRectInternal( x, y, x + w, y + h, r, g, b, a ); 
} 

void Dx11Renderer::DrawLine( int x1, int y1, int x2, int y2, D3DCOLOR color )
{ 
    int a = (color >> 24) & 0xff; 
    int r = (color >> 16) & 0xff; 
    int g = (color >> 8) & 0xff; 
    int b = (color) & 0xff; 

    this->DrawLineInternal( x1, y1, x2, y2, r, g, b, a ); 
} 

void Dx11Renderer::DrawCircle( int x, int y, int radius, DWORD color/*ARGB*/ ) 
{ 
	int a = (color >> 24) & 0xff;
	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = (color) & 0xff;

    this->DrawCircleInternal( x, y, radius, r, g, b, a); 
} 

void Dx11Renderer::DrawFilledCircleInternal(int x0, int y0, int radius, int r, int g, int b, int a) //TODO
{		
	if (this->bIsInitialized == false)
		return;

	if (m_pDevice == nullptr || m_pDeviceContext == nullptr)
		return;

	const int NUMPOINTS = CIRCLE_NUMPOINTS;
	UINT viewportNumber = 1;

	D3D11_VIEWPORT vp;

	this->m_pDeviceContext->RSGetViewports(&viewportNumber, &vp);

	COLOR_VERTEX* v = nullptr;

	D3D11_MAPPED_SUBRESOURCE mapData;

	if (FAILED(this->m_pDeviceContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData)))
		return;

	v = (COLOR_VERTEX*)mapData.pData;

	float WedgeAngle = (float)((2 * D3DX_PI) / NUMPOINTS);

	for (int i = 0; i <= NUMPOINTS; i++)
	{
		float Theta = (float)(i * WedgeAngle);
		float x = (float)(x0 + radius * cos(Theta));
		float y = (float)(y0 - radius * sin(Theta));

		v[i].Position.x = 2.0f * (x - 0.5f) / vp.Width - 1.0f;
		v[i].Position.y = 1.0f - 2.0f * (y - 0.5f) / vp.Height;
		v[i].Position.z = 0.0f;
		v[i].Color.r = ((FLOAT)r / 255.0f);
		v[i].Color.g = ((FLOAT)g / 255.0f);
		v[i].Color.b = ((FLOAT)b / 255.0f);
		v[i].Color.a = ((FLOAT)a / 255.0f);
	}
	

	this->m_pDeviceContext->Unmap(m_pVertexBuffer, NULL);

	this->m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	UINT Stride = sizeof(COLOR_VERTEX);
	UINT Offset = 0;

	this->m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &Stride, &Offset);

	this->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		
	D3DX11_TECHNIQUE_DESC techDesc;

	if (FAILED(m_pTechnique->GetDesc(&techDesc)))
		return;

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, this->m_pDeviceContext);
		
		this->m_pDeviceContext->Draw(NUMPOINTS + 1, 0);
	}
}

void Dx11Renderer::GradientRectangle(int x, int y, int width, int height, DWORD startCol, DWORD endCol, bool vertical)
{
	if (this->bIsInitialized == false)
		return;

	if (m_pDevice == nullptr || m_pDeviceContext == nullptr)
		return;

	UINT viewportNumber = 1;

	D3D11_VIEWPORT vp2;

	m_pDeviceContext->RSGetViewports(&viewportNumber, &vp2);

	float xx0 = 2.0f * (x - 0.5f) / vp2.Width - 1.0f;
	float yy0 = 1.0f - 2.0f * (y - 0.5f) / vp2.Height;
	float xx1 = 2.0f * (x + width - 0.5f) / vp2.Width - 1.0f;
	float yy1 = 1.0f - 2.0f * (y + height - 0.5f) / vp2.Height;

	COLOR_VERTEX* v = nullptr;

	D3D11_MAPPED_SUBRESOURCE mapData;

	if (FAILED(m_pDeviceContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData)))
	{
		return;
	}

	v = (COLOR_VERTEX*)mapData.pData;

	v[0].Position.x = xx0;
	v[0].Position.y = yy0;
	v[0].Position.z = 0;
	v[0].Color = vertical ? startCol : endCol;

	v[1].Position.x = xx1;
	v[1].Position.y = yy0;
	v[1].Position.z = 0;
	v[1].Color = startCol;

	v[2].Position.x = xx0;
	v[2].Position.y = yy1;
	v[2].Position.z = 0;
	v[2].Color = endCol;

	v[3].Position.x = xx1;
	v[3].Position.y = yy1;
	v[3].Position.z = 0;
	v[3].Color = vertical ? endCol : startCol ;

	m_pDeviceContext->Unmap(m_pVertexBuffer, NULL);

	m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	UINT Stride = sizeof(COLOR_VERTEX);
	UINT Offset = 0;

	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &Stride, &Offset);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	D3DX11_TECHNIQUE_DESC techDesc;

	if (FAILED(m_pTechnique->GetDesc(&techDesc)))
		return;

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, m_pDeviceContext);

		m_pDeviceContext->Draw(4, 0);
	}
}

void Dx11Renderer::DrawText( int x, int y, int scale, D3DCOLOR color, char *szText )
{
	if (this->bIsInitialized == false)	
		return;

	if (m_pDevice == nullptr || m_pDeviceContext == nullptr)
		return;
	
	int index = scale;
	if (index < 0 || index > 63)
		return;
	
	if (!m_pDxFont[ index ].IsInitialized())
	{
		if (m_pDxFont[ index ].Initialize(m_pDevice, m_pDeviceContext))
		{
			std::wstring wstr(this->fontName, this->fontName + strlen(this->fontName));
			m_pDxFont[ index ].InitializeFont( wstr, scale, DxFont::STYLE_BOLD, 1 );
		}
		else
			return;
	}	
	
	int a = (color >> 24) & 0xff;
	int r = (color >> 16) & 0xff;
	int g = (color >> 8)  & 0xff;
	int b = (color)       & 0xff;		
	
	m_pDxFont[ index ].DrawString(x, y, szText, r, g, b, a);
}

void Dx11Renderer::DrawRectInternal( int x0, int y0, int x1, int y1, int r, int g, int b, int a ) 
{ 
    if( this->bIsInitialized == false ) 
        return; 

    if( m_pDevice == nullptr || m_pDeviceContext == nullptr )  
        return; 

    UINT viewportNumber = 1; 

    D3D11_VIEWPORT vp; 

    this->m_pDeviceContext->RSGetViewports( &viewportNumber, &vp ); 

    float xx0 = 2.0f * ( x0 - 0.5f ) / vp.Width - 1.0f; 
    float yy0 = 1.0f - 2.0f * ( y0 - 0.5f ) / vp.Height; 
    float xx1 = 2.0f * ( x1 - 0.5f ) / vp.Width - 1.0f; 
    float yy1 = 1.0f - 2.0f * ( y1 - 0.5f ) / vp.Height; 

    COLOR_VERTEX* v = nullptr; 

    D3D11_MAPPED_SUBRESOURCE mapData; 

	if (FAILED(this->m_pDeviceContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData)))
        return; 

    v = ( COLOR_VERTEX* ) mapData.pData; 

    v[0].Position.x = xx0; 
    v[0].Position.y = yy0; 
    v[0].Position.z = 0; 
    v[0].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[0].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[0].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[0].Color.a = ( ( FLOAT ) a / 255.0f ); 

    v[1].Position.x = xx1; 
    v[1].Position.y = yy0; 
    v[1].Position.z = 0; 
    v[1].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[1].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[1].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[1].Color.a = ( ( FLOAT ) a / 255.0f ); 

    v[2].Position.x = xx0; 
    v[2].Position.y = yy1; 
    v[2].Position.z = 0; 
    v[2].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[2].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[2].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[2].Color.a = ( ( FLOAT ) a / 255.0f ); 

    v[3].Position.x = xx1; 
    v[3].Position.y = yy1; 
    v[3].Position.z = 0; 
    v[3].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[3].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[3].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[3].Color.a = ( ( FLOAT ) a / 255.0f ); 

    this->m_pDeviceContext->Unmap( m_pVertexBuffer, NULL ); 

    this->m_pDeviceContext->IASetInputLayout( m_pInputLayout ); 

    UINT Stride = sizeof( COLOR_VERTEX ); 
    UINT Offset = 0; 

    this->m_pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &Stride, &Offset ); 

    this->m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP ); 

    D3DX11_TECHNIQUE_DESC techDesc; 

    if( FAILED( m_pTechnique->GetDesc( &techDesc ) ) ) 
        return; 	

    for( UINT p = 0; p < techDesc.Passes; ++p ) 
    { 
        m_pTechnique->GetPassByIndex( p )->Apply( 0, this->m_pDeviceContext ); 

        this->m_pDeviceContext->Draw( 4, 0 ); 
    } 
}  

void Dx11Renderer::DrawTriangle( float x0 , float y0 , float x1 , float y1 , float x2 , float y2 , D3DCOLOR color )
{
	if ( this->bIsInitialized == false )
		return;

	if ( m_pDevice == nullptr || m_pDeviceContext == nullptr )
		return;

	int a = ( color >> 24 ) & 0xff;
	int r = ( color >> 16 ) & 0xff;
	int g = ( color >> 8 ) & 0xff;
	int b = ( color )& 0xff;

	

}

void Dx11Renderer::DrawLineInternal( int x0, int y0, int x1, int y1, int r, int g, int b, int a ) 
{ 
    if( this->bIsInitialized == false ) 
        return; 

    if( m_pDevice == nullptr || m_pDeviceContext == nullptr )  
        return; 

    UINT viewportNumber = 1; 

    D3D11_VIEWPORT vp; 

    this->m_pDeviceContext->RSGetViewports( &viewportNumber, &vp ); 

    float xx0 = 2.0f * ( x0 - 0.5f ) / vp.Width - 1.0f; 
    float yy0 = 1.0f - 2.0f * ( y0 - 0.5f ) / vp.Height; 
    float xx1 = 2.0f * ( x1 - 0.5f ) / vp.Width - 1.0f; 
    float yy1 = 1.0f - 2.0f * ( y1 - 0.5f ) / vp.Height; 

	COLOR_VERTEX* v = nullptr;

    D3D11_MAPPED_SUBRESOURCE mapData; 

	if (FAILED(this->m_pDeviceContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData)))
        return; 

    v = ( COLOR_VERTEX* ) mapData.pData; 

    v[0].Position.x = xx0; 
    v[0].Position.y = yy0; 
    v[0].Position.z = 0; 
    v[0].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[0].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[0].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[0].Color.a = ( ( FLOAT ) a / 255.0f ); 

    v[1].Position.x = xx1; 
    v[1].Position.y = yy1; 
    v[1].Position.z = 0; 
    v[1].Color.r = ( ( FLOAT ) r / 255.0f ); 
    v[1].Color.g = ( ( FLOAT ) g / 255.0f ); 
    v[1].Color.b = ( ( FLOAT ) b / 255.0f ); 
    v[1].Color.a = ( ( FLOAT ) a / 255.0f ); 
	
	this->m_pDeviceContext->Unmap(m_pVertexBuffer, NULL);

    this->m_pDeviceContext->IASetInputLayout( m_pInputLayout ); 

    UINT Stride = sizeof( COLOR_VERTEX ); 
    UINT Offset = 0; 

    this->m_pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &Stride, &Offset ); 

    this->m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP ); 

	D3DX11_TECHNIQUE_DESC techDesc; 

    if( FAILED( m_pTechnique->GetDesc( &techDesc ) ) ) 
        return; 
	
    for( UINT p = 0; p < techDesc.Passes; ++p ) 
    { 
        m_pTechnique->GetPassByIndex( p )->Apply( 0, this->m_pDeviceContext ); 

        this->m_pDeviceContext->Draw( 2, 0 ); 
    } 
} 

void Dx11Renderer::RenderSprite(ID3D11ShaderResourceView * texSRV, const CD3D11_RECT destinationRect, const CD3D11_RECT sourceRect, XMCOLOR c) 
{
	if (this->bIsInitialized == false)	
		return;

	if (this->m_pSprite == false )
		return;

	m_pSprite->BeginBatch(texSRV); // Not efficient for multi sprites

	m_pSprite->Draw(destinationRect, sourceRect, c);

	m_pSprite->EndBatch( );
}

void Dx11Renderer::DrawCircleInternal( int x0, int y0, int radius, int r, int g, int b, int a) 
{ 
	if ( this->bIsInitialized == false)
		return;

	if ( m_pDevice == nullptr || m_pDeviceContext == nullptr )
		return;

	const int NUMPOINTS = CIRCLE_NUMPOINTS;
	UINT viewportNumber = 1;

	D3D11_VIEWPORT vp;

	this->m_pDeviceContext->RSGetViewports(&viewportNumber, &vp);

	COLOR_VERTEX* v = nullptr;

	D3D11_MAPPED_SUBRESOURCE mapData;

	if (FAILED(this->m_pDeviceContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapData)))
		return;

	v = (COLOR_VERTEX*)mapData.pData;

	float WedgeAngle = (float)((2 * D3DX_PI) / NUMPOINTS);

	for (int i = 0; i <= NUMPOINTS; i++)
	{
		float Theta = (float)(i * WedgeAngle);
		float x = (float)(x0 + radius * cos(Theta));
		float y = (float)(y0 - radius * sin(Theta));

		v[i].Position.x = 2.0f * (x - 0.5f) / vp.Width - 1.0f;
		v[i].Position.y = 1.0f - 2.0f * (y - 0.5f) / vp.Height;
		v[i].Position.z = 0.0f;
		v[i].Color.r = ((FLOAT)r / 255.0f);
		v[i].Color.g = ((FLOAT)g / 255.0f);
		v[i].Color.b = ((FLOAT)b / 255.0f);
		v[i].Color.a = ((FLOAT)a / 255.0f);
	}

	this->m_pDeviceContext->Unmap(m_pVertexBuffer, NULL);

	this->m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	UINT Stride = sizeof(COLOR_VERTEX);
	UINT Offset = 0;

	this->m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &Stride, &Offset);
	this->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	//this->m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP ); 

	D3DX11_TECHNIQUE_DESC techDesc;

	if (FAILED(m_pTechnique->GetDesc(&techDesc)))
		return;
	
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, this->m_pDeviceContext);

		this->m_pDeviceContext->Draw(NUMPOINTS + 1, 0);
	}
}  

