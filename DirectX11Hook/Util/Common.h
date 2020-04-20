#ifndef _MACRO_H_
#define _MACRO_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <iostream>
#include <wtypes.h>
#include <Psapi.h>

#include "Log.h"
#include "Memory\Exceptions.h"
#include "Memory\InternalMemory.h"

#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define PAD(SIZE) BYTE MACRO_CONCAT(_pad, __COUNTER__)[SIZE];
#define SAFECALL(func) if(func) func( );

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if((p)) { (p)->Release(); (p) = NULL; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(a) { if((a)) { delete (a); (a) = NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(a) { if((a)) { delete[ ] (a); (a) = NULL; } }
#endif

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

struct EnumData
{
	DWORD dwProcessId;
	HWND hWnd;
};

namespace Util
{
	HWND GetProcessHWND( );
	HMODULE GetCurrentModule( );
	HMODULE GetModuleFromAddress( uintptr_t adr );
	HMODULE GetModuleHandleSafe( const char *ModuleName );
	void GetModuleAddressAndSize( HMODULE ModuleHandle , DWORD_PTR* base , DWORD_PTR* size );

	template <typename T , size_t N>
	inline size_t SizeOfArray( const T( &)[N] )
	{
		return N;
	}
}

#endif