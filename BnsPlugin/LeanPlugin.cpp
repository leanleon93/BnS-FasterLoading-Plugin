#include <thread>
#include <pe/module.h>
#include "detours.h"
#include "LeanPlugin.h"
#include <iostream>
#include "searchers.h"
#include "Hooks.h"
#include "xorstr.hpp"
#include "PluginConfig.h"

struct DETOUR_ALIGN
{
	BYTE    obTarget : 3;
	BYTE    obTrampoline : 5;
};

struct DETOUR_INFO
{
	// An X64 instuction can be 15 bytes long.
	// In practice 11 seems to be the limit.
	BYTE            rbCode[30];     // target code + jmp to pbRemain.
	BYTE            cbCode;         // size of moved target code.
	BYTE            cbCodeBreak;    // padding to make debugging easier.
	BYTE            rbRestore[30];  // original target code.
	BYTE            cbRestore;      // size of original target code.
	BYTE            cbRestoreBreak; // padding to make debugging easier.
	DETOUR_ALIGN    rAlign[8];      // instruction alignment array.
	PBYTE           pbRemain;       // first instruction after moved code. [free list]
	PBYTE           pbDetour;       // first instruction of detour function.
	BYTE            rbCodeIn[8];    // jmp [pbDetour]
};

extern "C"
{
	void hkStorageFix();
	LPVOID lpRemain;
	int64_t ogAddress;
}

gsl::span<uint8_t> data;
pe::module* module;
uintptr_t handle;

/// <summary>
/// Setup memory scanner
/// </summary>
/// <returns></returns>
void WINAPI ScannerSetup() {
#ifdef _DEBUG
	std::cout << "ScannerSetup" << std::endl;
#endif // _DEBUG
	module = pe::get_module();
	handle = module->handle();
	const auto sections2 = module->segments();
	const auto& s2 = std::find_if(sections2.begin(), sections2.end(), [](const IMAGE_SECTION_HEADER& x) {
		return x.Characteristics & IMAGE_SCN_CNT_CODE;
		});
	data = s2->as_bytes();
}

/// <summary>
/// Setup config values from fps.ini
/// </summary>
/// <returns></returns>
static void WINAPI InitConfigValues() {
	g_PluginConfig.ReloadFromConfig();
}

_AddInstantNotification oAddInstantNotification;

/// <summary>
/// Setup BnS messaging to send chat or notification messages in game.
/// From Tonic
/// </summary>
/// <returns></returns>
static void WINAPI InitMessaging() {
#ifdef _DEBUG
	std::cout << "InitMessaging" << std::endl;
#endif // _DEBUG

#ifdef _DEBUG
	std::cout << "Searching AddInstantNotification" << std::endl;
#endif // _DEBUG
	// Used for sending notifications about certain actions
	bool diffPattern = false;
	auto sAddNotif = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("45 33 DB 41 8D 42 ?? 3C 02 BB 05 00 00 00 41 0F 47 DB")));
	if (sAddNotif == data.end()) {
		// Old compiler stuff (NAEU CLIENT)
		diffPattern = true;
		sAddNotif = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("33 FF 80 BC 24 80 00 00 00 01 75 05")));
	}

	if (sAddNotif != data.end()) {
		oAddInstantNotification = module->rva_to<std::remove_pointer_t<decltype(oAddInstantNotification)>>((uintptr_t)&sAddNotif[0] - (diffPattern ? 0x13 : 0x68) - handle);
	}

#ifdef _DEBUG
	std::cout << "Searching Done" << std::endl;
#endif // _DEBUG

#ifdef _DEBUG
	printf("Address of AddInstantNotification is %p\n", (void*)oAddInstantNotification);
	std::cout << std::endl;
#endif // _DEBUG
}

/// <summary>
/// </summary>
/// <typeparam name="FuncType"></typeparam>
/// <param name="pattern"></param>
/// <param name="offset"></param>
/// <param name="originalFunction"></param>
/// <param name="hookFunction"></param>
/// <param name="debugName"></param>
/// <returns></returns>
template<typename FuncType>
uintptr_t HookFunction(const char* pattern, int offset, FuncType& originalFunction, FuncType hookFunction, const char* debugName)
{
	auto it = std::search(data.begin(), data.end(), pattern_searcher(pattern));
	if (it != data.end()) {
		uintptr_t address = (uintptr_t)&it[0] + offset;
#ifdef _DEBUG
		printf("Address of %s is %p\n", debugName, (void*)address);
		std::cout << std::endl;
#endif // _DEBUG
		originalFunction = module->rva_to<std::remove_pointer_t<FuncType>>(address - handle);
		DetourAttach(&(PVOID&)originalFunction, hookFunction);
		return address;
	}
	else {
#ifdef _DEBUG
		printf("Address of %s is not found\n", debugName);
		std::cout << std::endl;
#endif // _DEBUG
	}
	return 0;
}

/// <summary>
/// Setup function detours
/// </summary>
/// <returns></returns>
void WINAPI InitDetours() {
#ifdef _DEBUG
	std::cout << "InitDetours" << std::endl;
#endif // _DEBUG
	DetourTransactionBegin();
	DetourUpdateThread(NtCurrentThread());

	HookFunction(xorstr_("48 85 D2 0F 84 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8B F8 48 85 C0 0F 84 ?? ?? ?? ?? 4C 8B 03"), -0x15, oBUIWorld_ProcessEvent, &hkBUIWorld_ProcessEvent, "aBUIWorld_ProcessEvent");
	HookFunction(xorstr_("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 0F B6 FA 48 8B D9"), 0, oFocusUnfocus, &hkFocusUnfocus, "oFocusUnfocus");
	auto fpsAddr = HookFunction(xorstr_("48 83 EC 28 80 3D E1 8A 66 04 00 75 08 39 91 44"), 0, oSetForegroundFpsLimit, &hkSetForegroundFpsLimit, "oSetForegroundFpsLimit");
	if (fpsAddr == 0) {
		HookFunction(xorstr_("75 08 39 91 44 01 00 00 ?? ?? 8B ?? ?? ?? ?? ?? 85 C0"), -0x10, oSetForegroundFpsLimit, &hkSetForegroundFpsLimit, "oSetForegroundFpsLimit");
	}
	HookFunction(xorstr_("?? 48 85 C0 74 ?? 48 8B 80 ?? ?? ?? ?? C3 C3 CC CC CC CC CC CC CC CC CC CC CC 48 8B 05"), -0x06, BNSClient_GetWorld, &hkBNSClient_GetWorld, "BNSClient_GetWorld");
	DetourTransactionCommit();
}


static void WINAPI LeanPlugin_Init()
{
	InitConfigValues();
	ScannerSetup();
	InitMessaging();
	InitDetours();
}

void WINAPI LeanPlugin_Main()
{
#ifdef _DEBUG
	//Open console and print (testing)
	AllocConsole();
	(void)freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);  //insecure
	std::cout << "InitNotification: BNSR.exe" << std::endl;
#endif
	static std::once_flag once;

	std::call_once(once, LeanPlugin_Init);
}