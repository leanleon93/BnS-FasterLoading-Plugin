#include "Hooks.h"
#include "PluginConfig.h"
#include <iostream>

extern PluginConfig* pluginConfig;

/*
* Function that is called when fps limit settings are saved.
* Fortunately called after every loadscreen or escape.
*/
void(__fastcall* oSetForegroundFpsLimit)(int64_t* arg1, int32_t arg2);
void __fastcall hkSetForegroundFpsLimit(int64_t* arg1, int32_t arg2) {
#ifdef _DEBUG
	uint64_t focusUnfocusAddr = (int64_t)arg1;
	auto foregroundPtr = (uint32_t*)(focusUnfocusAddr + 0x144);
	printf("Address of the foregroundFps limit: %p\n", (void*)foregroundPtr);
	std::wcout << L"setting all fps limits to: " << *foregroundPtr << std::endl;
#endif // _DEBUG
#ifdef _DEBUG
	std::wcout << L"fps limits reset" << std::endl;
#endif // _DEBUG
	auto fpsLimit = arg2;
	if (pluginConfig->ForegroundLimit > -1) {
#ifdef _DEBUG
		std::wcout << L"found config override foreground limit: " << pluginConfig->ForegroundLimit << std::endl;
#endif // _DEBUG
		fpsLimit = pluginConfig->ForegroundLimit;
	}
	return oSetForegroundFpsLimit(arg1, fpsLimit);
}