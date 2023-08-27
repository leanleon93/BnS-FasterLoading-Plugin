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

PluginConfig* pluginConfig;

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
void WINAPI InitConfigValues() {
	pluginConfig = new PluginConfig();
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

uintptr_t GetAddress(uintptr_t AddressOfCall, int index, int length)
{
	if (!AddressOfCall)
		return 0;

	long delta = *(long*)(AddressOfCall + index);
	return (AddressOfCall + delta + length);
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

	HookFunction(xorstr_("40 57 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 50 48 89 74 24 58 0F B6 ?? 48 8B ?? 38 91"), 0, oFocusUnfocus, &hkFocusUnfocus, "oFocusUnfocus");
	auto fpsAddr = HookFunction(xorstr_("48 83 EC 28 80 3D E1 8A 66 04 00 75 08 39 91 44"), 0, oSetForegroundFpsLimit, &hkSetForegroundFpsLimit, "oSetForegroundFpsLimit");
	if (fpsAddr == 0) {
		HookFunction(xorstr_("75 08 39 91 44 01 00 00 ?? ?? 8B C2"), -0x10, oSetForegroundFpsLimit, &hkSetForegroundFpsLimit, "oSetForegroundFpsLimit");
	}

	auto result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("66 89 54 24 10 48 89 4C 24 08 57 48 81 EC 10 02 00 00 48 C7 84 24 B8 00 00 00 FE FF FF FF")));
	if (result != data.end()) {
		auto address = GetAddress(((uintptr_t)&result[0] + 0x38), 1, 5);
#ifdef _DEBUG
		printf("Address of %s is %p\n", "BNSClient_GetWorld", (void*)address);
		std::cout << std::endl;
#endif // _DEBUG
		BNSClient_GetWorld = module->rva_to<std::remove_pointer_t<decltype(BNSClient_GetWorld)>>(address - handle);
		DetourAttach(&(PVOID&)BNSClient_GetWorld, &hkBNSClient_GetWorld);
	}

	PDETOUR_TRAMPOLINE lpTrampolineData = {}; //Datas to store information about hooking
	auto sStorageFix = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("4C 8B 0E 48 8B CE 44 0F B6 80 32 03 00 00 8B 97 D4 4F 00 00 41 FF 91 88 01 00 00 8B 97 D4 4F 00 00 48 8B CF")));
	if (sStorageFix != data.end()) {
		uintptr_t aStorageFix = (uintptr_t)((uintptr_t)&sStorageFix[0] - 0x0E);
		ogAddress = (*(int32_t*)(aStorageFix + 1)) + aStorageFix + 5;
#ifdef _DEBUG
		printf("Address of aStorageFix is %p\n", (void*)aStorageFix);
		printf("Address of ogAddress in %p\n", (void*)ogAddress);
		std::cout << std::endl;
#endif // _DEBUG
		DetourAttachEx(&(PVOID&)aStorageFix, (PVOID)&hkStorageFix, &lpTrampolineData, nullptr, nullptr); //Use DetourAtttachEx to retrieve information about the hook
	}

	DetourTransactionCommit();

	const auto lpDetourInfo = (DETOUR_INFO*)lpTrampolineData;
	if (lpDetourInfo != nullptr)
		lpRemain = lpDetourInfo->pbRemain; //Retrieve the address to jump back the original function
}


static void WINAPI LeanPlugin_Init()
{
	InitConfigValues();
	ScannerSetup();
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