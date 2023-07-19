#pragma once
#include <cstdint>

class PluginConfig {
public:
	int32_t ForegroundLimit;
	PluginConfig();
	void ReloadFromConfig();
};