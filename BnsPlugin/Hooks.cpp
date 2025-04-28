#include "Hooks.h"
#include "PluginConfig.h"
#include <unordered_map>
#include <iostream>

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
	auto foregroundPtr = (uint32_t*)(focusUnfocusAddr + 0x144);
	printf("Address of the foregroundFps limit: %p\n", (void*)foregroundPtr);
	std::cout << "setting all fps limits to: " << *foregroundPtr << std::endl;
#endif // _DEBUG
#ifdef _DEBUG
	std::cout << "fps limits reset" << std::endl;
#endif // _DEBUG
	auto fpsLimit = arg2;
	if (g_PluginConfig.ForegroundLimit > -1) {
#ifdef _DEBUG
		std::cout << "found config override foreground limit: " << g_PluginConfig.ForegroundLimit << std::endl;
#endif // _DEBUG
		fpsLimit = g_PluginConfig.ForegroundLimit;
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
		std::cout << "setting all fps limits to: " << *foregroundPtr << std::endl;
#endif // _DEBUG
	}
}
#ifdef _BNSLIVE 
bool applyLowFpsToNextLoadingScreen = false; //previous loading screen was char select
#endif
static void SetAndApplyFpsLimits(uint32_t limit) {
	SetAndApplyFpsLimits(limit, false);
}
static void SetAndApplyUnlimitedFps() {
#ifdef _BNSLIVE 
	if (applyLowFpsToNextLoadingScreen) { //if previous loading screen was char select also apply low fps to login
		SetAndApplyFpsLimits(g_PluginConfig.CharselectLimit, true);
	}
	else {
		SetAndApplyFpsLimits(g_PluginConfig.LoadingLimit);
	}
	applyLowFpsToNextLoadingScreen = false;
#else
	SetAndApplyFpsLimits(g_PluginConfig.LoadingLimit);
#endif
}

#ifdef _BNSLIVE 
static void SetAndApplyLowFps() {
	SetAndApplyFpsLimits(5, true);
	applyLowFpsToNextLoadingScreen = true;
}
static void SetInTransitFps(const World* world) {
	if (world->_leaveReason == 1 && world->_geozoneId == 0) {
		SetAndApplyLowFps();
	}
	else {
		SetAndApplyUnlimitedFps();
	}
}
#endif

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
#ifdef _BNSLIVE
		if ((world->_isTransit != inTransitState || world->_geozoneId != prevGeoZoneId) && world->_geozoneId == 0 && (world->_leaveReason == 1 || world->_leaveReason == 2)) {
			SetAndApplyLowFps();
			inTransitState = world->_isTransit;
		}
		else if (world->_isTransit != inTransitState || (prevGeoZoneId == 0 && world->_geozoneId != prevGeoZoneId)) {
			if (world->_isTransit) {
				SetInTransitFps(world);
			}
#else
		if (world->_isTransit != inTransitState || (prevGeoZoneId == 0 && world->_geozoneId != prevGeoZoneId)) {
			if (world->_isTransit) {
				SetAndApplyUnlimitedFps();
			}
#endif
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
				//std::cout << "World: " << *world << std::endl;
				*prevWorld = *world;
			}
		}
		else {
			//std::cout << "World: " << *world << std::endl;
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
void(__fastcall * oFocusUnfocus)(uint64_t * arg1, uint32_t arg2);
void __fastcall hkFocusUnfocus(uint64_t * arg1, uint32_t focus) {
	focusState = focus;
	focusUnfocusAddr = (int64_t)arg1;

#ifdef _DEBUG
	std::cout << "focus: " << focus << std::endl;
#endif // _DEBUG

	return oFocusUnfocus(arg1, focus);
}

template <typename Callable>
void handleKeyEventWithModifiers(
	EInputKeyEvent const* InputKeyEvent,
	int vKeyTarget,
	bool alt,
	bool shift,
	bool ctrl,
	const Callable & onPress
) {
	static std::unordered_map<int, bool> toggleKeys;
	if (vKeyTarget == 0)  return;
	if (InputKeyEvent->_vKey == vKeyTarget) {
		bool& toggleKey = toggleKeys[vKeyTarget];
		if (!toggleKey && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
			// Check for Alt, Shift, and Ctrl modifiers
			if ((alt == InputKeyEvent->bAltPressed) &&
				(shift == InputKeyEvent->bShiftPressed) &&
				(ctrl == InputKeyEvent->bCtrlPressed)) {
				toggleKey = true;
				onPress();
			}
		}
		else if (toggleKey && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED) {
			toggleKey = false;
		}
	}
}
extern _AddInstantNotification oAddInstantNotification;
bool(__fastcall * oBUIWorld_ProcessEvent)(uintptr_t * This, EInputKeyEvent * InputKeyEvent);
bool __fastcall hkBUIWorld_ProcessEvent(uintptr_t * This, EInputKeyEvent * InputKeyEvent) {
	if (!InputKeyEvent)
		return false;
	if (!g_PluginConfig.IsLoaded()) return oBUIWorld_ProcessEvent(This, InputKeyEvent);
	if (InputKeyEvent->vfptr->Id(InputKeyEvent) == 2) {
		handleKeyEventWithModifiers(InputKeyEvent, 0x50, true, true, false, []() {
			g_PluginConfig.ReloadFromConfig();
			auto message = LR"(FPS Plugin Config Reloaded)";
			auto gameWorld = BNSClient_GetWorld();
			BSMessaging::DisplaySystemChatMessage(gameWorld, &oAddInstantNotification, message, false);
			});
	}
	return oBUIWorld_ProcessEvent(This, InputKeyEvent);
}