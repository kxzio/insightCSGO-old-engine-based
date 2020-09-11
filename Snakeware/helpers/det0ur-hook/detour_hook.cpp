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

	void __fastcall hkUpdateClientsideAnimation(C_BasePlayer * player, uint32_t) {
		

		if (!player || !player->IsAlive())
			return originalUpdateClientsideAnimation(player);

		// �� ������ � ����� ��� ���������� � �������� �� ���� UpdateAnims[player->EntIndex()] 
		if (Snakeware::UpdateAnims) {

            originalUpdateClientsideAnimation(player);

		}
	}




	void _fastcall hkDoExtraBoneProcessing(void * ecx, uint32_t, studiohdr_t * hdr, Vector * pos, Quaternion * q, const matrix3x4_t & matrix, uint8_t * bone_computed, void * context) {

		return;
	}

	void _fastcall hkStandardBlendingRules(C_BasePlayer * player, uint32_t, studiohdr_t * hdr, Vector * pos, Quaternion * q, const float time, int mask)
	{
		if (!player)
			return originalStandartBlendingRules(player, hdr, pos, q, time, mask);

		*(int*)((DWORD)player + 0x2698) = 0;
		mask |= 0x200;

		player->GetEffect() |= C_BaseEntity::E_F_NOINTERP;
		originalStandartBlendingRules(player, hdr, pos, q, time, mask);
		player->GetEffect() &= ~C_BaseEntity::E_F_NOINTERP;
	}

};