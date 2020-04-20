#ifndef _CVMTHOOK_H_
#define _CVMTHOOK_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <Windows.h>
#include <stdint.h>
#include <malloc.h>

struct HkContext
{
	uint8_t original_code[ 20 ];
	uint64_t dst_ptr;
	uint8_t far_jmp[6];
};

namespace HookLib
{	
#ifdef _AMD64_
	void* DetourCreate( void* src, void* tgt, int length, HkContext* context = nullptr );
#else
	void *DetourCreate( BYTE *src, const BYTE *dst, const int len );
#endif

	PVOID DetourVTable( void** vTable, int index, void* detour );

	class CVMTHook
	{

	public:
		CVMTHook(void);
		CVMTHook(PDWORD_PTR* ppdwClassBase);
		~CVMTHook(void);	

		bool Initialize(PDWORD_PTR* ppdwClassBase);
		bool Initialize(PDWORD_PTR** pppdwClassBase);

		void UnHook( );
		void UnHook(int index);
		bool UnHook( DWORD_PTR oldFunction, unsigned int iIndex );
		void ReHook( );
		void ReHook(int index);
		
		int GetFuncCount(void);	
		PDWORD_PTR GetOldVMT(void);
		DWORD_PTR GetMethodAddress( int Index );
		DWORD_PTR HookMethod(DWORD_PTR dwNewFunc, unsigned int iIndex);

	private:
		DWORD_PTR GetVMTCount(PDWORD_PTR pdwVMT);
		PDWORD_PTR* m_ppdwClassBase;
		PDWORD_PTR m_pdwNewVMT, m_pdwOldVMT;
		DWORD_PTR m_dwVMTSize, oFunction;	
	};
}

#endif