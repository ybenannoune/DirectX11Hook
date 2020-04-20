#include "InternalMemory.h"

namespace Memory
{			
	uintptr_t FindHeapAddressWithVTable( uintptr_t vTableAdress , uintptr_t excludePtr )
	{
#ifdef _AMD64_
#define _PTR_MAX_VALUE 0x7FFFFFFEFFFF
		MEMORY_BASIC_INFORMATION64 mbi = { 0 };
#else
#define _PTR_MAX_VALUE 0xFFE00000
		MEMORY_BASIC_INFORMATION32 mbi = { 0 };
#endif
		uintptr_t dwVTableAddress = 0;

		for ( uintptr_t memptr = 0x10000; memptr < _PTR_MAX_VALUE; memptr = mbi.BaseAddress + mbi.RegionSize )
		{
			if ( !VirtualQuery( reinterpret_cast< LPCVOID >( memptr ) , reinterpret_cast< PMEMORY_BASIC_INFORMATION >( &mbi ) , sizeof( MEMORY_BASIC_INFORMATION ) ) )
				continue;

			if ( mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS || ( mbi.Protect & PAGE_GUARD ) )
				continue;

			uintptr_t len = mbi.BaseAddress + mbi.RegionSize;

			for ( uintptr_t current = static_cast< uintptr_t >( mbi.BaseAddress ); current < len; ++current )
			{
				__try
				{
					dwVTableAddress = *reinterpret_cast< uintptr_t* >( current );
				}
				__except ( 1 )
				{
					continue;
				}

				if ( dwVTableAddress == vTableAdress )
				{
					if ( current == excludePtr )
						continue;
							
					return current;
				}
			}
		}
		return NULL;
	}
	
	bool ReadMemory( PVOID lpBuffer , const PVOID lpSource , size_t bytes )
	{
		DWORD dwProtection;

		if ( !VirtualProtect( lpBuffer , bytes , PAGE_EXECUTE_READWRITE , &dwProtection ) )
			return false;

		memcpy( lpBuffer , lpSource , bytes );

		VirtualProtect( lpBuffer , bytes , dwProtection , &dwProtection );

		return true;
	}

	bool WriteMemory( PVOID lpBuffer , const PVOID lpSource , size_t bytes )
	{
		DWORD dwProtection;

		if ( !VirtualProtect( lpBuffer , bytes , PAGE_EXECUTE_READWRITE , &dwProtection ) )
			return false;

		memcpy( lpBuffer , lpSource , bytes );

		VirtualProtect( lpBuffer , bytes , dwProtection , &dwProtection );

		return true;
	}


	MemoryPatch::MemoryPatch( )
	{
		this->m_length = 0;
		this->m_address = 0;
		this->m_bytes = nullptr;
	}

	MemoryPatch::~MemoryPatch( )
	{
		if ( m_bytes )
		{
			delete m_bytes;
		}
	}

	bool MemoryPatch::MemPatch( uintptr_t lpMem , PBYTE pSrc , DWORD len )
	{
		DWORD lpflOldProtect = NULL;
		unsigned char* pDst = reinterpret_cast< unsigned char* >( lpMem );

		// if bytes are not saved
		if ( m_length == 0 )
		{
			SaveBytes( lpMem , len );
		}

		if ( VirtualProtect( reinterpret_cast< LPVOID >( lpMem ) , len , PAGE_EXECUTE_READWRITE , &lpflOldProtect ) )
		{
			while ( len-- > 0 )
				*pDst++ = *pSrc++;

			VirtualProtect( reinterpret_cast< LPVOID >( lpMem ) , len , lpflOldProtect , &lpflOldProtect );
			return true;
		}

		return false;
	}

	bool MemoryPatch::Nop( uintptr_t lpMem , DWORD len )
	{
		DWORD lpflOldProtect = NULL;
		unsigned char* pDst = reinterpret_cast< unsigned char* >( lpMem );
		PBYTE pSrc = reinterpret_cast< PBYTE >( "\x90" );
		
		// if bytes are not saved
		if ( m_length == 0 )
		{
			SaveBytes( lpMem , len );
		}

		if ( VirtualProtect( reinterpret_cast< LPVOID >( lpMem ) , len , PAGE_EXECUTE_READWRITE , &lpflOldProtect ) )
		{
			while ( len-- > 0 )
				*pDst++ = *pSrc;

			VirtualProtect( reinterpret_cast< LPVOID >( lpMem ) , len , lpflOldProtect , &lpflOldProtect );
			return true;
		}

		return false;
	}

	bool MemoryPatch::Restore( )
	{
		if ( m_address && m_bytes && m_length > 0 )
		{
			MemPatch( m_address , m_bytes , m_length );

			this->m_length = 0;
			this->m_address = 0;

			if ( m_bytes )
			{
				delete m_bytes;
				m_bytes = nullptr;
			}

			return true;
		}
		return false;
	}


	void MemoryPatch::SaveBytes( uintptr_t adress , DWORD length )
	{
		//Save address and bytes numbers
		this->m_address = adress;
		this->m_length = length;

		if ( m_bytes )
		{
			delete m_bytes;
			m_bytes = nullptr;
		}

		this->m_bytes = new unsigned char[m_length + 1];
		for ( unsigned int i = 0; i < m_length; i++ )
		{
			m_bytes[i] = *reinterpret_cast< unsigned char* >( m_address + i );
		}
	}
}

