#pragma once
#include <windows.h>

EXTERN_C int GetModuleVersionInfo(HMODULE hModule, PCWSTR pwszSubBlock, LPCVOID* ppv);
