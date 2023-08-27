#include "PluginConfig.h"
#include "SimpleIni.h"
#include "xorstr.hpp"
#include <filesystem>
#include <iostream>

PluginConfig::PluginConfig()
{
	ForegroundLimit = -1;
	ReloadFromConfig();
}

//Ini file is fully optional and ignored if not exists

//Ini name: fps.ini in plugins folder

//Ini format
// [FPS]
// Foreground = x

/// <summary>
/// Loads config ini value to force overwrite foreground fps limit.
/// </summary>
void PluginConfig::ReloadFromConfig()
{
	WCHAR fullpath[MAX_PATH];
	CSimpleIniA hotkeys;
	hotkeys.SetUnicode();
	GetModuleFileNameW(NULL, fullpath, MAX_PATH);
	std::filesystem::path base(fullpath);
	std::wstring inipath(base.parent_path());
	inipath += xorstr_(L"/plugins/fps.ini");
	const wchar_t* fullinipath = inipath.c_str();
#ifdef _DEBUG
	std::wcout << fullinipath << std::endl;
#endif // _DEBUG

	auto error = hotkeys.LoadFile(fullinipath);
	if (error == SI_OK) {
		// File loaded successfully
		if (hotkeys.GetValue("FPS", "Foreground"))
		{
			ForegroundLimit = std::stoi(hotkeys.GetValue("FPS", "Foreground"));
		}
	}
}