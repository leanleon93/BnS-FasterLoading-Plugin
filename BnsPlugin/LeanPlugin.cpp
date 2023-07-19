#pragma once
#include <thread>
#include <pe/module.h>
#include "detours.h";
#include "LeanPlugin.h"
#include <iostream>
#include "searchers.h";
#include "Hooks.h";
#include "xorstr.hpp";
#include "PluginConfig.h"

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

	HookFunction(xorstr_("40 57 48 83 EC 40 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 50 48 89 74 24 58 0F B6 F2 48 8B F9 38 91"), 0, oFocusUnfocus, &hkFocusUnfocus, "oFocusUnfocus");
	HookFunction(xorstr_("48 83 EC 28 80 3D E1 8A 66 04 00 75 08 39 91 44"), 0, oSetForegroundFpsLimit, &hkSetForegroundFpsLimit, "oSetForegroundFpsLimit");
	HookFunction(xorstr_("48 89 4C 24 08 55 56 57 48 8B EC 48 83 EC 40 48 C7 45 F0 FE FF FF FF"), 0, oUiStateGame, &hkUiStateGame, "oUiStateGame");


	DetourTransactionCommit();
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