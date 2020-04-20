#ifndef _MEMORY_H_
#define _MEMORY_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <Windows.h>

#define INRANGE(x,a,b)		(x >= a && x <= b) 
#define getBits( x )		(INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte( x )		(getBits(x[0]) << 4 | getBits(x[1]))

namespace Memory
{
	uintptr_t FindHeapAddressWithVTable( uintptr_t vTableAdress , uintptr_t excludePtr = 0 );

	bool ReadMemory( PVOID lpBuffer , const PVOID lpSource , size_t bytes );
	bool WriteMemory( PVOID lpBuffer , const PVOID lpSource , size_t bytes );
	
	class MemoryPatch
	{
	public:
		MemoryPatch( );
		~MemoryPatch( );

		bool MemPatch( uintptr_t lpMem , PBYTE pSrc , DWORD len );
		bool Nop( uintptr_t lpMem , DWORD len );
		bool Restore( );

	private:
		uintptr_t		m_address;
		DWORD			m_length;
		unsigned char*  m_bytes;

		void SaveBytes( uintptr_t adress , DWORD length );
	};


	class InternalMemory
	{
	private:
		int filter( unsigned int code , struct _EXCEPTION_POINTERS* ep )
		{
			// Handle only AV
			return code == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
		}

	public:
		template <typename T>
		T Read( uintptr_t Address , const T& def_val = T( ) )
		{
			__try
			{
				return *static_cast<T*>( Address );
			}
			__except ( filter( GetExceptionCode( ) , GetExceptionInformation( ) ) )
			{
				return def_val;
			}
		}

		template <class T>
		BOOL Write( LPVOID lpvAddress , T tValue )
		{
			__try
			{
				*static_cast<T*>( lpvAddress ) = tValue;
			}
			__except ( filter( GetExceptionCode( ) , GetExceptionInformation( ) ) )
			{
				return false;
			}

			return TRUE;
		}

		template <class T>
		BOOL Add( LPVOID lpvAddress , T tValue )
		{
			__try
			{
				*static_cast<T*>( lpvAddress ) += tValue;
			}
			__except ( filter( GetExceptionCode( ) , GetExceptionInformation( ) ) )
			{
				return false;
			}

			return TRUE;
		}

		template <class T>
		BOOL Substract( LPVOID lpvAddress , T tValue )
		{
			__try
			{
				*static_cast<T*>( lpvAddress ) -= tValue;
			}
			__except ( filter( GetExceptionCode( ) , GetExceptionInformation( ) ) )
			{
				return false;
			}

			return TRUE;
		}

		template <class T>
		BOOL Multiply( LPVOID lpvAddress , T tValue )
		{
			__try
			{
				*static_cast<T*>( lpvAddress ) *= tValue;
			}
			__except ( filter( GetExceptionCode( ) , GetExceptionInformation( ) ) )
			{
				return false;
			}

			return TRUE;
		}

		template <class T>
		BOOL Divide( LPVOID lpvAddress , T tValue )
		{
			__try
			{
				*static_cast<T*>( lpvAddress ) /= tValue;
			}
			__except ( filter( GetExceptionCode( ) , GetExceptionInformation( ) ) )
			{
				return false;
			}

			return TRUE;
		}
	};
}




#endif