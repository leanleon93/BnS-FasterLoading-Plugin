#pragma once
#include <cstdint>
#include <iostream>

extern void(__fastcall* oSetForegroundFpsLimit)(int64_t* arg1, int32_t arg2);
void __fastcall hkSetForegroundFpsLimit(int64_t* arg1, int32_t arg2);

class World {
public:
	char unknown_0[0x50];
	bool _activated;
	bool _IsTerrainChanged;
	bool _isTransit;
	bool _isEnterWorld;
	bool _isEnterZone;
	bool _tryLeaveZone;
	char _leaveReason;
	char unknown_1[1];
	short _worldId;
	char unknown_2[6];
	__int64 _zoneId;
	int _geozoneId;
	int _prevGeozoneId;
	__int16 _arenaChatServerId;
	char pad3[2];
	int _clock;
	char _pcCafeCode;
	bool _isConnectedTestServer;
	char pad4[2];
	int _jackpotFaction1Score;
	int _jackpotFaction2Score;
	__int32 _keyboardModeConvertedResult;
};