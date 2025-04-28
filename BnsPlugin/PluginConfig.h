#pragma once
#include <string>

class PluginConfig {
public:
	PluginConfig();
	void Initialize();
	void ReloadFromConfig();
	bool IsLoaded() const;
	static constexpr const wchar_t* ConfigFileName = L"fps.ini"; // Compile-time constant

	std::wstring ConfigPath;
	int32_t ForegroundLimit = -1;
	int32_t LoadingLimit = 999;
	int32_t CharselectLimit = 10;
private:
	bool Loaded = false;
};

extern PluginConfig g_PluginConfig;