#include "HookLib.h"
#include "..\Log.h"

namespace HookLib
{
#ifdef _AMD64_
	void* DetourCreate( void* src, void* tgt, const int length, HkContext* context )
	{
		// Find free region for allocation
		MEMORY_BASIC_INFORMATION mbi = { 0 };
		for (uintptr_t addr = (uintptr_t)src;
			addr > (uintptr_t)src - 0x80000000;
			addr = (uintptr_t)mbi.BaseAddress - 1)
		{
			if (VirtualQuery((LPCVOID)addr, &mbi, sizeof(mbi)) != sizeof(MEMORY_BASIC_INFORMATION))
				break;

			if (mbi.State == MEM_FREE)
			{
				context = (HkContext*)VirtualAlloc(mbi.BaseAddress, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

				// Failed to allocate
				if ( !context )
					return 0;

				break;
			}
		}
		
		// Save original. Length check missing
		memcpy( context->original_code, src, length + 1 );

		context->original_code[ length + 1 ] = 0xE9;
		*( uint32_t* )( context->original_code + length + 2 ) = ( uint32_t )( ( uintptr_t )src - ( uintptr_t )context->original_code - 5 );			

		// Build jump (jmp &#91rip - offset&#93)
		context->far_jmp[0] = 0xFF;
		context->far_jmp[1] = 0x25;
		*(uint32_t*)(context->far_jmp + 2) = (uint32_t)((uintptr_t)context - (uintptr_t)src + FIELD_OFFSET(HkContext, dst_ptr) - 6);

		// final jump address
		context->dst_ptr = (uint64_t)tgt;

		// write hook
		DWORD flOld = 0;
		VirtualProtect(src, 6, PAGE_EXECUTE_READWRITE, &flOld);
		memcpy(src, context->far_jmp, sizeof(context->far_jmp));
		VirtualProtect(src, 6, flOld, &flOld);

		DEBUGOUT( "[LOG] DetourCreate | source : 0x%X  target : 0x%X  original_backup : 0x%X " , src , tgt , &context->original_code );

		return &context->original_code;
	}
#else
	void *DetourCreate(BYTE *src, const BYTE *dst, const int len)
	{
		BYTE *jmp;
		DWORD dwback;
		DWORD jumpto, newjump;

		VirtualProtect(src, len, PAGE_READWRITE, &dwback);

		if (src[0] == 0xE9)
		{
			jmp = (BYTE*)malloc(10);
			jumpto = (*(DWORD*)(src + 1)) + ((DWORD)src) + 5;
			newjump = (jumpto - (DWORD)(jmp + 5));
			jmp[0] = 0xE9;
			*(DWORD*)(jmp + 1) = newjump;
			jmp += 5;
			jmp[0] = 0xE9;
			*(DWORD*)(jmp + 1) = (DWORD)(src - jmp);
		}
		else
		{
			jmp = (BYTE*)malloc(5 + len);
			memcpy(jmp, src, len);
			jmp += len;
			jmp[0] = 0xE9;
			*(DWORD*)(jmp + 1) = (DWORD)(src + len - jmp) - 5;
		}

		src[0] = 0xE9;
		*(DWORD*)(src + 1) = (DWORD)(dst - src) - 5;

		for (int i = 5; i < len; i++)
			src[i] = 0x90;

		VirtualProtect(src, len, dwback, &dwback);

		DEBUGOUT( "[LOG] DetourCreate | source : 0x%X  target : 0x%X  original_backup : 0x%X " , src , dst , (jmp - len) );

		return (jmp - len);
	}

#endif
	
	PVOID DetourVTable( void** vTable, int index, void* detour )
	{
		PVOID ret = vTable[ index ];

		DWORD old;
		VirtualProtect( &( vTable[ index ] ), sizeof( PVOID ), PAGE_EXECUTE_READWRITE, &old );
		vTable[ index ] = detour;
		VirtualProtect( &( vTable[ index ] ), sizeof( PVOID ), old, &old );

		DEBUGOUT( "[LOG] DetourVTable | source : 0x%X  target : 0x%X  index : %d  original : 0x%X" , vTable , detour , index, ret);

		return ret;
	}

	CVMTHook::CVMTHook(void)
		{
			memset(this, 0, sizeof(CVMTHook));
		}

	CVMTHook::CVMTHook(PDWORD_PTR* ppdwClassBase)
		{
			Initialize(ppdwClassBase);
		}

	CVMTHook::~CVMTHook(void)
		{
			UnHook( );
			if (m_pdwNewVMT)
			{
				delete[] m_pdwNewVMT;
				m_pdwNewVMT = nullptr;
			}
		}
	
	bool CVMTHook::Initialize(PDWORD_PTR* ppdwClassBase)
		{
			m_ppdwClassBase = ppdwClassBase;
			m_pdwOldVMT = *ppdwClassBase;
			m_dwVMTSize = GetVMTCount(*ppdwClassBase);
			m_pdwNewVMT = new DWORD_PTR[m_dwVMTSize];
			memcpy(m_pdwNewVMT, m_pdwOldVMT, sizeof(DWORD_PTR)*m_dwVMTSize);
			*ppdwClassBase = m_pdwNewVMT;
			return true;
		}

	bool CVMTHook::Initialize(PDWORD_PTR** pppdwClassBase)
		{
			return Initialize(*pppdwClassBase);
		}

	void CVMTHook::UnHook( )
	{
		if ( m_ppdwClassBase )
		{
			*m_ppdwClassBase = m_pdwOldVMT;
		}
	}

	void CVMTHook::UnHook( int index )
	{
		if ( m_ppdwClassBase )
		{
			*m_ppdwClassBase[ index ] = m_pdwOldVMT[ index ];
		}
	}

	void CVMTHook::ReHook( )
	{
		if ( m_ppdwClassBase )
		{
			*m_ppdwClassBase = m_pdwNewVMT;
		}
	}

	void CVMTHook::ReHook( int index )
	{
		if ( m_ppdwClassBase )
		{
			*m_ppdwClassBase[ index ] = m_pdwNewVMT[ index ];
		}
	}

	bool CVMTHook::UnHook( DWORD_PTR oldFunction, unsigned int iIndex )
	{
		if ( m_pdwNewVMT && m_pdwOldVMT && iIndex <= m_dwVMTSize&&iIndex >= 0 )
		{
			m_pdwNewVMT[ iIndex ] = oldFunction;
			return true;
		}

		return false;
	}

	int CVMTHook::GetFuncCount( void )
	{
		return static_cast< int >( m_dwVMTSize );
	}

	DWORD_PTR CVMTHook::GetMethodAddress( int Index )
	{
		if ( Index >= 0 && Index <= static_cast< int >( m_dwVMTSize ) && m_pdwOldVMT != 0 )
		{
			oFunction = m_pdwOldVMT[ Index ];
			return m_pdwOldVMT[ Index ];
		}

		return 0;
	}

	PDWORD_PTR CVMTHook::GetOldVMT( void )
	{
		return m_pdwOldVMT;
	}

	DWORD_PTR CVMTHook::HookMethod( DWORD_PTR dwNewFunc, unsigned int iIndex )
	{
		if ( m_pdwNewVMT && m_pdwOldVMT && iIndex <= m_dwVMTSize&&iIndex >= 0 )
		{			
			m_pdwNewVMT[ iIndex ] = dwNewFunc;			
			DEBUGOUT( "[LOG] HookMethod | New : 0x%X Old : 0x%X " , m_pdwNewVMT[iIndex] , m_pdwOldVMT[iIndex] );
			return m_pdwOldVMT[ iIndex ];
		}
		return 0;
	}

	DWORD_PTR CVMTHook::GetVMTCount(PDWORD_PTR pdwVMT)
	{
		DWORD dwIndex = 0;
		for ( dwIndex = 0; pdwVMT[dwIndex]; dwIndex++ )
		{		
			if ( IsBadCodePtr ( reinterpret_cast<FARPROC >(pdwVMT[dwIndex]) ) )
				break;
		}
		return dwIndex;
	}	
}