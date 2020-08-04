#include "chams.hpp"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"


Chams::Chams() {
  

	materialRegular = g_MatSystem->FindMaterial("debug/debugambientcube");
	materialFlat = g_MatSystem->FindMaterial("debug/debugdrawflat");
	materialSpaceGlow = g_MatSystem->FindMaterial("dev/glow_armsrace", TEXTURE_GROUP_MODEL);
	materialGlow = g_MatSystem->FindMaterial("glowOverlay", TEXTURE_GROUP_MODEL);
	materialDog = g_MatSystem->FindMaterial("vitality_metallic", TEXTURE_GROUP_MODEL);

}

Chams::~Chams() {
}

void Chams::OverrideMaterial(bool ignorez, int type, const Color& rgba)
{
	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected()) return;


	IMaterial* material = nullptr;
	switch (type)
	{
	case 0: material = materialRegular; break;
	case 1: material = materialFlat; break;
	case 2: material = materialSpaceGlow; break;
	case 3: material = materialGlow; break;
	case 4: material = materialDog; break;
	}
	
		bool bFound = false;
		// https://developer.valvesoftware.com/wiki/Category:List_of_Shader_Parameters
		if (type == 2)
		{
			auto pVar = materialSpaceGlow->FindVar("$envmaptint", &bFound);
			if (bFound)
				(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, rgba.r() / 255.f, rgba.g() / 255.f, rgba.b() / 255.f);
			
		}
		else if (type == 3)
		{
			auto pVar = materialGlow->FindVar("$envmaptint", &bFound);
			if (bFound)
				(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, rgba.r() / 255.f, rgba.g() / 255.f, rgba.b() / 255.f);
		}
		
	
	
	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignorez);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);
	g_MdlRender->ForcedMaterialOverride(material);
}

void Chams::OnDrawModelExecute(
	IMatRenderContext* ctx,
	const DrawModelState_t& state,
	const ModelRenderInfo_t& info,
	matrix3x4_t* matrix)
{
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	const auto mdl = info.pModel;

	bool is_arm = strstr(mdl->szName, "arms") != nullptr;
	bool is_weapon = strstr(mdl->szName, "weapons") != nullptr;
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
	//bool is_weapon = strstr(mdl->szName, "weapons/v_")  != nullptr;

	if (is_player && g_Options.chams_player_enabled) {
		// 
		// Draw player Chams.
		// 
		auto ent = C_BasePlayer::GetPlayerByIndex(info.entity_index);

		if (ent && g_LocalPlayer && ent->IsAlive()) {
			const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
			if (!enemy && g_Options.chams_player_enemies_only)
				return;

			const auto clr_front = enemy ? Color(g_Options.color_chams_player_enemy_visible) : Color(g_Options.color_chams_player_ally_visible);
			const auto clr_back = enemy ? Color(g_Options.color_chams_player_enemy_occluded) : Color(g_Options.color_chams_player_ally_occluded);
			
			if (g_Options.chams_player_ignorez) {
				OverrideMaterial(true ,g_Options.chams_player_type,clr_back);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(false, g_Options.chams_player_type,clr_front);
			}
			else {
				OverrideMaterial(false, g_Options.chams_player_type ,clr_front);
			}
		}

		if (ent == g_LocalPlayer && g_LocalPlayer->IsAlive() && g_LocalPlayer )
		{
			
			if (Snakeware::FakeMatrix != nullptr )
			{
				if (g_Options.chams_fake)
				{
					const auto color4ek = Color(g_Options.color_chams_fake);
					if (g_Input->m_fCameraInThirdPerson)
					{
						for (auto& i : Snakeware::FakeMatrix)
						{
							i[0][3] += info.origin.x;
							i[1][3] += info.origin.y;
							i[2][3] += info.origin.z;
						}

						OverrideMaterial(false, g_Options.chams_player_type, color4ek);
						fnDME(g_MdlRender, 0, ctx, state, info, Snakeware::FakeMatrix);



						for (auto& i : Snakeware::FakeMatrix)
						{
							i[0][3] -= info.origin.x;
							i[1][3] -= info.origin.y;
							i[2][3] -= info.origin.z;
						}

					}
				}
					
				

				
			}
		}


	}
	else if (is_sleeve && g_Options.chams_arms_enabled) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		// 
		// Remove sleeves when drawing Chams.
		// 
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		g_MdlRender->ForcedMaterialOverride(material);
	}
	else if (is_arm) {
		Color hands = Color(g_Options.color_chams_arms_visible);
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		if (g_Options.misc_no_hands) {
			// 
			// No hands.
			// 
			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			g_MdlRender->ForcedMaterialOverride(material);
		}
		else if (g_Options.chams_arms_enabled) {
			if (g_Options.chams_arms_ignorez) {
				OverrideMaterial(
					true,g_Options.chams_arms_type,g_Options.color_chams_arms_occluded);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(false,g_Options.chams_arms_type,hands);
			}
			else {
				OverrideMaterial(
					false, g_Options.chams_arms_type, hands);
			}
		}
	}
	else if (is_weapon)
	{
		Color weapons = Color(g_Options.color_chams_weapons);
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		if (g_Options.chams_weapons)
		{
			OverrideMaterial(true, g_Options.chams_weapons_type, weapons);
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			OverrideMaterial(false, g_Options.chams_weapons_type, weapons);
		}
	}

	
}