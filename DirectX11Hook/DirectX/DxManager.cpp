#include "DxManager.h"

DXManager::DXManager( )
{	  	
	m_Initialized = false;
	m_pRenderer = nullptr;
	m_pDXGIHook = nullptr;
}

DXManager::~DXManager( )
{
	m_Initialized = false;	
	SAFE_DELETE( m_pRenderer );			
	SAFE_DELETE( m_pDXGIHook );	
}

bool DXManager::PlaceHook( std::function<void(void)> callBack )
{
	if ( m_Initialized == true )
		return false;

	m_pDXGIHook = new DXGIHook();
	m_pDXGIHook->SetCallBack(callBack);

	if (m_pDXGIHook->HookDXGI())
	{
		m_pRenderer = new Dx11Renderer();
		m_Initialized = reinterpret_cast<Dx11Renderer*>(m_pRenderer)->InitializeRenderClass(m_pDXGIHook->GetD11Device(), m_pDXGIHook->GetContext());
		return m_Initialized;
	}

	return false;
}

bool DXManager::IsInitialized( )
{
	return m_Initialized;
}

void DXManager::DrawLine( int x1 , int y1 , int x2 , int y2 , D3DCOLOR color )
{
	if ( m_pRenderer )
	{
		m_pRenderer->DrawLine( x1 , y1 , x2 , y2 , color );
	}
}

void DXManager::DrawText( int x , int y , int scale , bool outLined , D3DCOLOR color , char *szText , ... )
{
	if ( m_pRenderer )
	{
		if ( !szText || strlen( szText ) > 255 )
			return;

		va_list va_alist;
		char logbuf[256];

		va_start( va_alist , szText );
		_vsnprintf_s( logbuf , sizeof( logbuf ) , szText , va_alist );
		va_end( va_alist );

		if ( outLined )
		{
			m_pRenderer->DrawText( x - 1, y - 1, scale , DxColors::Black , szText );
			m_pRenderer->DrawText( x , y , scale , color , szText );
		}
		else
		{
			m_pRenderer->DrawText( x , y , scale , color , szText );
		}		
	}
}

void DXManager::DrawCircle( int x , int y , int radius , D3DCOLOR color )
{
	if ( m_pRenderer )
	{
		m_pRenderer->DrawCircle( x , y , radius , color );
	}
}

void DXManager::DrawFilledRect( int x , int y , int w , int h , D3DCOLOR color )
{
	if ( m_pRenderer )
	{
		m_pRenderer->DrawFilledRect( x , y , w , h, color );
	}
}

void DXManager::DrawRect( int x , int y , int w , int h , int px , D3DCOLOR color )
{
	if ( m_pRenderer )
	{
		DrawFilledRect( x , ( y + h - px ) , w , px , color );
		DrawFilledRect( x , y , px , h , color );
		DrawFilledRect( x , y , w , px , color );
		DrawFilledRect( ( x + w - px ) , y , px , h , color );
	}
}
void DXManager::BorderedBox( int x , int y , int w , int h , D3DCOLOR fillColor , D3DCOLOR borderColor )
{
	if ( m_pRenderer )
	{
		DrawFilledRect( x , y , w , h , fillColor );
		DrawRect( x , y , w , h , 1 , borderColor );
	}
}
void DXManager::DrawCornerBox( int x , int y , int w , int h , bool outLined , int indicatorH , int indicatorW , D3DCOLOR color )
{
	if ( m_pRenderer && indicatorH && indicatorW )
	{
		struct Corner_t
		{
			int x , y , w , h;
		}Corner[8];

		Corner[0].x = x;
		Corner[0].y = y;
		Corner[0].w = w / indicatorW;
		Corner[0].h = 1;

		Corner[1].x = x + w - ( w / indicatorW );
		Corner[1].y = y;
		Corner[1].w = w / indicatorW;
		Corner[1].h = 1;

		Corner[2].x = x;
		Corner[2].y = y + h;
		Corner[2].w = w / indicatorW;
		Corner[2].h = 1;

		Corner[3].x = x + w - ( w / indicatorW );
		Corner[3].y = y + h;
		Corner[3].w = w / indicatorW;
		Corner[3].h = 1;


		Corner[4].x = x;
		Corner[4].y = y;
		Corner[4].w = 1;
		Corner[4].h = h / indicatorH;

		Corner[5].x = x + w - 1;
		Corner[5].y = y;
		Corner[5].w = 1;
		Corner[5].h = h / indicatorH;

		Corner[6].x = x;
		Corner[6].y = y + h - ( h / indicatorH );
		Corner[6].w = 1;
		Corner[6].h = h / indicatorH;

		Corner[7].x = x + w - 1;
		Corner[7].y = y + h - ( h / indicatorH ) + 1;
		Corner[7].w = 1;
		Corner[7].h = h / indicatorH;

		if ( outLined == true )
		{
			for ( int i = 0; i < 8; i++ )
			{
				DrawFilledRect( Corner[i].x - 1 , Corner[i].y - 1 , Corner[i].w + 2 , Corner[i].h + 2 , D3DCOLOR_ARGB( 255 , 0 , 0 , 0 ) );
				DrawFilledRect( Corner[i].x + 1 , Corner[i].y + 1 , Corner[i].w - 2 , Corner[i].h - 2 , D3DCOLOR_ARGB( 255 , 0 , 0 , 0 ) );
			}
		}

		for ( int i = 0; i < 8; i++ )
		{
			DrawFilledRect( Corner[i].x , Corner[i].y , Corner[i].w , Corner[i].h , color );
		}
	}
}


DXRenderer* DXManager::GetRenderer( ) const
{
	return m_pRenderer;
}

int DXManager::GetWidth( )
{
	if ( m_pRenderer )
	{
		return m_pRenderer->GetWidth( );
	}
	else
	{
		return 0;
	}
}

int DXManager::GetHeight( )
{
	if ( m_pRenderer )
	{
		return m_pRenderer->GetHeight( );
	}
	else
	{
		return 0;
	}
}

