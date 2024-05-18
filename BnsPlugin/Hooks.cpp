#include "Hooks.h"
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

#ifdef _DEBUG
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

bool operator==(const World& lhs, const World& rhs) {
	return
		lhs._activated == rhs._activated &&
		lhs._IsTerrainChanged == rhs._IsTerrainChanged &&
		lhs._isTransit == rhs._isTransit &&
		lhs._isEnterWorld == rhs._isEnterWorld &&
		lhs._isEnterZone == rhs._isEnterZone &&
		lhs._tryLeaveZone == rhs._tryLeaveZone &&
		lhs._leaveReason == rhs._leaveReason &&
		lhs._worldId == rhs._worldId &&
		lhs._zoneId == rhs._zoneId &&
		lhs._geozoneId == rhs._geozoneId;
}

bool operator!=(const World& lhs, const World& rhs) {
	return !(lhs == rhs);
}
#endif // _DEBUG

static void SetAndApplyFpsLimits(uint32_t limit, bool unfocus) {
	if (focusUnfocusAddr != 0) {
		auto* foregroundPtr = (uint32_t*)(focusUnfocusAddr + 0x144);
		auto* backgroundPtr = (uint32_t*)(focusUnfocusAddr + 0x148);

		*foregroundPtr = limit;
		*backgroundPtr = limit;

		oFocusUnfocus((uint64_t*)focusUnfocusAddr, (focusState ^ 1));
		oFocusUnfocus((uint64_t*)focusUnfocusAddr, focusState);
		if (unfocus) {
			oFocusUnfocus((uint64_t*)focusUnfocusAddr, false);
		}
#ifdef _DEBUG
		printf("Address of the foregroundFps limit: %p\n", (void*)foregroundPtr);
		std::wcout << L"setting all fps limits to: " << *foregroundPtr << std::endl;
#endif // _DEBUG
	}
}

static void SetAndApplyFpsLimits(uint32_t limit) {
	SetAndApplyFpsLimits(limit, false);
}

static void SetAndApplyLowFps() {
	SetAndApplyFpsLimits(5, true);
}

static void SetAndApplyUnlimitedFps() {
	SetAndApplyFpsLimits(0);
}

static void SetInTransitFps(const World* world) {
	if (world->_leaveReason == 1 && world->_geozoneId == 0) {
		SetAndApplyLowFps();
	}
	else {
		SetAndApplyUnlimitedFps();
	}
}

static int prevGeoZoneId = -1;
static bool inTransitState = false;

#ifdef _DEBUG
static World* prevWorld = nullptr;
#endif // _DEBUG



/*
* Used to check if the game is in transit state.
*/
World* (__fastcall* BNSClient_GetWorld)();
World* __fastcall hkBNSClient_GetWorld() {
	World* world = BNSClient_GetWorld();
	if (world != nullptr) {

		//if transit state changed or we are leaving zone 0 (f8, char select)
		//cause zone 0 is always in transit state
		if ((world->_isTransit != inTransitState || world->_geozoneId != prevGeoZoneId) && world->_geozoneId == 0 && (world->_leaveReason == 1 || world->_leaveReason == 2)) {
			SetAndApplyLowFps();

			inTransitState = world->_isTransit;
		}
		else if (world->_isTransit != inTransitState || (prevGeoZoneId == 0 && world->_geozoneId != prevGeoZoneId)) {

			if (world->_isTransit) {
				SetInTransitFps(world);
			}
			inTransitState = world->_isTransit;
#ifdef _DEBUG
			std::cout << "World _isTransit toggled: " << world->_isTransit << std::endl;
#endif // _DEBUG
		}

		if (world->_geozoneId != prevGeoZoneId) {
			prevGeoZoneId = world->_geozoneId;
#ifdef _DEBUG
			std::cout << "Geozone changed: " << world->_geozoneId << std::endl;
#endif // _DEBUG
		}

#ifdef _DEBUG
		if (prevWorld && world) {
			if (*prevWorld != *world) {
				std::cout << "World: " << *world << std::endl;
				*prevWorld = *world;
			}
		}
		else {
			std::cout << "World: " << *world << std::endl;
			if (prevWorld == nullptr)
				prevWorld = new World();
			*prevWorld = *world;
		}
#endif // _DEBUG

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