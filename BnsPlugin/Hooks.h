#pragma once
#include <cstdint>

extern void(__fastcall* oSetForegroundFpsLimit)(int64_t* arg1, int32_t arg2);
void __fastcall hkSetForegroundFpsLimit(int64_t* arg1, int32_t arg2);

extern bool(_fastcall* oUiStateGame)(int64_t* thisptr);
bool _fastcall hkUiStateGame(int64_t* thisptr);

extern void(__fastcall* oFocusUnfocus)(uint64_t* arg1, uint32_t arg2);
void __fastcall hkFocusUnfocus(uint64_t* arg1, uint32_t arg2);