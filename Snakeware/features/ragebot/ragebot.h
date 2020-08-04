#pragma once
#include "../../valve_sdk/csgostructs.hpp"
#include "../../engine-prediction/engine-prediction.h"
#include "lagcompensation/lag-compensation.h"
#include "animation-system/animation-system.h"
#include <time.h>

class ShotSnapshot { public:
	C_BasePlayer* entity;
	std::string hitbox_where_shot;
	std::string resolver;
	float time;
	float first_processed_time;
	bool weapon_fire, bullet_impact;
	int hitgroup_hit;
	int damage;
	int hitbox;
	Animation* record;
	QAngle eyeangles;
	Vector  impact, start;
	int backtrack;
	matrix3x4_t* pMat;
	std::string get_info();
};
extern      std::vector<ShotSnapshot> ShotSnapshots;
extern bool CanHitHitbox(const Vector start, const Vector end, Animation* _animation, studiohdr_t* hdr, int box);

struct CLastHitted {
	int        hitbox, point_id;
	Animation* anims;
};
struct AutostopInfo {
	float call_time;
	bool  did_stop;
};
class RageBot : public Singleton<RageBot> { public:
	Animation backup_anims[65];
	void BackupPlayer(Animation*);
	void SetAnims(Animation*);
	void RestorePlayer(Animation*);
	Vector HeadScan(Animation* backshoot, int & hitbox, float & best_damage, float min_dmg);
	Vector PrimaryScan(Animation* anims, int & hitbox, float & simtime, float & best_damage, float min_dmg);
	std::vector<int> GetHitboxesToScan(C_BasePlayer*);
	std::vector<Vector> GetMultipoints(C_BasePlayer*, int, matrix3x4_t[128]);
	Vector FullScan(Animation* anims, int &hitbox, float &simtime, float &best_damage, float min_dmg);
	Vector GetPoint(C_BasePlayer * pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[128]);
	int GetTicksToShoot();
	int GetTicksToStop();
	bool HoldFiringAnimation();
	void FastStop();
	Vector GetVisualHitbox(C_BasePlayer* ent, int ihitbox);
	Vector GetAimVector(C_BasePlayer*, float &, Vector&, Animation*&, int&);
	bool Hitchance(Vector, bool, Animation*, int&);
	bool IsAbleToShoot();
	void DropTarget();
	int target_index = -1;
	float best_distance;
	bool did_dt;
	bool aimbotted_in_current_tick;
	bool fired_in_that_tick;
	float current_aim_simulationtime;
	int current_minusticks;
	Vector current_aim_position;
	bool lby_backtrack;
	Vector current_aim_player_origin;
	bool shot;
	bool Shooting;
	Vector Target;
	bool hitchanced;
	bool fired;
	Vector Angles;
	void CreateMove(CUserCmd* cmd);
	bool last_tick_shooted;
	bool target_lethal;
	bool Shooted[65];
	bool Hitted[65];
	matrix3x4_t BoneMatrix[128];
	int GetCurrentPriorityHitbox(C_BasePlayer* pEntity);
	bool HeadAiming;
	Animation* target_anims;
	ShotSnapshot last_hitted[65];
	QAngle last_shot_angle;
	float LerpTime();
	clock_t last_shot_tick;
	void DrawCapsule(Animation*);
private :
	CUserCmd* CurrentCmd = nullptr;
};


