
#include "API.h"
#include "../hooks.hpp"
#include <intrin.h>  
#include <algorithm>

#include "../render.hpp"
#include "../menu.hpp"
#include "../options.hpp"
#include "../helpers/input.hpp"
#include "../helpers/utils.hpp"
#include "../features/bhop.hpp"
#include "../features/chams.hpp"

//LUA EXTENDS
#include "../Lua/API.h"


#include "../features/visuals.hpp"
#include "../features/glow.hpp"
#include "../conifg-system/config-system.h"
#include "../features/miscellaneous/miscellaneous.h"
#include "../engine-prediction/engine-prediction.h"
#include "../features/ragebot/animation-system/animation-system.h"
#include "../grenade-prediction/grenade-prediction.h"
#include "../features/legitbot/legitbot.h"
#include "../features/legitbot/legit-backtrack/legit-backtrack.h"
#include "../features/night-mode/night-mode.h"
#include "../features/ragebot/antihit/antihit.h"
#include "../features/player-hurt/player-hurt.h"
#include "../features/bullet-manipulation/bullet-event.h"
#include "../skin-changer/skin-changer.h"
#include "../features/ragebot/ragebot.h"
#include "../features/ragebot/netvar-compensation/netvar-comp.h"
#include "../features/event-logger/event-logger.h"
#include "../features/tickbase-shift/tickbase-exploits.h"
// material system
#include "../materials/Materials.h"
#include "../Protected/enginer.h"
#include "../Achievment_sys.h"
#include "../Lua/API.h"

namespace lua {
	bool g_unload_flag = false;
	lua_State* g_lua_state = NULL;
	c_lua_hookManager* hooks = new c_lua_hookManager();
	std::vector<bool> loaded;
	std::vector<std::string> scripts;
	std::vector<std::filesystem::path> pathes;
	std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> menu_items = {};

	int extract_owner(sol::this_state st) {
		sol::state_view lua_state(st);
		sol::table rs = lua_state["debug"]["getinfo"](2, "S");
		std::string source = rs["source"];
		std::string filename = std::filesystem::path(source.substr(1)).filename().string();
		return get_script_id(filename);
	}

	namespace ns_client {
		void set_event_callback(sol::this_state s, std::string eventname, sol::function func) {
			sol::state_view lua_state(s);
			sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
			std::string source = rs["source"];
			std::string filename = std::filesystem::path(source.substr(1)).filename().string();

			hooks->registerHook(eventname, get_script_id(filename), func);

		}

		void run_script(std::string scriptname) {
			load_script(get_script_id(scriptname));
		}

		void reload_active_scripts() {
			reload_all_scripts();
		}

		void refresh() {
			unload_all_scripts();
			refresh_scripts();
			load_script(get_script_id("autorun.lua"));
		}
	};

	namespace ns_config {
		/*
		config.get(key)
		Returns value of given key or nil if key not found.
		*/
		std::tuple<sol::object, sol::object, sol::object, sol::object> get(sol::this_state s, std::string key) {
			std::tuple<sol::object, sol::object, sol::object, sol::object> retn = std::make_tuple(sol::nil, sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.b)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.c)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, (int)(kv.second[0] * 255)), sol::make_object(s, (int)(kv.second[1] * 255)), sol::make_object(s, (int)(kv.second[2] * 255)), sol::make_object(s, (int)(kv.second[3] * 255)));

			for (auto kv : g_config.f)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);


			for (auto kv : g_config.i)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_b)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_f)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_i)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.i_s)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_b)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_f)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_i)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			for (auto kv : g_config.s_s)
				if (kv.first == key)
					retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

			return retn;
		}



		/*
		config.load()
		Loads selected config
		*/
		void load(std::string id) {
			Config->Load(id);
		}

		/*
		config.save()
		Saves selected config
		*/
		void save(std::string id) {
			Config->Save(id);
		}
	};

	namespace ns_cvar {
		ConVar* find_var(std::string name) {
			return g_CVar->FindVar(name.c_str());
		}

	};

	namespace ns_convar {
		int get_int(ConVar* c_convar) {
			return c_convar->GetInt();
		}

		float get_float(ConVar* c_convar) {
			return c_convar->GetFloat();
		}
		Color get_Color(ConVar* c_convar) {
			return c_convar->GetColor();
		}

		void set_int(ConVar* c_convar, int value) {
			c_convar->SetValue(value);
		}

		void set_float(ConVar* c_convar, float value) {
			c_convar->SetValue(value);
		}

		void set_char(ConVar* c_convar, std::string value) {
			c_convar->SetValue(value.c_str());
		}
	};

	namespace ns_engine {
		void client_cmd(std::string cmd) {
			g_EngineClient->ClientCmd(cmd.c_str());
		}
		void client_cmd_unrestricted(std::string cmd) {
			g_EngineClient->ClientCmd_Unrestricted(cmd.c_str());
		}
		void execute_client_cmd(std::string cmd) {
			g_EngineClient->ExecuteClientCmd(cmd.c_str());
		}

		player_info_t get_player_info(int ent) {
			player_info_t p;
			g_EngineClient->GetPlayerInfo(ent, &p);
			return p;
		}

		int get_player_for_user_id(int userid) {
			return g_EngineClient->GetPlayerForUserID(userid);
		}

		int get_local_player_index() {
			return g_EngineClient->GetLocalPlayer();
		}

		QAngle get_view_angles() {
			QAngle va;
			g_EngineClient->GetViewAngles(&va);
			return va;
		}

		void set_view_angles(QAngle va) {
			g_EngineClient->SetViewAngles(&va);
		}

		int get_max_clients() {
			return g_EngineClient->GetMaxClients();
		}

		bool is_in_game() {
			return g_EngineClient->IsInGame();
		}

		bool is_connected() {
			return g_EngineClient->IsConnected();
		}
	};

	namespace ns_entity {

		Vector get_bone_position(C_BasePlayer* entity, int bone) {
			return entity->GetBonePos(bone);
		}
		Vector get_eye_position(C_BasePlayer* entity) {
			return entity->GetEyePos();
		}
		bool is_visible_none(C_BasePlayer* entity) {
			return entity->IsVisible(true);
		}
		bool is_enemy(C_BasePlayer* entity) {
			return entity->IsEnemy();
		}
		bool is_dormant(C_BasePlayer* entity) {
			return entity->IsDormant();
		}
		bool is_weapon(C_BasePlayer* entity) {
			return entity->IsWeapon();
		}
		bool is_alive(C_BasePlayer* entity) {
			return entity->IsAlive();
		}
		bool is_player(C_BaseEntity* entity) {
			return entity->isPlayer();
		}
		C_BaseCombatWeapon* get_active_weapon(C_BasePlayer* entity) {
			return entity->m_hActiveWeapon();
		}
		CCSWeaponInfo* get_weapon_data(C_BasePlayer* entity) {
			return entity->m_hActiveWeapon()->GetCSWeaponData();
		}
		float get_inaccuracy(C_BasePlayer* entity) {
			return entity->m_hActiveWeapon()->GetInaccuracy();
		}
		Vector get_abs_origin(C_BasePlayer* entity) {
			return entity->m_angAbsOrigin();
		}
		CCSGOPlayerAnimState* get_animstate(C_BasePlayer* entity) {
			return entity->GetPlayerAnimState();
		}
		bool is_in_reload(C_BasePlayer* entity) {
			return entity->m_hActiveWeapon()->IsReloading();
		}
		QAngle get_aim_punch(C_BasePlayer* entity) {
			return entity->m_aimPunchAngle();
		}

		//NETVAR
		int get_health(C_BasePlayer* entity) {
			return entity->m_iHealth();
		}
		int get_body(C_BasePlayer* entity) {
			return entity->GetBody();
		}
		int get_hitboxSet(C_BasePlayer* entity) {
			return entity->m_nHitboxSet();
		}
		Vector get_origin(C_BasePlayer* entity) {
			return entity->m_vecOrigin();
		}
		QAngle get_aim_punch_angle(C_BasePlayer* entity) {
			return entity->m_aimPunchAngle();
		}


		//void set_pnetvar_int(Entity* entity, std::string class_name, std::string var_name, int offset, int var){}
		//void set_pnetvar_bool(Entity* entity, std::string class_name, std::string var_name, int offset, bool var){}
		//void set_pnetvar_float(Entity* entity, std::string class_name, std::string var_name, int offset, float var){}
		//void set_pnetvar_vector(Entity* entity, std::string class_name, std::string var_name, int offset, Vector var){}
	};


	void test_func() {
		for (auto hk : hooks->getHooks("on_test"))
		{
			try
			{
				auto result = hk.func();
				if (!result.valid()) {
					sol::error err = result;
				}
			}
			catch (const std::exception&)
			{

			}
		}
	}

	void init_state() {
#ifdef _DEBUG
		lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
		lua_writeline();
#endif
		lua::unload();
		g_lua_state = luaL_newstate();
		luaL_openlibs(g_lua_state);
	}

	void init_console() {
		std::string str;
		//printf("lua_ptr: 0x%x \n", &g_console);
		std::printf(">");
		while (!g_unload_flag && std::getline(std::cin, str))
		{
			try
			{
				bool tmp = luaL_dostring(g_lua_state, str.c_str());
				if (tmp) {
					std::printf("lua::Error: %s\n", lua_tostring(g_lua_state, -1));
				}
			}
			catch (const std::exception & Error)
			{
				std::printf("std::Error: %s\n", Error.what());
				//std::printf("lua::Error: %s\n", lua_tostring(g_lua_state, 0));
				//g_console.log("std::Error: %s", Error.what());
				//g_console.log("lua::Error: %s", lua_tostring(g_lua_state, 0));
			}
			std::printf(">");
		}
	}

	void unload() {
		if (g_lua_state != NULL) {
			lua_close(g_lua_state);
			g_lua_state = NULL;
		}
	}

	void load_script(int id) {
		if (id == -1)
			return;

		if (loaded.at(id))
			return;

		auto path = get_script_path(id);
		if (path == (""))
			return;

		sol::state_view state(g_lua_state);
		state.script_file(path, [](lua_State* me, sol::protected_function_result result) {
			if (!result.valid()) {
				sol::error err = result;
				//Utilities->Game_Msg(err.what());
				//printf("%s\n", err.what());
			}

			return result;
			});
		loaded.at(id) = true;
	}

	void unload_script(int id) {
		if (id == -1)
			return;

		if (!loaded.at(id))
			return;

		std::map<std::string, std::map<std::string, std::vector<MenuItem_t>>> updated_items;
		for (auto i : menu_items) {
			for (auto k : i.second) {
				std::vector<MenuItem_t> updated_vec;

				for (auto m : k.second)
					if (m.script != id)
						updated_vec.push_back(m);

				updated_items[k.first][i.first] = updated_vec;
			}
		}
		menu_items = updated_items;

		hooks->unregisterHooks(id);
		loaded.at(id) = false;
	}

	void reload_all_scripts() {
		for (auto s : scripts) {
			if (loaded.at(get_script_id(s))) {
				unload_script(get_script_id(s));
				load_script(get_script_id(s));
			}
		}
	}

	void unload_all_scripts() {
		for (auto s : scripts)
			if (loaded.at(get_script_id(s)))
				unload_script(get_script_id(s));
	}

	void refresh_scripts() {
		std::filesystem::path path;
		std::string path_cfg("C:\\snakeware");
		path = path_cfg;
		path /= "Lua";

		if (!std::filesystem::is_directory(path)) {
			std::filesystem::remove(path);
			std::filesystem::create_directory(path);
		}

		auto oldLoaded = loaded;
		auto oldScripts = scripts;

		loaded.clear();
		pathes.clear();
		scripts.clear();

		for (auto& entry : std::filesystem::directory_iterator((path))) {
			if (entry.path().extension() == (".lua")) {
				auto path = entry.path();
				auto filename = path.filename().string();

				bool didPut = false;
				int oldScriptsSize = 0;
				oldScriptsSize = oldScripts.size();
				if (oldScriptsSize < 0)
					continue;

				for (int i = 0; i < oldScriptsSize; i++) {
					if (filename == oldScripts.at(i)) {
						loaded.push_back(oldLoaded.at(i));
						didPut = true;
					}
				}

				if (!didPut)
					loaded.push_back(false);

				pathes.push_back(path);
				scripts.push_back(filename);
			}
		}
	}

	int get_script_id(std::string name) {
		int scriptsSize = 0;
		scriptsSize = scripts.size();
		if (scriptsSize <= 0)
			return -1;

		for (int i = 0; i < scriptsSize; i++) {
			if (scripts.at(i) == name)
				return i;
		}

		return -1;
	}

	int get_script_id_by_path(std::string path) {
		int pathesSize = 0;
		pathesSize = pathes.size();
		if (pathesSize <= 0)
			return -1;

		for (int i = 0; i < pathesSize; i++) {
			if (pathes.at(i).string() == path)
				return i;
		}

		return -1;
	}

	std::string get_script_path(std::string name) {
		return get_script_path(get_script_id(name));
	}

	std::string get_script_path(int id) {
		if (id == -1)
			return  "";

		return pathes.at(id).string();
	}

	void c_lua_hookManager::registerHook(std::string eventName, int scriptId, sol::protected_function func) {
		c_lua_hook hk = { scriptId, func };

		this->hooks[eventName].push_back(hk);
	}

	void c_lua_hookManager::unregisterHooks(int scriptId) {
		for (auto& ev : this->hooks) {
			int pos = 0;

			for (auto& hk : ev.second) {
				if (hk.scriptId == scriptId)
					ev.second.erase(ev.second.begin() + pos);

				pos++;
			}
		}
	}

	std::vector<c_lua_hook> c_lua_hookManager::getHooks(std::string eventName) {
		return this->hooks[eventName];
	}
};

