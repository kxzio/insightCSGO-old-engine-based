#include "night-mode.h"
#include "../../options.hpp"
#include "../../helpers/utils.hpp"
Nightmode g_Nightmode;


static ConVar* old_sky_name;
bool executed = false;

void Nightmode::Run() noexcept {
	if (!g_Options.esp_enabled)
		return;

	g_Options.esp_nightmode ? g_Nightmode.Apply() : g_Nightmode.Remove();

	static auto r_drawspecificstaticprop = g_CVar->FindVar("r_DrawSpecificStaticProp");
	r_drawspecificstaticprop->SetValue(g_Options.esp_nightmode ? 0 : 1);

	static auto r_3dsky = g_CVar->FindVar("r_3dsky");
	r_3dsky->SetValue(g_Options.esp_nightmode ? 0 : 1);
}

void Nightmode::Apply() noexcept {
	if (executed) {
		return;
	}

	auto local_player = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));

	if (!local_player)
		return;

	old_sky_name = g_CVar->FindVar("sv_skyname");
	float brightness = g_Options.esp_nightmode_bright / 100.f;

	for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i)) {
		auto material = g_MatSystem->GetMaterial(i);

		if (!material)
			continue;

		if (strstr(material->GetTextureGroupName(), "World")) {
			material->ColorModulate(brightness, brightness, brightness);
		}
		else if (strstr(material->GetTextureGroupName(), "StaticProp")) {
			material->ColorModulate(brightness + 0.25f, brightness + 0.25f, brightness + 0.25f);
		}
		if (strstr(material->GetTextureGroupName(), ("SkyBox"))) {
			material->ColorModulate(g_Options.sky_color[0], g_Options.sky_color[1], g_Options.sky_color[2] );
		}
	}

	Utils::LoadNamedSky("sky_csgo_night02");
	executed = true;
}

void Nightmode::Remove() noexcept {
	if (!executed) {
		return;
	}

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

	executed = false;
}