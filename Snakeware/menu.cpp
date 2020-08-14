#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "config.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"

#include "imgui/smoke.h"

// config
#include "conifg-system/config-system.h"
#include <filesystem>
#include "fonts/Main_googlefont.h"
#include "Protected/enginer.h"
#include "render.hpp"
namespace fs = std::filesystem;




namespace ImGuiEx
{
	inline bool ColorEdit1337(const char* label, Color* v, bool show_alpha = true)
	{
		auto clr = ImVec4{
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};

		if (ImGui::ColorEdit4(label, &clr.x, show_alpha)) {
			v->SetColor(clr.x, clr.y, clr.z, clr.w);
			return true;
		}
		return false;
	}
	inline bool ColorEdit3(const char* label, Color* v)
	{
		return ColorEdit1337(label, v, false);
	}
}

template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h, bool sameline)
{
	bool values[N] = { false };

	values[activetab] = true;

	for (auto i = 0; i < N; ++i) {
		if (ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) {
			activetab = i;
		}
		if (sameline && i < N - 1)
			ImGui::SameLine();
	}
}


//==============================================================================================================
static int weapon_index = 7;
static int weapon_vector_index = 0;
struct WeaponName_t {
	constexpr WeaponName_t(int32_t definition_index, const char* name) :
		definition_index(definition_index),
		name(name) {
	}

	int32_t definition_index = 0;
	const char* name = nullptr;
};

std::vector< WeaponName_t> WeaponNames =
{

{ 0 ,"none"},
{ 7, "ak-47" },
{ 8, "aug" },
{ 9, "awp" },
{ 63, "cz75 auto" },
{ 1, "desert-eagle" },
{ 2, "dual-berettas" },
{ 10, "famas" },
{ 3, "five-seveN" },
{ 11, "g3sg1" },
{ 13, "galil ar" },
{ 4, "glock-18" },
{ 14, "m249" },
{ 60, "m4a1-s" },
{ 16, "m4a4" },
{ 17, "mac-10" },
{ 27, "mag-7" },
{ 33, "mp7" },
{ 23, "mp5" },
{ 34, "mp9" },
{ 28, "negev" },
{ 35, "nova" },
{ 32, "p2000" },
{ 36, "p250" },
{ 19, "p90" },
{ 26, "pp-bizon" },
{ 64, "r8 revolver" },
{ 29, "sawed-Off" },
{ 38, "scar20" },
{ 40, "ssg-08" },
{ 39, "sg 553" },
{ 30, "tec-9" },
{ 24, "ump-45" },
{ 61, "usp-s" },
{ 25, "xm1014" }

};

void RenderCurrentWeaponButton(float width) {

	if (!g_EngineClient->IsInGame() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon)   return;
	ImGui::SameLine();
	if (!g_Options.legitbot_auto_weapon) {
		if (ImGui::Button("current")) {
			short wpn_idx = weapon->m_Item().m_iItemDefinitionIndex();
			auto wpn_it = std::find_if(WeaponNames.begin(), WeaponNames.end(), [wpn_idx](const WeaponName_t& a) {
				return a.definition_index == wpn_idx;
				});
			if (wpn_it != WeaponNames.end()) {
				weapon_index = wpn_idx;
				weapon_vector_index = std::abs(std::distance(WeaponNames.begin(), wpn_it));
			}
		}
	}
	else
	{
		short wpn_idx = weapon->m_Item().m_iItemDefinitionIndex();
		auto wpn_it = std::find_if(WeaponNames.begin(), WeaponNames.end(), [wpn_idx](const WeaponName_t& a) {
			return a.definition_index == wpn_idx;
			});
		if (wpn_it != WeaponNames.end()) {
			weapon_index = wpn_idx;
			weapon_vector_index = std::abs(std::distance(WeaponNames.begin(), wpn_it));
		}
	}
}
// =====================================================================================================================

void RenderWatermark()
{
	if (!g_Options.misc_watermark) return;
	ImGui::SetNextWindowSize(ImVec2{ 150, 30 }, ImGuiSetCond_Once);

	static bool watermark = true;

	if (ImGui::Begin(" by @shitcoder | @ba1m0v ", &watermark, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)) {
		ImGui::Text("snakeware [alpha] | CS:GO");
	}
	ImGui::End();


}





void RenderRageBotTab()
{
	//  SUB TABS  // 

	static int SubTabs = 1;

	ImGui::SubTabButton("Ragebot", &SubTabs, 0, 4);
	ImGui::SubTabButton("AA", &SubTabs, 1, 4);
	ImGui::SubTabButton("Legit", &SubTabs, 2, 4);
	ImGui::SubTabButton("Other", &SubTabs, 3, 4);



	//  SUB END  //

	switch (SubTabs)
	{
	case 0:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("Ragebot", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::CheckboxEX("Enabled##rage", &g_Options.ragebot_enabled, "Turn on ragebot");
			//ImGui::Checkbox("Hitchance const", &g_Options.ragebot_hitchance_consider); íåòó â ðàãå
			//ImGui::Checkbox("Backtrack", &g_Options.ragebot_position_adj); ëîìàåò ðàãå
			//ImGui::Checkbox("Back-shoot priority", &g_Options.ragebot_backshoot); íåòó 
			ImGui::CheckboxEX("Silent", &g_Options.ragebot_silent, "NoVisible aim");
			ImGui::CheckboxEX("Remove recoil", &g_Options.ragebot_remove_recoil, "No recoil with rifles");
			//ImGui::SliderInt("Max misses :", &g_Options.ragebot_max_miss, 0, 10);  íÿìà â ðàãå
			//ImGui::Text("Key's");
			//ImGui::Text("Force-baim key :");
			//ImGui::Hotkey("##13Keybind", &g_Options.ragebot_baim_key, ImVec2(150, 20)); íåòó â ðàãå
			//ImGui::Text("Damage override key :");
			ImGui::Hotkey("test", &g_Options.ragebot_mindamage_override_key, ImVec2(150, 20));

			const char* autostop[] = { "Default","Maximum","Forced low","Full" };
			const char* autostop_if[] = { "If low hitchance","Always" };

			ImGui::CheckboxEX("Auto-scope", &g_Options.ragebot_autoscope, "autoscope with snipers");
			ImGui::CheckboxEX("Auto-stop", &g_Options.ragebot_autostop, "automatic stop");
			ImGui::CheckboxEX("Between shots", &g_Options.ragebot_autostop_bs, "stop you after shots");
			ImGui::Combo("Autostop type", &g_Options.ragebot_autostop_type, autostop, ARRAYSIZE(autostop));
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));


		const char* weapon[] = { "Pistols","Rifles","SMG","Shotguns","Auto","Scout","AWP","Other" };

		static int curGroup;

		ImGui::BeginChild("Weapon", ImVec2(310, 118), true);
		{



			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::PushFont(g_pCSGO_icons);
			{

				if (ImGui::Button("D", ImVec2(44 + 8, 22)))
					curGroup = WEAPON_GROUPS::AUTO;
				ImGui::SameLine();
				if (ImGui::Button("F", ImVec2(44 + 8, 22)))
					curGroup = WEAPON_GROUPS::SCOUT;
				ImGui::SameLine();
				if (ImGui::Button("g", ImVec2(44 + 8, 22)))
					curGroup = WEAPON_GROUPS::AWP;
				ImGui::SameLine();
				if (ImGui::Button("R", ImVec2(44 + 8, 22)))
					curGroup = WEAPON_GROUPS::H_PISTOLS;
				ImGui::SameLine();
				if (ImGui::Button("b", ImVec2(44 + 8, 22)))
					curGroup = WEAPON_GROUPS::PISTOLS;

			}
			ImGui::PopFont();

			ImGui::SliderFloat("Hit-chance", &g_Options.ragebot_hitchance[curGroup], 0, 99);

		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100 + 118 + 17 + 10));

		ImGui::BeginChild("Settings", ImVec2(310, 210), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::CheckboxEX("Auto-fire", &g_Options.ragebot_autofire, "Shoot automatic");
			ImGui::SliderInt("Fov", &g_Options.ragebot_fov, 0, 360);


			ImGui::SliderFloat("DMG autowall", &g_Options.ragebot_mindamage[curGroup], 0, 120);
			ImGui::SliderFloat("DMG visible", &g_Options.ragebot_vis_mindamage[curGroup], 0, 120);

			//ImGui::Checkbox("Delay shot", &g_Options.ragebot_delayshot[curGroup]);  íåòó â ðàãå
			//ImGui::Checkbox("Ignore hitchance autostop", &g_Options.ragebot_autostop_if[curGroup]);  Íåòó  â ðàãå
			ImGui::CheckboxEX("Auto-crouch", &g_Options.ragebot_autocrouch[curGroup], "Crouch when shooting");
			//ImGui::SliderFloat("Baim if hp lower than :", &g_Options.ragebot_baim_if_hp[curGroup], 0, 100); íåòó â ðàãå
			static std::string prevValue = "Select";
			if (ImGui::BeginCombo("Hitscan", "Select", 0)) {
				//prevValue = "Hitscan";
				const char* hitboxes[] = { "Head", "Neck", "Chest" , "Body","Arms","Legs" };
				std::vector<std::string> vec;
				for (size_t i = 0; i < IM_ARRAYSIZE(hitboxes); i++)
				{
					ImGui::Selectable(hitboxes[i], &g_Options.ragebot_hitbox[i][curGroup], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
					if (g_Options.ragebot_hitbox[i][curGroup])
						vec.push_back(hitboxes[i]);
				}

				for (size_t i = 0; i < vec.size(); i++)
				{
					if (vec.size() == 1)
						prevValue += vec.at(i);
					else if (i != vec.size())
						prevValue += vec.at(i) + ", ";
					else
						prevValue += vec.at(i);
				}
				ImGui::EndCombo();
			}
			const char* body[] = { "Default","Medium","Priority","Maximum" };
			///ImGui::Checkbox("Bodyaim priority", &g_Options.ragebot_adaptive_baim[curGroup]); íåòó â ðàãå

			ImGui::SliderFloat("Point-scale", &g_Options.ragebot_pointscale[curGroup], 0, 100);
			ImGui::SliderFloat("Body-scale", &g_Options.ragebot_bodyscale[curGroup], 0, 100);
			//ImGui::Text("Accuracy setting's :");
			//ImGui::SliderFloat("Damage ovveride", &g_Options.ragebot_mindamage_override[curGroup], 0, 140); íåòó â ðàãå
			//ImGui::Checkbox("Alternative-hitchance method", &g_Options.ragebot_alternative_hitchance[curGroup]); íåòó â ðàãå

		}
		ImGui::EndChild();

	}
	break;
	case 1:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("General", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::CheckboxEX("Enabled##aa", &g_Options.antihit_enabled, "Changing ur angles to be anhitable");
			ImGui::CheckboxEX("Stabilize lby", &g_Options.antihit_stabilize_lby, "Lower body yaw stab");
			const char* pitch[] = { "Down","Up","Zero","Ideal" };
			const char* yaw[] = { "Backward's" };
			const char* fake[] = { "Static","Lagsync" };
			const char* jitter[] = { "Default","Switch" };
			const char* lby[] = { "Default","Opposite","Sway","Low-delta" };
			ImGui::Combo("Pitch", &g_Options.antihit_pitch, pitch, IM_ARRAYSIZE(pitch));
			ImGui::Combo("Yaw", &g_Options.antihit_yaw, yaw, IM_ARRAYSIZE(yaw));
			ImGui::Combo("Fake", &g_Options.antihit_fake, fake, IM_ARRAYSIZE(fake));
			ImGui::Combo("Jitter", &g_Options.antihit_jitter_type, jitter, IM_ARRAYSIZE(jitter));
			ImGui::Combo("Lby-type", &g_Options.antihit_lby, lby, IM_ARRAYSIZE(lby));



		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));

		ImGui::BeginChild("Customization", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);
			ImGui::SliderFloat("Fake ammount", &g_Options.antihit_fake_ammount, 0, 65);
			ImGui::SliderInt("Jitter-radius", &g_Options.antihit_jitter_radius, 0, 120);
			ImGui::SliderInt("Body-lean", &g_Options.antihit_body_lean, 0, 69);
			ImGui::SliderInt("Inverted body-lean", &g_Options.antihit_invert_body_lean, 0, 69);
			ImGui::Text("Key-bind's :");
			ImGui::Text("Switch fake side :");
			ImGui::Hotkey("##Fakekey", &g_Options.antihit_fake_switch_key, ImVec2(150, 20));


		}
		ImGui::EndChild();


	}
	break;
	case 2:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		static char* priorities[] = {
		"fov",
		"health",
		"damage",
		"distance"
		};

		static char* aim_types[] = {
			"hitbox",
			"nearest"
		};
		static char* rcs_types[] = {
					"standalone",
					"on aim"
		};
		static char* smooth_types[] = {
			"static",
			"adaptive"
		};

		static char* fov_types[] = {
			"static",
			"adaptive"
		};

		static char* hitbox_list[] = {
			"head",
			"neck",
			"pelvis",
			"stomach",
			"lower chest",
			"chest",
			"upper chest",
		};


		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("General", ImVec2(310, 118), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			if (ImGui::Combo(
				"##Weapon_aimbot", &weapon_vector_index,
				[](void* data, int32_t idx, const char** out_text) {
					auto vec = reinterpret_cast<std::vector< WeaponName_t >*>(data);
					*out_text = vec->at(idx).name;
					return true;
				}, (void*)(&WeaponNames), WeaponNames.size())) {
				weapon_index = WeaponNames[weapon_vector_index].definition_index;
			}

			RenderCurrentWeaponButton(1337);
			ImGui::CheckboxEX("Auto", &g_Options.legitbot_auto_weapon, "Auto-selection");



		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22, 100 + 118 + 17 + 10));


		auto settings = &g_Options.legitbot_items[weapon_index];


		ImGui::BeginChild("Settings", ImVec2(310, 210), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Text("General-legitbot");
			ImGui::CheckboxEX("Enabled", &settings->enabled, "Aim for valve servers");
			ImGui::CheckboxEX("Friendly-fire", &settings->deathmatch, "Aiming for teemate");
			ImGui::CheckboxEX("Backtrack", &settings->backtrack, "Returns the player in time");
			ImGui::CheckboxEX("Silent-aim", &settings->silent, "NoVisible aim");
			ImGui::Text("Check's");
			ImGui::CheckboxEX("Smoke-check", &settings->smoke_check, "NoAim in smoke");
			ImGui::CheckboxEX("Flash-check ", &settings->flash_check, "NoAim while flashed");
			ImGui::CheckboxEX("Jump-check", &settings->jump_check, "NoAim while jump");
			if (weapon_index == WEAPON_AWP || weapon_index == WEAPON_SSG08 ||
				weapon_index == WEAPON_AUG || weapon_index == WEAPON_SG553) {
				ImGui::CheckboxEX("In-zoom check", &settings->only_in_zoom, "Aim only with scope");
			}
			if (weapon_index == WEAPON_P250 ||
				weapon_index == WEAPON_USPS ||
				weapon_index == WEAPON_GLOCK ||
				weapon_index == WEAPON_FIVESEVEN ||
				weapon_index == WEAPON_TEC9 ||
				weapon_index == WEAPON_DEAGLE ||
				weapon_index == WEAPON_ELITE ||
				weapon_index == WEAPON_P2000) {
				ImGui::CheckboxEX("Auto-pistol", &settings->autopistol, "RifleFire for pistols");
			}



		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));

		ImGui::BeginChild("Customization", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::CheckboxEX("Only on key", &settings->on_key, "Aimbot on key");

			if (settings->on_key) {
				ImGui::Text("Bind");
				ImGui::SameLine();
				ImGui::Hotkey("##Keybind", &settings->key, ImVec2(150, 20));
			}


			ImGui::Text("Legitbot setting's");

			ImGui::Combo("Aim-type", &settings->aim_type, aim_types, IM_ARRAYSIZE(aim_types));

			if (settings->aim_type == 0) {
				ImGui::Text("Hitbox:");
				ImGui::Combo("##aimbot.hitbox", &settings->hitbox, hitbox_list, IM_ARRAYSIZE(hitbox_list));
			}

			ImGui::Combo("Priority", &settings->priority, priorities, IM_ARRAYSIZE(priorities));

			ImGui::Combo("Fov-type", &settings->fov_type, fov_types, IM_ARRAYSIZE(fov_types));
			ImGui::Combo("Smooth-type", &settings->smooth_type, smooth_types, IM_ARRAYSIZE(smooth_types));
			if (settings->silent) {
				ImGui::SliderFloat("Silent fov", &settings->silent_fov, 0, 40);
				settings->fov = 0;
			}
			else
			{
				ImGui::SliderFloat("Fov", &settings->fov, 0, 40);
			}



			ImGui::SliderFloat("Smooth", &settings->smooth, 0, 25);

			ImGui::Text("delay's");
			if (!settings->silent) {
				ImGui::SliderInt("Shot delay", &settings->shot_delay, 0, 200);
			}

			ImGui::SliderInt("Kill delay", &settings->kill_delay, 0, 1001);
			if (settings->backtrack)
				ImGui::SliderFloat("Backtrack-time", &settings->backtrack_time, 0.f, 0.4f);

			ImGui::Text("Secoil setting's");

			ImGui::CheckboxEX("Auto recoil-control", &settings->rcs, "Auto-recoil control");
			ImGui::Combo("Recoil type", &settings->rcs_type, rcs_types, IM_ARRAYSIZE(rcs_types));

			ImGui::SliderInt("Rcs x: ", &settings->rcs_x, 0, 100);
			ImGui::SliderInt("Rcs y: ", &settings->rcs_y, 0, 100);
			ImGui::SliderInt("Rcs start bullet: ", &settings->rcs_start, 1, 29);
			ImGui::Text("Recoil custom setting's");
			ImGui::CheckboxEX("Rcs custom-fov", &settings->rcs_fov_enabled, "Fov in recoil");
			if (settings->rcs_fov_enabled) {
				ImGui::SliderFloat("Rcs fov", &settings->rcs_fov, 0, 30);
			}
			ImGui::CheckboxEX("Rcs custom-smooth", &settings->rcs_smooth_enabled, "Smooth in recoil");
			if (settings->rcs_smooth_enabled) {
				ImGui::SliderFloat("Rcs smooth", &settings->rcs_smooth, 1, 25);
			}


		}
		ImGui::EndChild();


	}
	break;
	case 3:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;



		ImGui::SetCursorPos(ImVec2(22, 100));

		auto settings = &g_Options.legitbot_items[weapon_index];

		ImGui::BeginChild("General", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Text("General-triggerbot");
			ImGui::CheckboxEX("Trigger", &settings->triggerbot_enable, "Autoshoot when player on crosshair");
			if (settings->triggerbot_enable) {

				ImGui::Text("Trigger");
				ImGui::SameLine();
				ImGui::Hotkey("##Triggerkeybind", &g_Options.legitbot_trigger_key, ImVec2(150, 20));
			}
			ImGui::Text("Trigger-bot setting's");
			{
				ImGui::SliderInt("Delay : ", &settings->triggerbot_delay, 0, 250);
				ImGui::SliderFloat("Hitchance :", &settings->triggerbot_hitchance, 0, 100);
			}
			ImGui::Text("Trigger-bot hitboxe's");
			{
				ImGui::CheckboxEX("Hit head", &settings->triggerbot_head, "Trigger head");
				ImGui::CheckboxEX("Hit body", &settings->triggerbot_body, "Trigger body");
				ImGui::CheckboxEX("Hit other-hitboxes", &settings->triggerbot_other, "Trigger legs,arms");
			}



		}
		ImGui::EndChild();



	}
	break;



	}


}
void RenderEspTab()
{

	static int SubTabs = 1;

	ImGui::SubTabButton("Esp", &SubTabs, 0, 2);
	ImGui::SubTabButton("World", &SubTabs, 1, 2);


	bool placeholder_true = true;

	auto& style = ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

	switch (SubTabs)
	{
	case 0:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("General", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::CheckboxEX("enabled", &g_Options.esp_enabled, "Enable wallhack");
			ImGui::CheckboxEX("enemy only", &g_Options.esp_enemies_only, "Enemy wallhack");
			ImGui::CheckboxEX("boxes", &g_Options.esp_player_boxes, "Box around player");
			const char* box_type[4] = { "default", "outlined", "corner" , "corner-outlined" };
			if (g_Options.esp_player_boxes)
				ImGui::Combo("box-type", &g_Options.esp_player_boxes_type, box_type, IM_ARRAYSIZE(box_type));
			ImGui::CheckboxEX("names", &g_Options.esp_player_names, "Draw player name");
			ImGui::CheckboxEX("health", &g_Options.esp_player_health, "Draw player health");
			ImGui::CheckboxEX("armour", &g_Options.esp_player_armour, "Draw armour");
			ImGui::CheckboxEX("weapon", &g_Options.esp_player_weapons, "Show weapon");
			ImGui::CheckboxEX("snaplines", &g_Options.esp_player_snaplines, "Line to player");
			ImGui::CheckboxEX("skeleton", &g_Options.esp_player_skeleton, "Draw skeleton");
			ImGui::CheckboxEX("ammo##1", &g_Options.esp_player_ammo, "Show ammo");
			ImGui::CheckboxEX("head dot", &g_Options.esp_head_dot, "Pos of head");




		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));


		static int curGroup = 0;


		ImGui::BeginChild("Flags", ImVec2(310, 118 + 50), true);
		{
			ImGui::Checkbox("flags enabled", &g_Options.esp_player_flags);

			if (g_Options.esp_player_flags)
			{
				ImGui::Text("flags  :");
				ImGui::CheckboxEX("player", &g_Options.esp_player_flags_player, "Is entity player");
				ImGui::CheckboxEX("scoped", &g_Options.esp_player_flags_scoped, "Is scoped");
				ImGui::CheckboxEX("armour", &g_Options.esp_flags_armour, "Have armour");
				ImGui::CheckboxEX("defuse-kit", &g_Options.esp_flags_kit, "Have D.Kits");
				ImGui::CheckboxEX("on move", &g_Options.esp_flags_move, "Is moving");
				ImGui::CheckboxEX("choking", &g_Options.esp_flags_choking, "Is choking");
				ImGui::CheckboxEX("flashed", &g_Options.esp_flags_flash, "Is flashed");
			}


		}
		ImGui::EndChild();


		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100 + 118 + 17 + 10 + 50));

		ImGui::BeginChild("Model", ImVec2(310, 210 - 50), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::CheckboxEX("chams enabled", &g_Options.chams_player_enabled, "Model chams");
			static const char* glow_type[] = { "outer", "cover", "inner" };
			const char* chams_type[5] = { "material", "flat", "glow-1" , "glow-2", "metallic" };
			if (g_Options.chams_player_enabled)
				ImGui::Combo("chams-type", &g_Options.chams_player_type, chams_type, IM_ARRAYSIZE(chams_type));
			ImGui::CheckboxEX("ignore wall", &g_Options.chams_player_ignorez, "Draw behind wall");
			ImGui::CheckboxEX("ignore team", &g_Options.chams_player_enemies_only, "Only enemy");
			ImGui::Text("arms-chams");
			ImGui::CheckboxEX("arms enabled", &g_Options.chams_arms_enabled, "Draw hands");
			ImGui::CheckboxEX("no arms", &g_Options.misc_no_hands, "Remove hands");
			if (g_Options.chams_arms_enabled && !g_Options.misc_no_hands)
				ImGui::Combo("arms-type", &g_Options.chams_arms_type, chams_type, IM_ARRAYSIZE(chams_type));

			ImGui::CheckboxEX("weapons enabled", &g_Options.chams_weapons, "Weapon chams");
			if (g_Options.chams_weapons)
				ImGui::Combo("gun-type", &g_Options.chams_weapons_type, chams_type, IM_ARRAYSIZE(chams_type));
			ImGui::Text("fake-chams");
			ImGui::CheckboxEX("fake-chams", &g_Options.chams_fake, "Draw desync");
			if (g_Options.chams_fake)
			{
				ImGui::Combo("fake-model", &g_Options.chams_fake_types, chams_type, IM_ARRAYSIZE(chams_type));
			}

			ImGui::Text("player-glow");
			ImGui::CheckboxEX("glow enabled", &g_Options.glow_enabled, "Player outline");
			ImGui::CheckboxEX("glow player's", &g_Options.glow_players, "Players");
			ImGui::CheckboxEX("only enemy", &g_Options.glow_enemies_only, "Only enemy");
			if (g_Options.glow_enabled)
			{
				if (g_Options.glow_enemies_only)
					ImGui::Combo("enemy glow", &g_Options.glow_type, glow_type, IM_ARRAYSIZE(glow_type));
				else
					ImGui::Combo("team glow", &g_Options.glow_team_type, glow_type, IM_ARRAYSIZE(glow_type));
			}


		}
		ImGui::EndChild();
	}
	break;
	case 1:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("World", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Text("viewmodel-setting's");
			ImGui::SliderInt("viewmodel_fov:", &g_Options.viewmodel_fov, 65, 135);
			ImGui::SliderInt("world_fov:", &g_Options.world_fov, 0, 65);
			ImGui::SliderFloat("aspect ratio:", &g_Options.esp_aspect_ratio, 0, 5.5);
			ImGui::Text("grenade prediction :");
			ImGui::CheckboxEX("nade prediction", &g_Options.esp_nade_prediction, "Draw local grenade way");
			ImGui::CheckboxEX("molotov timer", &g_Options.esp_molotov_timer, "Molotov timer (sec)");
			ImGui::Text("sound esp :");
			ImGui::CheckboxEX("sound esp", &g_Options.sound_esp, "Draw sound of player");
			if (g_Options.sound_esp)
			{
				//ImGui::Checkbox("animated beam's (sexy)", &g_Options.sound_esp_animated);
				ImGui::SliderFloat("sound esp radius", &g_Options.sound_esp_radius, 5, 30);
				ImGui::SliderFloat("sound esp time", &g_Options.sound_esp_time, 0.3, 5);
			}

			ImGui::Text("other :");
			//	ImGui::Checkbox("draw bullet-tracer's", &g_Options.esp_bullet_impacts);
			ImGui::CheckboxEX("draw crosshair", &g_Options.esp_crosshair, "Default crosshair");
			ImGui::CheckboxEX("draw recoil-crosshair", &g_Options.esp_recoil_crosshair, "Recoil based crosshair");
			ImGui::CheckboxEX("draw legitbot fov", &g_Options.esp_draw_fov, "Aim radius for legitbot");
			ImGui::CheckboxEX("draw angle's", &g_Options.esp_angle_lines, "Fake and Real angles");
			ImGui::CheckboxEX("draw shot-hitboxes", &g_Options.shot_hitboxes, "Hitbox Positions");
			if (g_Options.shot_hitboxes)
				ImGui::SliderFloat("shot-hitboxes duration", &g_Options.shot_hitboxes_duration, 0, 7);
			ImGui::CheckboxEX("OFESP", &g_Options.esp_offscreen, "Flag invisible players on screen");
			if (g_Options.esp_offscreen)
			{
				ImGui::SliderFloat("offscreen-radius:", &g_Options.esp_offscreen_range, 0, 600);
				ImGui::SliderFloat("offscreen-size", &g_Options.esp_offscreen_size, 10, 60);
			}



		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));


		ImGui::BeginChild("Other", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::CheckboxEX("remove smoke", &g_Options.remove_smoke, "No smoke effect");
			ImGui::CheckboxEX("remove flash", &g_Options.remove_flash, "No flasheffect");
			if (g_Options.remove_flash)
				ImGui::SliderInt("flash-ammount", &g_Options.esp_flash_ammount, 0, 255);
			ImGui::CheckboxEX("remove scope", &g_Options.remove_scope, "Sniper crosshair");
			ImGui::CheckboxEX("remove vis-recoil", &g_Options.remove_visual_recoil, "No recoil");
			ImGui::CheckboxEX("remove sleeves", &g_Options.remove_sleeves, "Delete sleeves");

			ImGui::CheckboxEX("bloom effect", &g_Options.esp_bloom_effect, "CSGO bloom");
			if (g_Options.esp_bloom_effect)
			{
				ImGui::Text("bloom :");
				ImGui::SliderInt("  bloom factor", &g_Options.esp_bloom_factor, 0, 100);
				ImGui::SliderFloat("model ambient", &g_Options.esp_model_ambient, 0, 11.1f);
			}

			ImGui::SliderFloat("red", &g_Options.mat_ambient_light_r, 0, 1);
			ImGui::SliderFloat("green", &g_Options.mat_ambient_light_g, 0, 1);
			ImGui::SliderFloat("blue", &g_Options.mat_ambient_light_b, 0, 1);

			ImGui::Text("postprocessing:");
			ImGui::Checkbox("Nightmode", &g_Options.esp_nightmode);
			if (g_Options.esp_nightmode)
				ImGui::SliderFloat("Nightmode bright", &g_Options.esp_nightmode_bright, 0, 100);

			ImGui::CheckboxEX("fullbright", &g_Options.esp_fullbright, "Remove shadows");


		}
		ImGui::EndChild();

	}
	break;



	}
}

void RenderMiscTab()
{
	bool placeholder_true = true;

	auto& style = ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

	static int SubTabs;

	ImGui::SubTabButton("Main", &SubTabs, 0, 4);
	ImGui::SubTabButton("Other", &SubTabs, 1, 4);
	ImGui::SubTabButton("Skins", &SubTabs, 2, 4);
	ImGui::SubTabButton("Info", &SubTabs, 3, 4);

	switch (SubTabs)
	{
	case 0:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("General", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::CheckboxEX("anti-untrusted", &g_Options.misc_anti_untrusted, "Remove untrusted angles");
			ImGui::CheckboxEX("infinity duck", &g_Options.misc_infinity_duck, "Nolimit for duck");
			ImGui::CheckboxEX("anti-obs", &g_Options.misc_anti_screenshot, "No ESP on OBS and screenshot");
			ImGui::CheckboxEX("legit resolver", &g_Options.misc_legit_resolver, "Resolve legit aa");


			ImGui::CheckboxEX("legit antihit", &g_Options.misc_legit_antihit, "local legit AA");
			if (g_Options.misc_legit_antihit)
			{
				ImGui::CheckboxEX("lby based", &g_Options.misc_legit_antihit_lby, "LBY mode");
				ImGui::Text("switch side key :");
				ImGui::Hotkey("##3side", &g_Options.misc_legit_antihit_key, ImVec2(150, 20));

			}

			ImGui::Text("movement");
			ImGui::CheckboxEX("auto-jump", &g_Options.misc_bhop, "Bunnyhop");
			ImGui::CheckboxEX("auto-strafe", &g_Options.misc_autostrafe, "StrafeBot");
			ImGui::CheckboxEX("jump-throw", &g_Options.misc_jump_throw, "Jump Throw props"); // @blick1337
			ImGui::CheckboxEX("third-person", &g_Options.misc_thirdperson, "Local thirdperson");
			if (g_Options.misc_thirdperson)
			{
				ImGui::Text("thirdperson-key:");
				ImGui::Hotkey("##3rdkey", &g_Options.misc_thirdperson_key, ImVec2(150, 20));
				//	ImGui::SliderFloat("distance", &g_Options.misc_thirdperson_dist, 0.f, 150.f);
			}
			ImGui::CheckboxEX("inventory-access", &g_Options.misc_unlock_inventory, "change weapons in inventory");
			ImGui::CheckboxEX("left-knife", &g_Options.misc_left_knife, "Left hand if knife");
			ImGui::CheckboxEX("fake-fps", &g_Options.misc_fake_fps, "Boost FPS");
			ImGui::CheckboxEX("show-ranks", &g_Options.misc_showranks, "Show MM ranks");
			ImGui::CheckboxEX("clan-tag", &g_Options.misc_clantag, "Cheat tag");
			ImGui::CheckboxEX("chat-spam", &g_Options.misc_chat_spam, "Spam in chat");

			static const char* models[] = { "Default","Special Agent Ava "," FBI","Operator "," FBI SWAT","Markus Delrow "," FBI HRT","Michael Syfers "," FBI Sniper","B Squadron Officer "," SAS","Seal Team 6 Soldier "," NSWC SEAL","Buckshot "," NSWC SEAL","Lt. Commander Ricksaw "," NSWC SEAL","Third Commando Company "," KSK","'Two Times' McCoy "," USAF TACP","Dragomir "," Sabre","Rezan The Ready "," Sabre","'The Doctor' Romanov "," Sabre","Maximus "," Sabre","Blackwolf "," Sabre","The Elite Mr. Muhlik "," Elite Crew","Ground Rebel "," Elite Crew","Osiris "," Elite Crew","Prof. Shahmat "," Elite Crew","Enforcer "," Phoenix","Slingshot "," Phoenix","Soldier "," Phoenix" };

			ImGui::Combo("player t", &g_Options.playerModelT, models, IM_ARRAYSIZE(models));

			ImGui::Combo("player ct", &g_Options.playerModelCT, models, IM_ARRAYSIZE(models));


			ImGui::CheckboxEX("bullet-impact's", &g_Options.misc_bullet_impacts, "Bullet pos");
			ImGui::CheckboxEX("bullet-tracer", &g_Options.misc_bullet_tracer, "Bullet way");
			ImGui::CheckboxEX("hit-marker", &g_Options.misc_hitmarker, "Marker if hit");


		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));

		ImGui::BeginChild("Server", ImVec2(310, 360), true);
		{

			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::CheckboxEX("slow-walk", &g_Options.misc_slowwalk, "Slowmotion");
			if (g_Options.misc_slowwalk)
			{
				ImGui::Hotkey("##slow-walk key", &g_Options.misc_slowwalk_key, ImVec2(150, 20));
				ImGui::SliderInt("slow-walk speed %", &g_Options.misc_slowwalk_speed, 0, 99);
			}
			ImGui::CheckboxEX("fake-duck", &g_Options.misc_fakeduck, "Fakeduck");
			if (g_Options.misc_fakeduck)
			{
				ImGui::Text("fakeduck key :");
				ImGui::Hotkey("##fakeduck key", &g_Options.misc_fakeduck_key, ImVec2(150, 20));
				ImGui::SliderInt("fakeduck tick's", &g_Options.misc_fakeduck_ticks, 0, 16);
			}
			const char* fakelag_type[4] = { "default", "adaptive", "on peek", "switch" };
			ImGui::CheckboxEX("fakelag", &g_Options.misc_fakelag, "Fake lags");
			ImGui::CheckboxEX("Shot ignore", &g_Options.misc_fakelag_on_shot, "No fl on shot");
			if (g_Options.misc_fakelag)
			{
				ImGui::Combo("fakelag-type", &g_Options.misc_fakelag_type, fakelag_type, IM_ARRAYSIZE(fakelag_type));
				ImGui::SliderInt("fakelag ticks :", &g_Options.misc_fakelag_ticks, 0, 16);
			}
			ImGui::CheckboxEX("knife-bot", &g_Options.misc_knifebot, "Autoknife");
			if (g_Options.misc_knifebot)
			{
				ImGui::CheckboxEX("auto atack", &g_Options.misc_auto_knifebot, "Always attack");
				ImGui::Checkbox("360 atack[untrusted]", &g_Options.misc_knifebot_360);
			}
			ImGui::CheckboxEX("double-tap", &g_Options.exploit_doubletap, "Fast fire exploit");
			if (g_Options.exploit_doubletap)
			{
				ImGui::Hotkey("##rapidfire key", &g_Options.exploit_doubletap_key, ImVec2(150, 20));
				ImGui::CheckboxEX("hide-shots", &g_Options.exploit_hideshots, "hideshots exploit");
			}

		}
		ImGui::EndChild();

	}
	break;

	case 1:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("Windows", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			if (g_Options.misc_radar)
				ImGui::SliderFloat("radar-range", &g_Options.misc_radar_range, 200.f, 1600.f);




			ImGui::CheckboxEX("watermark", &g_Options.misc_watermark, "Cheat watermark");
			ImGui::CheckboxEX("spectator-list", &g_Options.misc_spectator_list, "Spectator list");
			ImGui::CheckboxEX("in-game radar", &g_Options.misc_engine_radar, "Csgo radar");
			ImGui::CheckboxEX("radar-window", &g_Options.misc_radar, "Cheat radar");
			ImGui::Text("other # 2 :");
			ImGui::Checkbox("Gravity ragdolls", &g_Options.misc_exaggerated_ragdolls);
			if (g_Options.misc_exaggerated_ragdolls)
				ImGui::SliderInt("ragdolls force :", &g_Options.misc_exaggerated_ragdolls_force, 1, 35);



		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));


		ImGui::BeginChild("Other", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			if (g_Options.misc_hitmarker)
				ImGui::SliderFloat("hit-marker size", &g_Options.misc_hitmarker_size, 1.f, 30.f);
			ImGui::CheckboxEX("hit-sound", &g_Options.misc_hitsound, "Sound if hit");
			const char* sound_type[5] = { "Arena switch", "Bank" ,"Hit", "Metalic", "Rifk" };
			ImGui::Combo("HitSound type", &g_Options.misc_hitsound_type, sound_type, IM_ARRAYSIZE(sound_type));

			ImGui::CheckboxEX("hit-effect", &g_Options.misc_hiteffect, "Effect if hit");
			if (g_Options.misc_hiteffect)
				ImGui::SliderFloat("hit-effect duration", &g_Options.misc_hiteffect_duration, 0.1f, 5.f);
			ImGui::Text("event logger");
			ImGui::CheckboxEX("event log's", &g_Options.misc_event_log, "CSGO event log");
			ImGui::Text("log's :");
			ImGui::CheckboxEX("player hit", &g_Options.event_log_hit, "Log it, if was hit");
			ImGui::CheckboxEX("player purchases", &g_Options.event_log_item, "Log purchases");
			ImGui::CheckboxEX("planting", &g_Options.event_log_plant, "Log it, if bomb has been planted");
			ImGui::CheckboxEX("defusing", &g_Options.event_log_defuse, "Log it, if bomb is defusing");


		}
		ImGui::EndChild();

	}
	break;

	case 2:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		if (k_skins.size() == 0) {
			initialize_kits();
		}
		auto& entries = g_Options.m_skins.m_items;

		static auto definition_vector_index = 0;



	}
	break;

	case 3:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));


		ImGui::BeginChild("Info", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Text("Coded by snake & ba1m0v ");
			ImGui::Text("snakeware beta for csgo v0.0014 ");

			if (ImGui::Button("DISCORD SERVER", ImVec2(176, 40)))
				ShellExecute(nullptr, TEXT("open"), TEXT("https://discord.gg/uW4437Q"), nullptr, nullptr, 1);

			if (ImGui::Button("VK GROUP", ImVec2(176, 40)))
				ShellExecute(nullptr, TEXT("open"), TEXT("https://vk.com/snakewarecheat"), nullptr, nullptr, 1);

			if (ImGui::Button("FORUM", ImVec2(176, 40)))
				ShellExecute(nullptr, TEXT("open"), TEXT("https://snakeware.xyz/"), nullptr, nullptr, 1);


		}
		ImGui::EndChild();



	}
	break;



	}

}
void RenderSkinsTab()
{


	ImGui::BeginChild("skins", ImVec2(740, 500), true);
	{
		ImGui::Columns(2, nullptr, false);
		{


		}
		ImGui::NextColumn();
		{

		}
		ImGui::Columns(1, nullptr, false);
	}
	ImGui::EndChild();
}
void RenderConfigTab()
{
	static int SubTabs;

	ImGui::SubTabButton("Configs", &SubTabs, 0, 2);
	ImGui::SubTabButton("Scripts", &SubTabs, 1, 2);

	switch (SubTabs)
	{
	case 0:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("Config", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Text("configuration system");
			static std::vector<std::string> configs;

			static auto load_configs = []() {
				std::vector<std::string> items = {};

				std::string path("C:\\snakeware");
				if (!fs::is_directory(path))
					fs::create_directories(path);

				for (auto& p : fs::directory_iterator(path))
					items.push_back(p.path().string().substr(path.length() + 1));

				return items;
			};

			static auto is_configs_loaded = false;
			if (!is_configs_loaded) {
				is_configs_loaded = true;
				configs = load_configs();
			}

			static std::string current_config;

			static char config_name[32];
			ImGui::PushItemWidth(250.f);






			if (ImGui::BeginCombo("select config", current_config.c_str()))
			{


				for (auto& config : configs) {
					if (ImGui::Selectable(config.c_str(), config == current_config)) {
						current_config = config;
					}
				}

				ImGui::EndCombo();
			}

			ImGui::InputText("##config_name", config_name, sizeof(config_name));
			if (ImGui::Button("create", ImVec2(250, 20))) {
				current_config = std::string(config_name);

				Config->Save(current_config + ".xyz");
				is_configs_loaded = false;
				memset(config_name, 0, 32);
			}

			if (ImGui::Button("refresh", ImVec2(250, 20)));
			is_configs_loaded = false;

			if (ImGui::Button("open settings folder", ImVec2(250, 20)))
				ShellExecuteA(0, "open", "C:/snakeware", NULL, NULL, SW_NORMAL);
			if (!current_config.empty()) {

				if (ImGui::Button("load", ImVec2(250, 20)))
					Config->Load(current_config);


				if (ImGui::Button("save", ImVec2(250, 20)))
					Config->Save(current_config);


				if (ImGui::Button("delete", ImVec2(250, 20)) && fs::remove("C:\\snakware\\" + current_config)) {
					current_config.clear();
					is_configs_loaded = false;
				}


			}



		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));

		ImGui::BeginChild("Colors", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::TextHeader("esp colors :");
			ImGui::ColorEdit4("ally visible", g_Options.color_esp_ally_visible);
			ImGui::ColorEdit4("enemy visible", g_Options.color_esp_enemy_visible);
			ImGui::ColorEdit4("ally occluded", g_Options.color_esp_ally_occluded);
			ImGui::ColorEdit4("enemy occluded", g_Options.color_esp_enemy_occluded);
			ImGui::ColorEdit4("sound esp", g_Options.color_sound_esp);
			ImGui::ColorEdit4("shot hitbox", g_Options.color_shot_hitboxes);
			ImGui::ColorEdit4("crosshair", g_Options.color_esp_crosshair);
			ImGui::TextHeader("glow colors :");
			ImGui::ColorEdit4("ally", g_Options.color_glow_ally);
			ImGui::ColorEdit4("enemy", g_Options.color_glow_enemy);
			ImGui::TextHeader("chams colors :");
			ImGui::ColorEdit4("all-visible", g_Options.color_chams_player_ally_visible);
			ImGui::ColorEdit4("all-invisible", g_Options.color_chams_player_ally_occluded);
			ImGui::ColorEdit4("enemy-visible", g_Options.color_chams_player_enemy_visible);
			ImGui::ColorEdit4("enemy-inivisble", g_Options.color_chams_player_enemy_occluded);

			ImGui::TextHeader("arms :");
			ImGui::ColorEdit4("arms color", g_Options.color_chams_arms_visible);
			ImGui::TextHeader("weapons chams :");
			ImGui::ColorEdit4("weapon color", g_Options.color_chams_weapons);
			ImGui::TextHeader("world :");
			ImGui::ColorEdit4("offscreen esp", g_Options.color_esp_offscreen);
			ImGui::ColorEdit4("hitmarker", g_Options.color_hitmarker);
			ImGui::ColorEdit4("bullet-tracer", g_Options.color_bullet_tracer);
			ImGui::ColorEdit4("molotov-timer", g_Options.color_molotov);

			ImGuiStyle& style = ImGui::GetStyle();

			ImGui::ColorEdit4("Menu color", g_Options.menu_color);


		}
		ImGui::EndChild();

	}
	break;
	case 1:
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("Scipts", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);
			
		}
		ImGui::EndChild();



	}
	break;



	}

	bool placeholder_true = true;

	auto& style = ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;


}




void Menu::Initialize()
{
	CreateStyle();

	_visible = true;
}

void Menu::Shutdown()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
	ImGui_ImplDX9_CreateDeviceObjects();
}

void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;

	RenderWatermark();





	if (!_visible) return;




	ImGui::SetNextWindowSizeConstraints(ImVec2(691, 520), ImVec2(691, 520));


	if (ImGui::Begin("coded by snake | ba1m0v", &_visible, ImGuiWindowFlags_NoTitleBar || ImGuiWindowFlags_NoScrollbar || ImGuiWindowFlags_BackForce)) {
		static char* menu_tab_names[] = { "RAGEBOT","LEGITBOT", "VISUALS", "OTHER" ,"SKINS", "CONFIG" };
		static int active_menu_tab = -1;

		ImGuiWindow* window = GImGui->CurrentWindow;

		// mat sneika shluha

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(4000, 4000), ImColor(0.f, 0.f, 0.f, 0.3f));


		ImGui::Image(smoke, ImVec2(window->Size.x, window->Size.y));

		ImGui::GetWindowDrawList()->AddRectFilled(window->Pos, window->Pos + window->Size, ImColor(0.14f, 0.13f, 0.14f, 0.85f));

		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(window->Pos, window->Pos + ImVec2(window->Size.x, 75), ImColor(0.11f, 0.1f, 0.11f, 1.0f), ImColor(0.11f, 0.1f, 0.11f, 1.0f), ImColor(0.11f, 0.1f, 0.11f, 0.4f), ImColor(0.11f, 0.1f, 0.11f, 0.4f));
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(window->Pos + ImVec2(212, 19), window->Pos + ImVec2(window->Size.x, 75), ImColor(0.0f, 0.0f, 0.0f, 0.15f), ImColor(0.0f, 0.0f, 0.0f, 0.15f), ImColor(0.0f, 0.0f, 0.0f, 0.00f), ImColor(0.0f, 0.0f, 0.0f, 0.00f));

		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(window->Pos, window->Pos + ImVec2(window->Size.x, 19), ImColor(0.11f, 0.10f, 0.11f, 0.7f), ImColor(0.11f, 0.10f, 0.11f, 0.7f), ImColor(0.16f, 0.15f, 0.16f, 0.8f), ImColor(0.16f, 0.15f, 0.16f, 0.8f));




		ImGui::GetWindowDrawList()->AddRectFilled(window->Pos + ImVec2(0, window->Size.y - 40), window->Pos + ImVec2(window->Size.x, window->Size.y), ImColor(0.0f, 0.0f, 0.0f, 0.15f));

		ImGui::GetWindowDrawList()->AddRectFilled(window->Pos + ImVec2(0, window->Size.y - 5), window->Pos + ImVec2(window->Size.x, window->Size.y), (ImColor(g_Options.menu_color[0], g_Options.menu_color[1], g_Options.menu_color[2], 0.8f)));
		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(window->Pos + ImVec2(0, window->Size.y - 25), window->Pos + ImVec2(window->Size.x, window->Size.y), ImColor(g_Options.menu_color[0], g_Options.menu_color[1], g_Options.menu_color[2], 0.0f), ImColor(g_Options.menu_color[0], g_Options.menu_color[1], g_Options.menu_color[2], 0.0f), ImColor(g_Options.menu_color[0], g_Options.menu_color[1], g_Options.menu_color[2], 0.1f), ImColor(g_Options.menu_color[0], g_Options.menu_color[1], g_Options.menu_color[2], 0.1f));


		ImGui::GetWindowDrawList()->AddTextBig(window->Pos + ImVec2(35, 30), ImColor(0.75f, 0.75f, 0.75f, 1.0f), "SNAKEWARE");

		const char* HeaderText_left = "Welcome back, User , Days left : 30";
		ImGui::GetWindowDrawList()->AddText(window->Pos + ImVec2(5, 2), ImColor(0.44f, 0.44f, 0.44f, 0.85f), HeaderText_left);

		const char* HeaderText_right = "Alpha build";
		ImGui::GetWindowDrawList()->AddText(window->Pos + ImVec2(3 + 620, 2), ImColor(0.44f, 0.44f, 0.44f, 0.85f), HeaderText_right);


		static auto ChildPose = [](int num) -> ImVec2 {
			return ImVec2(ImGui::GetWindowPos().x + 12 + (ImGui::GetWindowSize().x / 2 - 65) * num + 20 * num, ImGui::GetWindowPos().y + 42);
		};

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		_style.Colors[ImGuiCol_CheckMark] = ImVec4(g_Options.menu_color[0], g_Options.menu_color[1], g_Options.menu_color[2], 1.f);

		ImGui::TabButton("Aimbot", &active_menu_tab, 0, 6);
		ImGui::TabButton("Visuals", &active_menu_tab, 1, 6);
		ImGui::TabButton("Misc", &active_menu_tab, 2, 6);
		ImGui::TabButton("Files", &active_menu_tab, 3, 6);




		switch (active_menu_tab)
		{


		case 0: // ragebot
			RenderRageBotTab();
			break;
		case 1: // esp
			RenderEspTab();
			break;
		case 2: // misc
			RenderMiscTab();
			break;
		case 3: // files
			RenderConfigTab();
			break;
		}


	} ImGui::End();

}

void Menu::Toggle()
{
	_visible = !_visible;
}

void Menu::CreateStyle()
{
	// IMGUI STYLES //
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
	_style.Alpha = 1.0f;
	_style.WindowPadding = ImVec2(8, 8);
	_style.WindowRounding = 0.0f;
	_style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	_style.FramePadding = ImVec2(4, 3);
	_style.FrameRounding = 6.0f;
	_style.ChildRounding = 8.0f;
	_style.ItemSpacing = ImVec2(8, 9);
	_style.ItemInnerSpacing = ImVec2(4, 4);
	_style.TouchExtraPadding = ImVec2(0, 0);
	_style.IndentSpacing = 21.0f;
	_style.ColumnsMinSpacing = 6.0f;
	_style.ScrollbarSize = 10.0f;
	_style.ScrollbarRounding = 7.0f;
	_style.GrabMinSize = 10.0f;
	_style.GrabRounding = 6.0f;
	_style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	_style.DisplayWindowPadding = ImVec2(22, 22);
	_style.DisplaySafeAreaPadding = ImVec2(4, 4);
	_style.AntiAliasedLines = true;
	_style.CurveTessellationTol = 1.25f;

	static bool Textured3 = false;

	if (Menu::Get().smoke == nullptr && !Textured3)
	{
		D3DPRESENT_PARAMETERS pp = {};
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &texture2, 54284, 1000, 1000, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &Menu::Get().smoke);
		Textured3 = true;
	}

	// IMGUI COLORS //huehuehuehuehuehuehuehuehue

	static int hue = 140;
	ImVec4 col_text = ImColor(170, 170, 170);
	ImVec4 col_main = ImColor(36, 33, 36);
	ImVec4 col_back = ImColor(36, 33, 36);
	ImVec4 col_area = ImColor(56, 53, 56);

	_style.Colors[ImGuiCol_Text] = ImVec4(170 / 255.f, 170 / 255.f, 170 / 255.f, 1.00f);
	_style.Colors[ImGuiCol_TextDisabled] = ImVec4(70, 70, 70, 1.00f);
	_style.Colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.13f, 0.14f, 1.0f);
	_style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.11f, 0.1f, 0.11f, 0.6f);
	_style.Colors[ImGuiCol_Border] = ImVec4(0.27f, 0.27f, .27f, 1.00f);
	_style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	_style.Colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.13f, 0.14f, 1.f);
	_style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.13f, 0.14f, 0.8f);
	_style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.11, 0.11, 0.11, 1.f);
	_style.Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.15f, 0.16f, 1.0f);
	_style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.15f, 0.16f, 1.0f);
	_style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.15f, 0.16f, 1.0f);
	_style.Colors[ImGuiCol_MenuBarBg] = ImVec4(.52f, 0.f, 0.52f, .7f);;
	_style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
	_style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.50f, 0.50f, 0.50f, 0.30f); //main half
	_style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.14f, 0.13f, 0.14f, 0.8f); //main half
	_style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76, 0.08, 0.74, 1.f);
	_style.Colors[ImGuiCol_CheckMark] = ImVec4(g_Options.menu_color[0], g_Options.menu_color[1], g_Options.menu_color[2], 1.f);
	_style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.14f, 0.13f, 0.14f, 1.f); //main half
	_style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.76, 0.08, 0.74, 1.f);
	_style.Colors[ImGuiCol_Button] = ImVec4(0.14f, 0.13f, 0.14f, 1.f);
	_style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.14f, 0.13f, 0.14f, 0.7f);
	_style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.14f, 0.13f, 0.14f, 0.5f);
	_style.Colors[ImGuiCol_Header] = ImVec4(0.14f, 0.13f, 0.14f, 1.f);
	_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.17f, 0.18f, 1.f); // combobox hover
	_style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.16f, 0.15f, 0.16f, 1.f);
	_style.Colors[ImGuiCol_Column] = ImVec4(col_text.x, col_text.y, col_text.z, 0.32f);
	_style.Colors[ImGuiCol_ColumnHovered] = ImVec4(col_text.x, col_text.y, col_text.z, 0.78f);
	_style.Colors[ImGuiCol_ColumnActive] = ImVec4(col_text.x, col_text.y, col_text.z, 1.00f);
	_style.Colors[ImGuiCol_ResizeGrip] = ImVec4(col_main.x, col_main.y, col_main.z, 0.20f);
	_style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.78f);
	_style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	_style.Colors[ImGuiCol_PlotLines] = ImVec4(col_text.x, col_text.y, col_text.z, 0.63f);
	_style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	_style.Colors[ImGuiCol_PlotHistogram] = ImVec4(col_text.x, col_text.y, col_text.z, 0.63f);
	_style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	_style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(col_main.x, col_main.y, col_main.z, 0.43f);
	_style.Colors[ImGuiCol_PopupBg] = ImVec4(col_main.x, col_main.y, col_main.z, 0.92f);
	_style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	ImGui::GetStyle() = _style;
}
