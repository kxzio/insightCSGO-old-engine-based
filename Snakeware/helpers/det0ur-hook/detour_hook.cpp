#include "detour_hook.h"
#include "../utils.hpp"
#include "../../options.hpp"
// Detour nano-tech
// by @Snake & @Kamaz



namespace DetourHooks  {
	void Initialize () {
		// Maybe need GetModuleHandle
		static const auto csPlayerVT = Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C") + 0x47;
		DWORD* exPointer             = (DWORD*)*(DWORD*)(csPlayerVT);

		originalUpdateClientsideAnimation = (UpdateClientsideAnimationT) DetourFunction((PBYTE)exPointer[223], (PBYTE)hkUpdateClientsideAnimation);
		originalDoExtraBoneProcessing     = (DoExtraBoneProcessingT)     DetourFunction((PBYTE)exPointer[197], (PBYTE)hkDoExtraBoneProcessing);
		originalStandartBlendingRules     = (StandartBlendingRulesT)     DetourFunction((PBYTE)exPointer[205], (PBYTE)hkStandardBlendingRules);
	}

	void Shutdown () {
		DetourRemove ((PBYTE)originalUpdateClientsideAnimation, (PBYTE)hkUpdateClientsideAnimation);
		DetourRemove ((PBYTE)originalDoExtraBoneProcessing    , (PBYTE)hkDoExtraBoneProcessing);
		DetourRemove ((PBYTE)originalStandartBlendingRules,     (PBYTE)hkStandardBlendingRules);
	}


};