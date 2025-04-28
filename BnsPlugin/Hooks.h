#pragma once
#include <cstdint>
#include <iostream>
#include "BSFunctions.h"

extern void(__fastcall* oSetForegroundFpsLimit)(int64_t* arg1, int32_t arg2);
void __fastcall hkSetForegroundFpsLimit(int64_t* arg1, int32_t arg2);

extern void(__fastcall* oFocusUnfocus)(uint64_t* arg1, uint32_t arg2);
void __fastcall hkFocusUnfocus(uint64_t* arg1, uint32_t arg2);

#ifdef _DEBUG
std::ostream& operator<<(std::ostream& os, const World& world);
bool operator== (const World& lhs, const World& rhs);
bool operator!= (const World& lhs, const World& rhs);
#endif // _DEBUG

extern World* (__fastcall* BNSClient_GetWorld)();
World* __fastcall hkBNSClient_GetWorld();

extern bool(__fastcall* oBUIWorld_ProcessEvent)(uintptr_t* This, EInputKeyEvent* InputKeyEvent);
bool __fastcall hkBUIWorld_ProcessEvent(uintptr_t* This, EInputKeyEvent* InputKeyEvent);