#include "stdafx.h"
#include "DxManager.h"
#include <time.h>

DXManager* g_pDxManager;

char* GetFps()
{
	static int	 FPScounter = 0;
	static float FPSfLastTickCount = 0.0f;
	static float FPSfCurrentTickCount;
	static char cStr[64] = "";

	FPSfCurrentTickCount = clock() * 0.001f;
	FPScounter++;

	if ((FPSfCurrentTickCount - FPSfLastTickCount) > 1.0f)
	{
		FPSfLastTickCount = FPSfCurrentTickCount;
		sprintf_s(cStr, "%d FPS", FPScounter);
		FPScounter = 0;
	}

	return cStr;
}

void Present()
{			
	if (g_pDxManager && g_pDxManager->IsInitialized())
	{	
		g_pDxManager->DrawCircle(100, 100, 50, DxColors::Gold);
		g_pDxManager->DrawLine(0, 35, 100, 35, DxColors::Aqua);
		g_pDxManager->DrawText(0, 0, 30, true, DxColors::Gold, "DirectX Hook Demo");
		g_pDxManager->DrawText(0, 50, 15, true, DxColors::Blue, GetFps());
	}	
}

DWORD __stdcall InitHook(LPVOID)
{
	g_pDxManager = new DXManager();
	g_pDxManager->PlaceHook(Present);
	ExitThread(0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:	
		HANDLE hHandle = CreateThread(nullptr, 0, InitHook, nullptr, 0, nullptr);
		break;
	}
	return TRUE;
}