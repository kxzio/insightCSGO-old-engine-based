#pragma once

#include "../singleton.hpp"

#include "../render.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"



struct Sounds
{
	DWORD time;
	Vector origin;
	bool enemy;
};

class SoundEsp : public Singleton<SoundEsp>
{
public:
	std::vector<Sounds> sound;

	void Update()
	{
		for (size_t i = 0; i < sound.size(); i++) {
			if (sound[i].time + 800 <= GetTickCount()) {
				sound.erase(sound.begin() + i);
			}
		}
	}

	void AddSound(Vector origin, bool is_enemy)
	{
		for (auto& soundinfo : sound) {
			if (soundinfo.origin == origin)
				return;
		}

		Sounds entry;
		entry.time = GetTickCount();
		entry.origin = origin;
		entry.enemy = is_enemy;

		sound.push_back(entry);
	}
};



class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
		} ctx;

		bool Begin(C_BasePlayer * pl);
		void RenderBox();
		bool IsChoke();
		void RenderFlags();
		void RenderName();
		void RenderWeaponName();
		void RenderAmmo();
		void RenderHealth();
		void RenderArmour();
		void RenderSnapline();
		void RenderSkeleton();

		void RenderHeadDot();
	
	};
	void RenderCrosshair();
	void RenderWeapon(C_BaseCombatWeapon* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void RenderSoundEsp();
	void RenderAnimatedSoundEsp();
	void ThirdPerson();
public:
	void DrawFOV();
	void RenderRecoilCrosshair();
	void DrawMolotov();
	void Indicators(); // Skeet
	void DrawScopeLines();
	void AddToDrawList();
	void Render();
	void RenderOffscreen();
	void PlayerChanger(ClientFrameStage_t stage);
};
