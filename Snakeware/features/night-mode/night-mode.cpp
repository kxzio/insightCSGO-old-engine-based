#include "night-mode.h"
#include "../../options.hpp"
#include "../../helpers/utils.hpp"
Nightmode g_Nightmode;


static ConVar* old_sky_name;
bool executed = false;
bool NighmoveActive = true;
int BRUH = 1;

void Nightmode::Run() noexcept {


	if (g_Options.esp_nightmode)
	{
		
		Nightmode::Apply();
	}
	else
	{
		Nightmode::Remove();
	}


}

void Nightmode::Apply() noexcept {

	static auto st_spops   = g_CVar->FindVar("r_DrawSpecificStaticProp");
	static auto r_3dsky	   = g_CVar->FindVar("r_3dsky");
	auto local_player      = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));
	old_sky_name           = g_CVar->FindVar("sv_skyname");
	float brightness       = g_Options.esp_nightmode_bright;

	
	if (!local_player)
		return;

	BRUH++;

	std::clamp(BRUH, 0, g_Options.esp_nightmode_bright);

	if ((BRUH != g_Options.esp_nightmode_bright))
	{
		for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i)) {
			auto material = g_MatSystem->GetMaterial(i);

			if (!material)
				continue;

			if (strstr(material->GetTextureGroupName(),          "World")) {
				material->ColorModulate(brightness / 100.f, brightness / 100.f, brightness / 100.f);
				//material->AlphaModulate(g_Options.world_color[3]);//set alpha to texture: WORLD
			}
			else if (strstr(material->GetTextureGroupName(),     "StaticProp")) {
				material->ColorModulate(brightness / 100.f, brightness / 100.f, brightness / 100.f);
				//material->AlphaModulate(g_Options.prop_color[3]);//set alpha to texture:  PROPS

			}
			if (strstr(material->GetTextureGroupName(),          "SkyBox")) {
				//material->AlphaModulate(g_Options.sky_color[3]);//set alpha to texture:   SKYBX
				


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