#include "night-mode.h"
#include "../../options.hpp"
#include "../../helpers/utils.hpp"
Nightmode g_Nightmode;


static ConVar* old_sky_name;
bool executed = false;

void Nightmode::Run() noexcept { 
	// shitcode fixed
	if (!g_EngineClient->IsInGame() && !g_EngineClient->IsConnected()) return;
	if (g_Options.esp_nightmode ) {
		Apply();
	}
	else if (!g_Options.esp_nightmode) {
		Remove();
	}


}

void Nightmode::Apply() noexcept {

	static auto st_spops   = g_CVar->FindVar("r_DrawSpecificStaticProp");
	static auto r_3dsky	   = g_CVar->FindVar("r_3dsky");
	auto local_player      = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));
	old_sky_name           = g_CVar->FindVar("sv_skyname");
	float brightness       = g_Options.esp_nightmode_bright / 100.f;
	float WorldColorVar[4] = { g_Options.world_color[0], g_Options.world_color[1], g_Options.world_color[2], g_Options.world_color[3] };
	float PropColorVar[4]  = { g_Options.prop_color[0], g_Options.prop_color[1], g_Options.prop_color[2], g_Options.prop_color[3] };
	float SkyColorVar[4]   = { g_Options.sky_color[0], g_Options.sky_color[1], g_Options.sky_color[2], g_Options.sky_color[3] };
	

	
	if (!local_player) return; // why ?

	if ((WorldColorVar != g_Options.world_color) || (PropColorVar != g_Options.prop_color) || (SkyColorVar != g_Options.sky_color)) {

		for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i)) {
			auto material = g_MatSystem->GetMaterial(i);

			if (!material) continue;

			if (strstr(material->GetTextureGroupName(),          "World")) {
				material->ColorModulate(g_Options.world_color[0] + brightness, g_Options.world_color[1] + brightness, g_Options.world_color[2] + brightness);
				material->AlphaModulate(g_Options.world_color[3]);//set alpha to texture: WORLD
			}
			else if (strstr(material->GetTextureGroupName(),     "StaticProp")) {
				material->ColorModulate(g_Options.prop_color[0] + brightness, g_Options.prop_color[1] + brightness, g_Options.prop_color[2] + brightness);
				material->AlphaModulate(g_Options.prop_color[3]);//set alpha to texture:  PROPS

			}
			if (strstr(material->GetTextureGroupName(), "SkyBox")) {
				material->ColorModulate(g_Options.sky_color[0], g_Options.sky_color[1], g_Options.sky_color[2]);
				material->AlphaModulate(g_Options.sky_color[3]);//set alpha to texture:   SKYBX
				


			}
		}
	}

	r_3dsky->SetValue(g_Options.esp_nightmode ? 0 : 1);
	st_spops->SetValue(g_Options.esp_nightmode ? 0 : 1);
	Utils::LoadNamedSky("sky_csgo_night02");

}

void Nightmode::Remove() noexcept {


	auto local_player = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));
	if (!local_player)
		return;

	for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i)) {
		auto material = g_MatSystem->GetMaterial(i);

		if (!material)
			continue;

		if (strstr(material->GetTextureGroupName(), "World")) {
			material->ColorModulate(1.f, 1.f, 1.f);
		}
		else if (strstr(material->GetTextureGroupName(), "StaticProp")) {
			material->ColorModulate(1.f, 1.f, 1.f);
		}
		if (strstr(material->GetTextureGroupName(), ("SkyBox"))) {
			material->ColorModulate(1.f, 1.f, 1.f);
		}
	}

	if (old_sky_name)
		Utils::LoadNamedSky(old_sky_name->GetString());

}