#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <windows.h>

#ifdef _AMD64_
#define _MAX_VALUE ((PVOID)0x000F000000000000)
#define _VALUE ULONG_PTR
#define _Allign 0x7 
#else
#define _MAX_VALUE ((PVOID)0xFFE00000)
#define _VALUE ULONG
#define _Allign 0x3
#endif

#define EXCEPTION_HIT ( ( PVOID )0x1338cafebabef00d )

#pragma code_seg( push, ".text" )
static __declspec(allocate(".text")) UCHAR __stub[4] = { 0x48, 0x8B, 0x01, 0xC3 };
#pragma code_seg( )

typedef PVOID( *tStub )( PVOID );
const tStub lpStub = ( tStub )&__stub;

static LONG NTAPI EH( _EXCEPTION_POINTERS *ExceptionInfo )
{	
#ifdef _AMD64_

	// printf( "   ContextRecord RIP: %I64X\n", ExceptionInfo->ContextRecord->Rip );
	// printf( "   Exception Address: %I64X\n", ExceptionInfo->ExceptionRecord->ExceptionAddress );


	if ( ExceptionInfo->ContextRecord->Rip != ( ULONG_PTR )__stub )
		return EXCEPTION_CONTINUE_SEARCH;

	ExceptionInfo->ContextRecord->Rip += 3;
	ExceptionInfo->ContextRecord->Rax = ( ULONG_PTR )EXCEPTION_HIT;

#else



#endif
	return EXCEPTION_CONTINUE_EXECUTION;
}

__inline bool _VALID( PVOID Ptr )
{
	return ( ( Ptr >= ( PVOID )0x10000 ) && ( Ptr < _MAX_VALUE ) && lpStub( Ptr ) != EXCEPTION_HIT && Ptr != nullptr );
}



#endif