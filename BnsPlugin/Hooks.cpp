#include "Hooks.h";
#include "PluginConfig.h"
#include <iostream>

extern PluginConfig* pluginConfig;
int32_t focusState = 1; //game focused or not
uint64_t focusUnfocusAddr;

/*
* Function that is called when fps limit settings are saved.
* Fortunately called after every loadscreen or escape.
*/
void(__fastcall* oSetForegroundFpsLimit)(int64_t* arg1, int32_t arg2);
void __fastcall hkSetForegroundFpsLimit(int64_t* arg1, int32_t arg2) {
	focusUnfocusAddr = (int64_t)arg1;
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

// Overload the output operator (<<)
std::ostream& operator<<(std::ostream& os, const World& world) {
	os << "_activated: " << world._activated
		<< ", _IsTerrainChanged: " << world._IsTerrainChanged
		<< ", _isTransit: " << world._isTransit
		<< ", _isEnterWorld: " << world._isEnterWorld
		<< ", _isEnterZone: " << world._isEnterZone
		<< ", _tryLeaveZone: " << world._tryLeaveZone
		<< ", _leaveReason: " << static_cast<int>(world._leaveReason)
		<< ", _worldId: " << world._worldId
		<< ", _zoneId: " << world._zoneId
		<< ", _geozoneId: " << world._geozoneId;
	return os;
}

void SetAndApplyUnlimitedFps() {
	if (focusUnfocusAddr != 0) {
		uint32_t* foregroundPtr = (uint32_t*)(focusUnfocusAddr + 0x144);
		uint32_t* backgroundPtr = (uint32_t*)(focusUnfocusAddr + 0x148);

		*foregroundPtr = 0;
		*backgroundPtr = 0;

#ifdef _DEBUG
		printf("Address of the foregroundFps limit: %p\n", (void*)foregroundPtr);
		std::wcout << L"setting all fps limits to: " << *foregroundPtr << L" (unlimited)" << std::endl;
#endif // _DEBUG

		oFocusUnfocus((uint64_t*)focusUnfocusAddr, (focusState ^ 1));
		oFocusUnfocus((uint64_t*)focusUnfocusAddr, focusState);
	}
}

bool inTransitState = false;
/*
* Used to check if the game is in transit state.
*/
World* (__fastcall* BNSClient_GetWorld)();
World* __fastcall hkBNSClient_GetWorld() {
	World* world = BNSClient_GetWorld();
	if (world != nullptr) {
		if (world->_isTransit != inTransitState) {
			if (world->_isTransit) {
				SetAndApplyUnlimitedFps();
			}
			inTransitState = world->_isTransit;
#ifdef _DEBUG
			std::cout << "World _isTransit toggled: " << world->_isTransit << std::endl;
#endif // _DEBUG
		}
	}
	return world;
}

/*
* Function that is called when the game is focused or unfocused(click outside, minimize, alt-tab etc.)
* focus = 1 => game is now focused
* focus = 0 => game is now unfocused
*/
void(__fastcall* oFocusUnfocus)(uint64_t* arg1, uint32_t arg2);
void __fastcall hkFocusUnfocus(uint64_t* arg1, uint32_t focus) {
	focusState = focus;
	focusUnfocusAddr = (int64_t)arg1;

#ifdef _DEBUG
	std::wcout << L"focus: " << focus << std::endl;
#endif // _DEBUG

	return oFocusUnfocus(arg1, focus);
}