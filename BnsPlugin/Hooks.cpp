#include "Hooks.h";
#include "PluginConfig.h"
#include <iostream>

extern PluginConfig* pluginConfig;
int32_t focusState = 0; //game focused or not
uint64_t focusUnfocusAddr;

/*
* Function that is called when fps limit settings are saved.
* Fortunately called after every loadscreen or escape.
*/
void(__fastcall* oSetForegroundFpsLimit)(int64_t* arg1, int32_t arg2);
void __fastcall hkSetForegroundFpsLimit(int64_t* arg1, int32_t arg2) {
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


/*
* Function that is called when a new map is loaded.
* Fortunately called right at the start of a loadscreen.
*/
bool(_fastcall* oUiStateGame)(int64_t* thisptr);
bool _fastcall hkUiStateGame(int64_t* thisptr) {
#ifdef _DEBUG
	std::wcout << L"loading screen start" << std::endl;
#endif // _DEBUG
	if (focusUnfocusAddr != 0) {
		uint32_t* foregroundPtr = (uint32_t*)(focusUnfocusAddr + 0x144);
		uint32_t* backgroundPtr = (uint32_t*)(focusUnfocusAddr + 0x148);

		*foregroundPtr = 0;
		*backgroundPtr = 0;

#ifdef _DEBUG
		std::wcout << L"setting all fps limits to: " << *foregroundPtr << L" (unlimited)" << std::endl;
#endif // _DEBUG

		oFocusUnfocus((uint64_t*)focusUnfocusAddr, (focusState ^ 1));
		oFocusUnfocus((uint64_t*)focusUnfocusAddr, focusState);
	}
	return oUiStateGame(thisptr);
}

/*
* Function that is called when the game is focused or unfocused(click outside, minimize, alt - tab etc.)
* focus = 1 => game is now focused
* focus = 0 => game is now unfocused
*/
void(__fastcall* oFocusUnfocus)(uint64_t* arg1, uint32_t arg2);
void __fastcall hkFocusUnfocus(uint64_t* arg1, uint32_t focus) {
#ifdef _DEBUG
	std::wcout << L"focus: " << focus << std::endl;
#endif // _DEBUG
	focusState = focus;
	focusUnfocusAddr = (int64_t)arg1;

	return oFocusUnfocus(arg1, focus);
}