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

        if(ImGui::ColorEdit4(label, &clr.x, show_alpha)) {
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

    for(auto i = 0; i < N; ++i) {
        if(ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) {
            activetab = i;
        }
        if(sameline && i < N - 1)
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
	case 0 : 
	{
		bool placeholder_true = true;

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

		ImGui::SetCursorPos(ImVec2(22, 100));

		ImGui::BeginChild("Ragebot", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Checkbox("Enabled", &g_Options.ragebot_enabled);
			ImGui::Checkbox("Hitchance const", &g_Options.ragebot_hitchance_consider);
			ImGui::Checkbox("Backtrack", &g_Options.ragebot_position_adj);
			ImGui::Checkbox("Back-shoot priority", &g_Options.ragebot_backshoot);
			ImGui::SliderInt("Max misses :", &g_Options.ragebot_max_miss, 0, 10);
			ImGui::Text("Key's");
			ImGui::Text("Force-baim key :");
			ImGui::Hotkey("##13Keybind", &g_Options.ragebot_baim_key, ImVec2(150, 20));
			ImGui::Text("Damage override key :");
			ImGui::Hotkey("##t4Keybind", &g_Options.ragebot_mindamage_override_key, ImVec2(150, 20));


		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));


		static int curGroup = 0;


		ImGui::BeginChild("Weapon", ImVec2(310, 118), true);
		{


			const char* weapon[] = { "Pistols", "Rifles", "Smg" , "Shotguns","Auto","Scout","Awp" };

			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Combo("Weapon", &curGroup, weapon, IM_ARRAYSIZE(weapon));
			switch (curGroup) {
			case 0:
				curGroup = WEAPON_GROUPS::PISTOLS; break;
			case 1:
				curGroup = WEAPON_GROUPS::RIFLES; break;
			case 2:
				curGroup = WEAPON_GROUPS::SHOTGUNS; break;
			case 3:
				curGroup = WEAPON_GROUPS::SCOUT; break;
			case 4:
				curGroup = WEAPON_GROUPS::AUTO; break;
			case 5:
				curGroup = WEAPON_GROUPS::AWP; break;
			case 6:
				curGroup = WEAPON_GROUPS::SMG; break;
			case 7:
				curGroup = WEAPON_GROUPS::UNKNOWN; break;
			}

			ImGui::SliderFloat("Min-damage", &g_Options.ragebot_mindamage[curGroup], 0, 120);
			ImGui::SliderFloat("Hit-chance", &g_Options.ragebot_hitchance[curGroup], 0, 99);

		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100 + 118 + 17 + 10));

		ImGui::BeginChild("Settings", ImVec2(310, 210), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Checkbox("Auto-fire", &g_Options.ragebot_autofire[curGroup]);
			ImGui::Checkbox("Delay shot", &g_Options.ragebot_delayshot[curGroup]);
			ImGui::Checkbox("Auto-scope", &g_Options.ragebot_autoscope[curGroup]);
			ImGui::Checkbox("Auto-stop", &g_Options.ragebot_autostop[curGroup]);
			ImGui::Checkbox("Auto-crouch", &g_Options.ragebot_autocrouch[curGroup]);
			ImGui::SliderFloat("Baim if hp lower than :", &g_Options.ragebot_baim_if_hp[curGroup], 0, 100);
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
			if (ImGui::BeginCombo("Baim-scan", "Select##baim", 0))
			{
				//prevValue = "Hitscan";
				const char* baim[] = { "chest", "stomach", "pelvis" };
				std::vector<std::string> vec;
				for (size_t i = 0; i < IM_ARRAYSIZE(baim); i++)
				{
					ImGui::Selectable(baim[i], &g_Options.ragebot_baimhitbox[i][curGroup], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
					if (g_Options.ragebot_baimhitbox[i][curGroup])
						vec.push_back(baim[i]);
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
			ImGui::SliderFloat("Point-scale", &g_Options.ragebot_pointscale[curGroup], 0, 100);
			ImGui::SliderFloat("Body-scale", &g_Options.ragebot_bodyscale[curGroup], 0, 100);
			ImGui::Text("Accuracy setting's :");
			ImGui::SliderFloat("Damage ovveride", &g_Options.ragebot_mindamage_override[curGroup], 0, 140);
			ImGui::Checkbox("Alternative-hitchance method", &g_Options.ragebot_alternative_hitchance[curGroup]);

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

				ImGui::Checkbox("Enabled##aa", &g_Options.antihit_enabled);
				ImGui::Checkbox("Stabilize lby", &g_Options.antihit_stabilize_lby);
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
			ImGui::Checkbox("Auto", &g_Options.legitbot_auto_weapon);



		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 , 100 + 118 + 17 + 10));


		auto settings = &g_Options.legitbot_items[weapon_index];


		ImGui::BeginChild("Settings", ImVec2(310, 210), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Text("General-legitbot");
			ImGui::Checkbox("Enabled", &settings->enabled);
			ImGui::Checkbox("Friendly-fire", &settings->deathmatch);
			ImGui::Checkbox("Backtrack", &settings->backtrack);
			ImGui::Checkbox("Silent-aim", &settings->silent);
			ImGui::Text("Check's");
			ImGui::Checkbox("Smoke-check", &settings->smoke_check);
			ImGui::Checkbox("Flash-check ", &settings->flash_check);
			ImGui::Checkbox("Jump-check", &settings->jump_check);
			if (weapon_index == WEAPON_AWP || weapon_index == WEAPON_SSG08 ||
				weapon_index == WEAPON_AUG || weapon_index == WEAPON_SG553) {
				ImGui::Checkbox("In-zoom check", &settings->only_in_zoom);
			}
			if (weapon_index == WEAPON_P250 ||
				weapon_index == WEAPON_USPS ||
				weapon_index == WEAPON_GLOCK ||
				weapon_index == WEAPON_FIVESEVEN ||
				weapon_index == WEAPON_TEC9 ||
				weapon_index == WEAPON_DEAGLE ||
				weapon_index == WEAPON_ELITE ||
				weapon_index == WEAPON_P2000) {
				ImGui::Checkbox("Auto-pistol", &settings->autopistol);
			}



		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));

		ImGui::BeginChild("Customization", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Checkbox("Only on key", &settings->on_key);

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

			ImGui::Checkbox("Auto recoil-control", &settings->rcs);
			ImGui::Combo("Recoil type", &settings->rcs_type, rcs_types, IM_ARRAYSIZE(rcs_types));

			ImGui::SliderInt("Rcs x: ", &settings->rcs_x, 0, 100);
			ImGui::SliderInt("Rcs y: ", &settings->rcs_y, 0, 100);
			ImGui::SliderInt("Rcs start bullet: ", &settings->rcs_start, 1, 29);
			ImGui::Text("Recoil custom setting's");
			ImGui::Checkbox("Rcs custom-fov", &settings->rcs_fov_enabled);
			if (settings->rcs_fov_enabled) {
				ImGui::SliderFloat("Rcs fov", &settings->rcs_fov, 0, 30);
			}
			ImGui::Checkbox("Rcs custom-smooth", &settings->rcs_smooth_enabled);
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
				ImGui::Checkbox("Triggerbot enabled", &settings->triggerbot_enable);
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
					ImGui::Checkbox("Hit head", &settings->triggerbot_head);
					ImGui::Checkbox("Hit body", &settings->triggerbot_body);
					ImGui::Checkbox("Hit other-hitboxes", &settings->triggerbot_other);
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

			ImGui::Checkbox("enabled", &g_Options.esp_enabled);
			ImGui::Checkbox("enemy only", &g_Options.esp_enemies_only);
			ImGui::Checkbox("player-boxes", &g_Options.esp_player_boxes);
			const char* box_type[4] = { "default", "outlined", "corner" , "corner-outlined" };
			if (g_Options.esp_player_boxes)
				ImGui::Combo("box-type", &g_Options.esp_player_boxes_type, box_type, IM_ARRAYSIZE(box_type));
			ImGui::Checkbox("player-names", &g_Options.esp_player_names);
			ImGui::Checkbox("player-health", &g_Options.esp_player_health);
			ImGui::Checkbox("player-armour", &g_Options.esp_player_armour);
			ImGui::Checkbox("player-weapon", &g_Options.esp_player_weapons);
			ImGui::Checkbox("player-snaplines", &g_Options.esp_player_snaplines);
			ImGui::Checkbox("player-skeleton", &g_Options.esp_player_skeleton);
			ImGui::Checkbox("player-ammo##1", &g_Options.esp_player_ammo);
			ImGui::Checkbox("player-head dot", &g_Options.esp_head_dot);




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
				ImGui::Checkbox("player", &g_Options.esp_player_flags_player);
				ImGui::Checkbox("scoped", &g_Options.esp_player_flags_scoped);
				ImGui::Checkbox("armour", &g_Options.esp_flags_armour);
				ImGui::Checkbox("defuse-kit", &g_Options.esp_flags_kit);
				ImGui::Checkbox("on move", &g_Options.esp_flags_move);
				ImGui::Checkbox("choking", &g_Options.esp_flags_choking);
				ImGui::Checkbox("flashed", &g_Options.esp_flags_flash);
			}
		

		}
		ImGui::EndChild();


		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100 + 118 + 17 + 10 + 50));

		ImGui::BeginChild("Model", ImVec2(310, 210 - 50 ), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Checkbox("chams enabled", &g_Options.chams_player_enabled);
			static const char* glow_type[] = { "outer", "cover", "inner" };
			const char* chams_type[5] = { "material", "flat", "glow-1" , "glow-2", "metallic" };
			if (g_Options.chams_player_enabled)
				ImGui::Combo("chams-type", &g_Options.chams_player_type, chams_type, IM_ARRAYSIZE(chams_type));
			ImGui::Checkbox("ignore wall", &g_Options.chams_player_ignorez);
			ImGui::Checkbox("ignore team", &g_Options.chams_player_enemies_only);
			ImGui::Text("arms-chams");
			ImGui::Checkbox("arms enabled", &g_Options.chams_arms_enabled);
			ImGui::Checkbox("no arms", &g_Options.misc_no_hands);
			if (g_Options.chams_arms_enabled && !g_Options.misc_no_hands)
				ImGui::Combo("arms-type", &g_Options.chams_arms_type, chams_type, IM_ARRAYSIZE(chams_type));

			ImGui::Checkbox("weapons enabled", &g_Options.chams_weapons);
			if (g_Options.chams_weapons)
				ImGui::Combo("gun-type", &g_Options.chams_weapons_type, chams_type, IM_ARRAYSIZE(chams_type));
				ImGui::Text("fake-chams");
				ImGui::Checkbox("fake-chams", &g_Options.chams_fake);
				if (g_Options.chams_fake)
				{
					ImGui::Combo("fake-model", &g_Options.chams_fake_types, chams_type, IM_ARRAYSIZE(chams_type));
				}

			ImGui::Text("player-glow");
			ImGui::Checkbox("glow enabled", &g_Options.glow_enabled);
			ImGui::Checkbox("glow player's", &g_Options.glow_players);
			ImGui::Checkbox("only enemy", &g_Options.glow_enemies_only);
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
				ImGui::Checkbox("nade prediction", &g_Options.esp_nade_prediction);
				ImGui::Checkbox("molotov timer", &g_Options.esp_molotov_timer);
				ImGui::Text("sound esp :");
				ImGui::Checkbox("sound esp", &g_Options.sound_esp);
				if (g_Options.sound_esp)
				{
					//ImGui::Checkbox("animated beam's (sexy)", &g_Options.sound_esp_animated);
					ImGui::SliderFloat("sound esp radius", &g_Options.sound_esp_radius, 5, 30);
					ImGui::SliderFloat("sound esp time", &g_Options.sound_esp_time, 0.3, 5);
				}

				ImGui::Text("other :");
				//	ImGui::Checkbox("draw bullet-tracer's", &g_Options.esp_bullet_impacts);
				ImGui::Checkbox("draw crosshair", &g_Options.esp_crosshair);
				ImGui::Checkbox("draw recoil-crosshair", &g_Options.esp_recoil_crosshair);
				ImGui::Checkbox("draw legitbot fov", &g_Options.esp_draw_fov);
				ImGui::Checkbox("draw angle's", &g_Options.esp_angle_lines);
				ImGui::Checkbox("draw shot-hitboxes", &g_Options.shot_hitboxes);
				if (g_Options.shot_hitboxes)
					ImGui::SliderFloat("shot-hitboxes duration", &g_Options.shot_hitboxes_duration, 0, 7);
				ImGui::Checkbox("draw offscreen player's", &g_Options.esp_offscreen);
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

				ImGui::Checkbox("remove smoke", &g_Options.remove_smoke);
				ImGui::Checkbox("remove flash", &g_Options.remove_flash);
				if (g_Options.remove_flash)
					ImGui::SliderInt("flash-ammount", &g_Options.esp_flash_ammount, 0, 255);
				ImGui::Checkbox("remove scope", &g_Options.remove_scope);
				ImGui::Checkbox("remove vis-recoil", &g_Options.remove_visual_recoil);
				ImGui::Checkbox("remove sleeves", &g_Options.remove_sleeves);

				ImGui::Checkbox("bloom effect", &g_Options.esp_bloom_effect);
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
				ImGui::Checkbox("nightmode", &g_Options.esp_nightmode);
				if (g_Options.esp_nightmode)
					ImGui::SliderInt("nightmode bright", &g_Options.esp_nightmode_bright, 0, 100);
				ImGui::Checkbox("fullbright", &g_Options.esp_fullbright);


			}
			ImGui::EndChild();

	}
	break;



	}
}


void RenderLegitTab() {
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
		ImGui::BeginChild("weapon's", ImVec2(280, 110), true);
		{
			ImGui::Text("weapons");
			
			if (ImGui::Combo(
				"##weapon_aimbot", &weapon_vector_index,
				[](void* data, int32_t idx, const char** out_text) {
				auto vec = reinterpret_cast<std::vector< WeaponName_t >*>(data);
				*out_text = vec->at(idx).name;
				return true;
			}, (void*)(&WeaponNames), WeaponNames.size())) {
				weapon_index = WeaponNames[weapon_vector_index].definition_index;
			}
			
			RenderCurrentWeaponButton(1337);
			ImGui::Checkbox("auto-weapon's", &g_Options.legitbot_auto_weapon);
			
		} ImGui::EndChild();
		auto settings = &g_Options.legitbot_items[weapon_index];
		ImGui::SetCursorPosX(+ 12);
		ImGui::BeginChild("general", ImVec2(280, 240), true);
		{

			ImGui::Text("general-legitbot");
			ImGui::Checkbox("enabled", &settings->enabled);
			ImGui::Checkbox("friendly-fire", &settings->deathmatch);
			ImGui::Checkbox("backtrack", &settings->backtrack);
			ImGui::Checkbox("silent-aim", &settings->silent);
			ImGui::Text("check's");
			ImGui::Checkbox("smoke-check", &settings->smoke_check);
			ImGui::Checkbox("flash-check ", &settings->flash_check);
			ImGui::Checkbox("jump-check", &settings->jump_check);
			if (weapon_index == WEAPON_AWP || weapon_index == WEAPON_SSG08 ||
				weapon_index == WEAPON_AUG || weapon_index == WEAPON_SG553) {
				ImGui::Checkbox("in-zoom check", &settings->only_in_zoom);
			}
			if (weapon_index == WEAPON_P250 ||
				weapon_index == WEAPON_USPS ||
				weapon_index == WEAPON_GLOCK ||
				weapon_index == WEAPON_FIVESEVEN ||
				weapon_index == WEAPON_TEC9 ||
				weapon_index == WEAPON_DEAGLE ||
				weapon_index == WEAPON_ELITE ||
				weapon_index == WEAPON_P2000) {
				ImGui::Checkbox("auto-pistol", &settings->autopistol);
			}


		} ImGui::EndChild();

		ImGui::SetCursorPosX(+12);

		ImGui::BeginChild("triggerbot", ImVec2(280, 140), true);
		{
			ImGui::Text("general-triggerbot");
			ImGui::Checkbox("triggerbot enabled", &settings->triggerbot_enable);
			if (settings->triggerbot_enable) {

				ImGui::Text("trigger");
				ImGui::SameLine();
				ImGui::Hotkey("##triggerkeybind", &g_Options.legitbot_trigger_key, ImVec2(150, 20));
			}
			ImGui::Text("trigger-bot setting's");
			{
				ImGui::SliderInt("delay : ", &settings->triggerbot_delay, 0, 250);
				ImGui::SliderFloat("hitchance :", &settings->triggerbot_hitchance, 0, 100);
			}
			ImGui::Text("trigger-bot hitboxe's");
			{
				ImGui::Checkbox("hit head", &settings->triggerbot_head);
				ImGui::Checkbox("hit body", &settings->triggerbot_body);
				ImGui::Checkbox("hit other-hitboxes", &settings->triggerbot_other);
			}
		}ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("setting's", ImVec2(220, 354), true);
		{
			ImGui::Checkbox("only on key", &settings->on_key);

			if (settings->on_key) {
				ImGui::Text("bind");
				ImGui::SameLine();
				ImGui::Hotkey("##keybind", &settings->key, ImVec2(150, 20));
			}


			ImGui::NextColumn();
			ImGui::Text("legitbot setting's");

			ImGui::Combo("aim-type", &settings->aim_type, aim_types, IM_ARRAYSIZE(aim_types));

			if (settings->aim_type == 0) {
				ImGui::Text("hitbox:");
				ImGui::Combo("##aimbot.hitbox", &settings->hitbox, hitbox_list, IM_ARRAYSIZE(hitbox_list));
			}

			ImGui::Combo("priority", &settings->priority, priorities, IM_ARRAYSIZE(priorities));

			ImGui::Combo("fov-type", &settings->fov_type, fov_types, IM_ARRAYSIZE(fov_types));
			ImGui::Combo("smooth-type", &settings->smooth_type, smooth_types, IM_ARRAYSIZE(smooth_types));
			if (settings->silent) {
				ImGui::SliderFloat("silent fov", &settings->silent_fov, 0, 40);
				settings->fov = 0;
			}
			else
			{
				ImGui::SliderFloat("fov", &settings->fov, 0, 40);
			}



			ImGui::SliderFloat("smooth", &settings->smooth, 0, 25);

			ImGui::Text("delay's");
			if (!settings->silent) {
				ImGui::SliderInt("shot delay", &settings->shot_delay, 0, 200);
			}

			ImGui::SliderInt("kill delay", &settings->kill_delay, 0, 1001);
			if (settings->backtrack)
				ImGui::SliderFloat("backtrack-time", &settings->backtrack_time, 0.f, 0.4f);

		} ImGui::EndChild();
		ImGui::SetCursorPosX(+298);
		ImGui::BeginChild("recoil-control", ImVec2(220, 140), true);
		{

			ImGui::Text("recoil setting's");
			
			ImGui::Checkbox("auto recoil-control", &settings->rcs);
			ImGui::Combo("recoil type", &settings->rcs_type, rcs_types, IM_ARRAYSIZE(rcs_types));

			ImGui::SliderInt("rcs x: ", &settings->rcs_x, 0, 100);
			ImGui::SliderInt("rcs y: ", &settings->rcs_y, 0, 100);
			ImGui::SliderInt("rcs start bullet: ", &settings->rcs_start, 1, 29);
			ImGui::Text("recoil custom setting's");
			ImGui::Checkbox("rcs custom-fov", &settings->rcs_fov_enabled);
			if (settings->rcs_fov_enabled) {
				ImGui::SliderFloat("rcs fov", &settings->rcs_fov, 0, 30);
			}
			ImGui::Checkbox("rcs custom-smooth", &settings->rcs_smooth_enabled);
			if (settings->rcs_smooth_enabled) {
				ImGui::SliderFloat("rcs smooth", &settings->rcs_smooth, 1, 25);
			}

		}
		ImGui::EndChild();

	
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

			ImGui::Checkbox("anti-untrusted", &g_Options.misc_anti_untrusted);
			ImGui::Checkbox("unlimited stamina on duck", &g_Options.misc_infinity_duck);
			ImGui::Checkbox("anti-obs/screenshot", &g_Options.misc_anti_screenshot);
			ImGui::Checkbox("legit resolver[debug]", &g_Options.misc_legit_resolver);


			ImGui::Checkbox("legit antihit", &g_Options.misc_legit_antihit);
			if (g_Options.misc_legit_antihit)
			{
				ImGui::Checkbox("lby based", &g_Options.misc_legit_antihit_lby);
				ImGui::Text("switch side key :");
				ImGui::Hotkey("##3side", &g_Options.misc_legit_antihit_key, ImVec2(150, 20));

			}

			ImGui::Text("movement");
			ImGui::Checkbox("auto-jump", &g_Options.misc_bhop);
			ImGui::Checkbox("auto-strafe", &g_Options.misc_autostrafe);
			ImGui::Checkbox("jump-throw", &g_Options.misc_jump_throw); // @blick1337
			ImGui::Checkbox("third-person", &g_Options.misc_thirdperson);
			if (g_Options.misc_thirdperson)
			{
				ImGui::Text("thirdperson-key:");
				ImGui::Hotkey("##3rdkey", &g_Options.misc_thirdperson_key, ImVec2(150, 20));
				//	ImGui::SliderFloat("distance", &g_Options.misc_thirdperson_dist, 0.f, 150.f);
			}

			ImGui::Checkbox("left-knife", &g_Options.misc_left_knife);
			ImGui::Checkbox("fake-fps", &g_Options.misc_fake_fps);
			ImGui::Checkbox("show-ranks", &g_Options.misc_showranks);
			ImGui::Checkbox("clan-tag", &g_Options.misc_clantag);
			ImGui::Checkbox("chat-spam", &g_Options.misc_chat_spam);


	

			ImGui::Checkbox("bullet-impact's", &g_Options.misc_bullet_impacts);
			ImGui::Checkbox("bullet-tracer", &g_Options.misc_bullet_tracer);
			ImGui::Checkbox("hit-marker", &g_Options.misc_hitmarker);


		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));

		ImGui::BeginChild("Server", ImVec2(310, 360), true);
		{
		
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Checkbox("slow-walk", &g_Options.misc_slowwalk);
			if (g_Options.misc_slowwalk)
			{
				ImGui::Hotkey("##slow-walk key", &g_Options.misc_slowwalk_key, ImVec2(150, 20));
				ImGui::SliderInt("slow-walk speed %", &g_Options.misc_slowwalk_speed, 0, 99);
			}
			ImGui::Checkbox("fake-duck", &g_Options.misc_fakeduck);
			if (g_Options.misc_fakeduck)
			{
				ImGui::Text("fakeduck key :");
				ImGui::Hotkey("##fakeduck key", &g_Options.misc_fakeduck_key, ImVec2(150, 20));
				ImGui::SliderInt("fakeduck tick's", &g_Options.misc_fakeduck_ticks, 0, 16);
			}
			const char* fakelag_type[4] = { "default", "adaptive", "on peek", "switch" };
			ImGui::Checkbox("fakelag", &g_Options.misc_fakelag);
			ImGui::Checkbox("disable on shot", &g_Options.misc_fakelag_on_shot);
			if (g_Options.misc_fakelag)
			{
				ImGui::Combo("fakelag-type", &g_Options.misc_fakelag_type, fakelag_type, IM_ARRAYSIZE(fakelag_type));
				ImGui::SliderInt("fakelag ticks :", &g_Options.misc_fakelag_ticks, 0, 16);
			}
			ImGui::Checkbox("knife-bot", &g_Options.misc_knifebot);
			if (g_Options.misc_knifebot)
			{
				ImGui::Checkbox("auto atack", &g_Options.misc_auto_knifebot);
				ImGui::Checkbox("360 atack[untrusted]", &g_Options.misc_knifebot_360);
			}
			ImGui::Checkbox("double-tap", &g_Options.exploit_doubletap);
			if (g_Options.exploit_doubletap)
			{
				ImGui::Hotkey("##rapidfire key", &g_Options.exploit_doubletap_key, ImVec2(150, 20));
				ImGui::Checkbox("hide-shots", &g_Options.exploit_hideshots);
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




			ImGui::Checkbox("watermark", &g_Options.misc_watermark);
			ImGui::Checkbox("spectator-list", &g_Options.misc_spectator_list);
			ImGui::Checkbox("in-game radar", &g_Options.misc_engine_radar);
			ImGui::Checkbox("radar-window", &g_Options.misc_radar);
			ImGui::Text("other # 2 :");
			ImGui::Checkbox("exaggerated-ragdolls", &g_Options.misc_exaggerated_ragdolls);
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
			ImGui::Checkbox("hit-sound", &g_Options.misc_hitsound);
			ImGui::Checkbox("hit-effect", &g_Options.misc_hiteffect);
			if (g_Options.misc_hiteffect)
				ImGui::SliderFloat("hit-effect duration", &g_Options.misc_hiteffect_duration, 0.1f, 5.f);
			ImGui::Text("event logger");
			ImGui::Checkbox("event log's", &g_Options.misc_event_log);
			ImGui::Text("log's :");
			ImGui::Checkbox("player hit", &g_Options.event_log_hit);
			ImGui::Checkbox("player purchases", &g_Options.event_log_item);
			ImGui::Checkbox("planting", &g_Options.event_log_plant);
			ImGui::Checkbox("defusing", &g_Options.event_log_defuse);


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

		ImGui::BeginChild("Skins", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Text("weapon's :");
			ImGui::SetCursorPosX(-7);
			ImGui::ListBoxHeader("##skinsss", ImVec2(20, 245));
			{
				for (size_t w = 0; w < k_weapon_names.size(); w++) {
					if (ImGui::Selectable(k_weapon_names[w].name, definition_vector_index == w)) {
						definition_vector_index = w;
					}
				}
			}
			ImGui::ListBoxFooter();


			if (ImGui::Button("update", ImVec2(245, 20)))
				g_ClientState->ForceFullUpdate();

		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(22 + 310 + 17 + 10, 100));


		ImGui::BeginChild("Other", ImVec2(310, 360), true);
		{
			ImGui::Text(" ");

			ImGui::SetCursorPosY(+15);

			ImGui::Text("skin's setting's :");
			auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
			selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
			selected_entry.definition_vector_index = definition_vector_index;
			ImGui::Checkbox("enabled", &selected_entry.enabled);
			ImGui::InputInt("seed", &selected_entry.seed);
			ImGui::InputInt("stat-trak", &selected_entry.stat_trak);
			ImGui::SliderFloat("wear", &selected_entry.wear, 0.f, 5);
			if (selected_entry.definition_index != GLOVE_T_SIDE)
			{
				ImGui::Combo("paint-kit", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = k_skins[idx].name.c_str();
						return true;
					}, nullptr, k_skins.size(), 10);
				selected_entry.paint_kit_index = k_skins[selected_entry.paint_kit_vector_index].id;
			}
			else
			{
				ImGui::Combo("paint-kit##2", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = k_gloves[idx].name.c_str();
						return true;
					}, nullptr, k_gloves.size(), 10);
				selected_entry.paint_kit_index = k_gloves[selected_entry.paint_kit_vector_index].id;
			}
			if (selected_entry.definition_index == WEAPON_KNIFE)
			{
				ImGui::Combo("knife", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = k_knife_names.at(idx).name;
						return true;
					}, nullptr, k_knife_names.size(), 5);
				selected_entry.definition_override_index = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;
			}
			else if (selected_entry.definition_index == GLOVE_T_SIDE)
			{
				ImGui::Combo("glove", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = k_glove_names.at(idx).name;
						return true;
					}, nullptr, k_glove_names.size(), 5);
				selected_entry.definition_override_index = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
			}
			else
			{
				static auto unused_value = 0;
				selected_entry.definition_override_vector_index = 0;
				ImGui::Combo("unavailable", &unused_value, "For knives or gloves\0");
			}
			ImGui::InputText("name-tag", selected_entry.custom_name, 32);




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
			ImGui::Text("esp colors :");
			ImGui::ColorEdit4("ally visible", g_Options.color_esp_ally_visible);
			ImGui::ColorEdit4("enemy visible", g_Options.color_esp_enemy_visible);
			ImGui::ColorEdit4("ally occluded", g_Options.color_esp_ally_occluded);
			ImGui::ColorEdit4("enemy occluded", g_Options.color_esp_enemy_occluded);
			ImGui::ColorEdit4("sound esp", g_Options.color_sound_esp);
			ImGui::ColorEdit4("shot hitbox", g_Options.color_shot_hitboxes);
			ImGui::ColorEdit4("crosshair", g_Options.color_esp_crosshair);
			ImGui::Text("glow colors :");
			ImGui::ColorEdit4("ally", g_Options.color_glow_ally);
			ImGui::ColorEdit4("enemy", g_Options.color_glow_enemy);
			ImGui::Text("chams colors :");
			ImGui::ColorEdit4("all-visible", g_Options.color_chams_player_ally_visible);
			ImGui::ColorEdit4("all-invisible", g_Options.color_chams_player_ally_occluded);
			ImGui::ColorEdit4("enemy-visible", g_Options.color_chams_player_enemy_visible);
			ImGui::ColorEdit4("enemy-inivisble", g_Options.color_chams_player_enemy_occluded);

			ImGui::Text("arms :");
			ImGui::ColorEdit4("arms color", g_Options.color_chams_arms_visible);
			ImGui::Text("weapons chams :");
			ImGui::ColorEdit4("weapon color", g_Options.color_chams_weapons);
			ImGui::Text("world :");
			ImGui::ColorEdit4("offscreen esp", g_Options.color_esp_offscreen);
			ImGui::ColorEdit4("sky color[nightmode]", g_Options.sky_color);
			ImGui::ColorEdit4("hitmarker", g_Options.color_hitmarker);
			ImGui::ColorEdit4("bullet-tracer", g_Options.color_bullet_tracer);
			ImGui::ColorEdit4("molotov-timer", g_Options.color_molotov);
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
    if(!_visible) return;

 
	ImGui::SetNextWindowSizeConstraints(ImVec2(691 , 520), ImVec2(691, 520));


	if (ImGui::Begin("coded by snake | ba1m0v", &_visible , ImGuiWindowFlags_NoTitleBar || ImGuiWindowFlags_NoScrollbar || ImGuiWindowFlags_BackForce )) {
		static char* menu_tab_names[] = { "RAGEBOT","LEGITBOT", "VISUALS", "OTHER" ,"SKINS", "CONFIG" };
		static int active_menu_tab = 0;

		ImGuiWindow* window = GImGui->CurrentWindow;

		// mat sneika shluha

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(4000, 4000), ImColor(0.f, 0.f, 0.f, 0.3f));


		ImGui::Image(smoke, ImVec2(window->Size.x, window->Size.y));

		ImGui::GetWindowDrawList()->AddRectFilled(window->Pos, window->Pos + window->Size, ImColor(0.14f, 0.13f, 0.14f, 0.7f));

		ImGui::GetWindowDrawList()->AddRectFilled(window->Pos, window->Pos + ImVec2(window->Size.x, 75), ImColor(0.11f, 0.1f, 0.11f, 1.0f));
		ImGui::GetWindowDrawList()->AddRectFilled(window->Pos + ImVec2(212, 19), window->Pos + ImVec2(window->Size.x, 75), ImColor(0.0f, 0.0f, 0.0f, 0.08f));





		ImGui::GetWindowDrawList()->AddRectFilled(window->Pos + ImVec2(0, window->Size.y - 40), window->Pos + ImVec2(window->Size.x, window->Size.y), ImColor(0.0f, 0.0f, 0.0f, 0.15f));

		ImGui::GetWindowDrawList()->AddTextBig(window->Pos + ImVec2(35, 30), ImColor(0.44f, 0.44f, 0.44f, 1.0f), "SNAKEWARE");
		ImGui::GetWindowDrawList()->AddText(window->Pos + ImVec2(1, 1), ImColor(0.44f, 0.44f, 0.44f, 0.85f), " snakeware::csgo beta [ 01.08.20 ]");


		static auto ChildPose = [](int num) -> ImVec2 {
			return ImVec2(ImGui::GetWindowPos().x + 12 + (ImGui::GetWindowSize().x / 2 - 65) * num + 20 * num, ImGui::GetWindowPos().y + 42);
		};

		auto& style = ImGui::GetStyle();
		float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
		
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
		D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, &smokeback, 130166, 1000, 1000, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &Menu::Get().smoke);
		Textured3 = true;
	}

	// IMGUI COLORS //huehuehuehuehuehuehuehuehue

	static int hue = 140;
	ImVec4 col_text = ImColor(112, 112, 112);
	ImVec4 col_main = ImColor(36, 33, 36);
	ImVec4 col_back = ImColor(36, 33, 36);
	ImVec4 col_area = ImColor(56, 53, 56);

	_style.Colors[ImGuiCol_Text] = ImVec4(112 / 255.f, 112 / 255.f, 112 / 255.f, 1.00f);
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
	_style.Colors[ImGuiCol_CheckMark] = ImVec4(0.76, 0.08, 0.74, 1.f);
	_style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.14f, 0.13f, 0.14f, 1.f); //main half
	_style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.76, 0.08, 0.74, 1.f);
	_style.Colors[ImGuiCol_Button] = ImVec4(0.14f, 0.13f, 0.14f, 1.f);
	_style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.14f, 0.13f, 0.14f, 0.7f);
	_style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.14f, 0.13f, 0.14f, 0.5f);
	_style.Colors[ImGuiCol_Header] = ImVec4(0.14f, 0.13f, 0.14f, 1.f);
	_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.76, 0.08, 0.74, 0.5f); // combobox hover
	_style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.76, 0.08, 0.74, 1.f);
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

