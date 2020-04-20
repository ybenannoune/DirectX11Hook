#include "Common.h"

BOOL CALLBACK EnumProc( HWND hWnd , LPARAM lParam )
{
	EnumData& ed = *( EnumData* )lParam;
	DWORD dwProcessId = 0x0;
	GetWindowThreadProcessId( hWnd , &dwProcessId );

	if ( ed.dwProcessId == dwProcessId )
	{
		// Found a window matching the process ID
		ed.hWnd = hWnd;
		// Report success
		SetLastError( ERROR_SUCCESS );		
		return FALSE;
	}
	// Continue enumeration
	return TRUE;
}

namespace Util
{	
	HWND GetProcessHWND( )
	{
		EnumData ed = { GetCurrentProcessId( ) };
		if ( !EnumWindows( EnumProc , ( LPARAM )&ed ) && ( GetLastError( ) == ERROR_SUCCESS ) )
		{
			return ed.hWnd;
		}
	}

	HMODULE GetCurrentModule( )
	{
		return reinterpret_cast< HINSTANCE >( &__ImageBase );
	}

	HMODULE GetModuleFromAddress( uintptr_t adr )
	{
		HMODULE hModule = NULL;
		GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT ,
			( LPCTSTR )adr ,
			&hModule );

		return hModule;
	}

	HMODULE GetModuleHandleSafe( const char *ModuleName )
	{
		HMODULE Ret = NULL;

		while ( Ret == NULL )
		{
			Ret = GetModuleHandleA( ModuleName );
			if ( Ret == NULL )
				Sleep( 100 );
		}
		return Ret;
	}

	void GetModuleAddressAndSize( HMODULE ModuleHandle , DWORD_PTR* base , DWORD_PTR* size )
	{
		if ( !ModuleHandle )
			return;

		MODULEINFO module;
		ZeroMemory( &module , sizeof( module ) );
		*base = 0;
		*size = 0;			

		if ( GetModuleInformation( GetCurrentProcess( ) , ModuleHandle , &module , sizeof( module ) ) )
		{
			*base = reinterpret_cast< DWORD_PTR >( module.lpBaseOfDll );
			*size = static_cast< DWORD_PTR >( module.SizeOfImage );
		}
	}

}

