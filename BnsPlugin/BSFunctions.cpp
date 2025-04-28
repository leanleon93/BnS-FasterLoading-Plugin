#include "BSFunctions.h"
#include <iostream>
#include <string>


void BSMessaging::SendGameMessage_s(void* GameWorld, _AddInstantNotification* oAddInstantNotification, const wchar_t* text, const wchar_t* particleRef, const wchar_t* sound, char track, bool stopPreviousSound, bool headline2, bool boss_headline, bool chat, char category, const wchar_t* sound2)
{
	if (GameWorld != nullptr) {

		// Always check if oAddInstantNotification was defined otherwise you will cause a crash
		if (oAddInstantNotification) {
#ifdef _DEBUG
			std::wcout << "Sending message: " << text << std::endl;
#endif // _DEBUG
			(*oAddInstantNotification)(GameWorld, text, particleRef, sound, track, stopPreviousSound, headline2, boss_headline, chat, category, sound2);
		}
		else {
#ifdef _DEBUG
			std::cout << "oAddInstantNotification not defined" << std::endl;
#endif // _DEBUG
		}
	}
	else {
#ifdef _DEBUG
		std::cout << "BNSClientInstance not defined" << std::endl;
#endif // _DEBUG
	}
}

void BSMessaging::DisplaySystemChatMessage(void* GameWorld, _AddInstantNotification* oAddInstantNotification, const wchar_t* text, bool playSound)
{
	SendGameMessage_s(GameWorld, oAddInstantNotification, text, L"", playSound ? L"00003805.Signal_UI.S_Sys_FindNewSpaceCue" : L"", 0, playSound, false, false, true, 22, L"");
}

void BSMessaging::DisplayScrollingTextHeadline(void* GameWorld, _AddInstantNotification* oAddInstantNotification, const wchar_t* text, bool playSound)
{
	SendGameMessage_s(GameWorld, oAddInstantNotification, text, L"", playSound ? L"00003805.Signal_UI.S_Sys_FindNewSpaceCue" : L"", 0, playSound, false, false, false, 0, L"");
}

uintptr_t GetAddress(uintptr_t AddressOfCall, int index, int length)
{
	if (!AddressOfCall)
		return 0;

	long delta = *(long*)(AddressOfCall + index);
	return (AddressOfCall + delta + length);
}

std::string EngineKeyStateString(EngineKeyStateType type) {
	switch (type)
	{
	case EngineKeyStateType::EKS_PRESSED:
		return "EKS_PRESSED";
		break;
	case EngineKeyStateType::EKS_RELEASED:
		return "EKS_RELEASED";
		break;
	case EngineKeyStateType::EKS_REPEAT:
		return "EKS_REPEAT";
		break;
	case EngineKeyStateType::EKS_DOUBLECLICK:
		return "EKS_DOUBLECLICK";
		break;
	case EngineKeyStateType::EKS_AXIS:
		return "EKS_AXIS";
		break;
	default:
		break;
	}
}