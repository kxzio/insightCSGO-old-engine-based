#pragma once
#include "../detour_hook.h"
#include "../../../options.hpp"

DetourHooks::UpdateClientsideAnimationT  DetourHooks::originalUpdateClientsideAnimation;

void __fastcall DetourHooks::hkUpdateClientsideAnimation(C_BasePlayer * player, uint32_t) {


	if (!player || !player->IsAlive())
		return originalUpdateClientsideAnimation(player);

	// Ну вообще я видел тут реализацию с массивом по типу UpdateAnims[player->EntIndex()] 
	if (Snakeware::UpdateAnims) {

		originalUpdateClientsideAnimation(player);

	}
}

