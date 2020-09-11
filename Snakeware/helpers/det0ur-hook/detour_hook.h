#pragma once
#include "../../valve_sdk/csgostructs.hpp"
#include "det0urs-hook.h"
namespace DetourHooks  {

	void Initialize();
	void Shutdown  ();
	// Update-clientside animation's..
	typedef void (__thiscall* UpdateClientsideAnimationT) (C_BasePlayer*);
	extern           UpdateClientsideAnimationT originalUpdateClientsideAnimation;
	void __fastcall  hkUpdateClientsideAnimation (C_BasePlayer* player, uint32_t);

	// Do extraBone proccessing..
	typedef void(__thiscall* DoExtraBoneProcessingT)(void*, studiohdr_t*, Vector*, Quaternion*, const matrix3x4_t&, uint8_t*, void*);
	extern           DoExtraBoneProcessingT originalDoExtraBoneProcessing;
	void _fastcall   hkDoExtraBoneProcessing(void* ecx, uint32_t, studiohdr_t* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_computed, void* context);

	// Standart blending-rules.
	typedef void(__thiscall* StandartBlendingRulesT)(C_BasePlayer*, studiohdr_t*, Vector*, Quaternion*, float, int);
	extern StandartBlendingRulesT originalStandartBlendingRules;
	void _fastcall hkStandardBlendingRules(C_BasePlayer* player, uint32_t, studiohdr_t* hdr, Vector* pos, Quaternion* q, const float time, int mask);
	
};