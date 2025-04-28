#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <set>
#include <vector>

enum class EngineKeyStateType {
	EKS_PRESSED = 0,
	EKS_RELEASED = 1,
	EKS_REPEAT = 2,
	EKS_DOUBLECLICK = 3,
	EKS_AXIS = 4
};

std::string EngineKeyStateString(EngineKeyStateType type);

struct EngineEvent;

struct EngineEventVtbl
{
	void* (__fastcall* __vecDelDtor)(EngineEvent* This, unsigned int);
	int(__fastcall* Id)(EngineEvent* This);
};

struct EngineEvent {
	EngineEventVtbl* vfptr;
	EngineEvent* _next;
	__int64 _etime;
};

struct EInputKeyEvent : EngineEvent {
	char _vKey;
	char padd_2[0x2];
	EngineKeyStateType KeyState;
	bool bCtrlPressed;
	bool bShiftPressed;
	bool bAltPressed;
};

typedef void(__cdecl* _AddInstantNotification)(
	void* thisptr,
	const wchar_t* text,
	const wchar_t* particleRef,
	const wchar_t* sound,
	char track,
	bool stopPreviousSound,
	bool headline2,
	bool boss_headline,
	bool chat,
	char category,
	const wchar_t* sound2);

typedef void* (__cdecl* _BNSClient_GetWorld)();

#ifndef BSMessaging_H
#define BSMessaging_H

class BSMessaging {
private:
	static void SendGameMessage_s(
		void* GameWorld,
		_AddInstantNotification* oAddInstantNotification,
		const wchar_t* text,
		const wchar_t* particleRef,
		const wchar_t* sound,
		char track,
		bool stopPreviousSound,
		bool headline2,
		bool boss_headline,
		bool chat,
		char category,
		const wchar_t* sound2);
public:
	static void DisplaySystemChatMessage(void*, _AddInstantNotification*, const wchar_t*, bool playSound);
	static void DisplayScrollingTextHeadline(void*, _AddInstantNotification*, const wchar_t*, bool playSound);
};
#endif // !BSMessaging_H

uintptr_t GetAddress(uintptr_t AddressOfCall, int index, int length);


#pragma pack(push, 1)
struct PartyMemberProperty
{
	char pad0[8];
	__int64 hp;
	char level;
	char mastery_level;
	__int16 x;
	__int16 y;
	__int16 z;
	char pad1[2];
	int geo_zone;
	__int64 max_hp;
	int max_hp_equip;
	__int16 max_sp;
	__int16 yaw;
	char dead;
	char faction;
	char faction2;
	char pad2[1];
	int faction_score;
	char hp_alert;
	char pad3[7];
	__int64 newbie_info_flag;
	__int64 guard_gauge;
	__int64 max_guard_gauge;
	int max_guard_gauge_equip;
	char pad4[6];
};

struct FWindowsPlatformTime {
};

struct PreciseTimer // sizeof=0x10
{
	unsigned __int64 _startTime;
	FWindowsPlatformTime _timer;
	char pad0[3];
	float _limit;
};
struct Member {
	int _memberKey;
	char pad0[4];
	unsigned __int64 _creatureId;
	PartyMemberProperty _property;
	void* _manager;
	int _summonedDataId;
	int _zoneChannel;
	int _race;
	int _job;
	bool _inSight;
	bool _login;
	bool _banishAgreement;
	char pad1[5];
	/*std::vector<Member::MemberEffect> _memberEffectList;*/
	char padMemberEffect[0x18];
	PreciseTimer _logoutTimer;
	std::wstring _name;
	__int16 _worldId;
	bool _changedDeadState;
	char pad3[5];
	PreciseTimer _deadStateTimer;
	std::set<unsigned __int64> _aggroNormalNpcList;
	unsigned __int64 _aggroBoss1NpcId;
	unsigned __int64 _aggroBoss2NpcId;
	unsigned __int64 _aggroBoss3NpcId;
	unsigned __int64 _aggroBoss4NpcId;
	unsigned __int64 _aggroSummonedId;
	std::set<unsigned __int64> _summonedAggroNormalNpcList;
	unsigned __int64 _summonedAggroBoss1NpcId;
	unsigned __int64 _summonedAggroBoss2NpcId;
	bool _comebackSession;
	bool _reinforcement;
	char _newbieCareDungeonList[20];
	bool _newbieCareStae;
	char pad6[1];
};

struct Party {
	void* _vtbl;
	unsigned __int64 _partyId;
	//SimpleVector<Member*> _memberList;
	std::vector<Member*> _memberList;
};

struct Player {
	char pad0[0x08];
	unsigned __int64 id;
	char pad1[0xE60 - 16];
	Party* Party;
};

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
	char pad5[56];
	Player* _player;
	void* _playerSummoned;
};

#pragma pack(pop)