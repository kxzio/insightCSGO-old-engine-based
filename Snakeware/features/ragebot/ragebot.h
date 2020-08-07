
#pragma once
#include "../../valve_sdk/csgostructs.hpp"
#include "../../engine-prediction/engine-prediction.h"
#include "lagcompensation/lag-compensation.h"
#include "animation-system/animation-system.h"
#include <time.h>

struct  CLastHitted  {
	int        hitbox, point_id;
	Animation* anims;
};

struct  AutostopInfo  {
	float call_time;
	bool  did_stop;
};

class   ShotSnapshot  { 
    public:
	C_BasePlayer* entity;
	Animation*    record;
	QAngle        eyeangles;
	Vector        impact, start;

	std::string  hitbox_where_shot, resolver;
	float        time , first_processed_time;
	bool         weapon_fire, bullet_impact;
	int          hitgroup_hit, damage, hitbox, backtrack;
	
	
	matrix3x4_t* pMat;
	std::string  get_info();
};


class RageBot : public Singleton<RageBot> { 
    public:
    void    CreateMove (CUserCmd* cmd); // Ragebot setup func.

	// Ragebot-helper's :
	void    BackupPlayer (Animation*);
	void    SetAnims     (Animation*);
	void    RestorePlayer(Animation*);
	bool    Hitchance    (Vector, bool, Animation*, int&);
	bool    IsAbleToShoot();
	void    DropTarget   ();
	int     target_index = -1;
	float   best_distance;
	bool    did_dt,aimbotted_in_current_tick,fired_in_that_tick;
	float   current_aim_simulationtime;
	int     current_minusticks;
	bool    lby_backtrack; //Why?
	bool    shot, Shooting;
	bool    hitchanced, fired;
	void    QuickStop ();
	void    QuickCrouch();
	bool    last_tick_shooted,target_lethal;
	bool    Shooted[65],Hitted[65];
	int     GetCurrentPriorityHitbox (C_BasePlayer* pEntity);
	bool    HeadAiming;

	// Some information.
	Animation           backup_anims [65];
	matrix3x4_t         BoneMatrix   [128];
	ShotSnapshot        last_hitted  [65];

	// Some vector,qangle and other class operator's
	Vector              current_aim_position, current_aim_player_origin;
	Vector              Angles;
	Vector              Target;
	Vector              HeadScan          (Animation* backshoot, int & hitbox, float & best_damage, float min_dmg);
	Vector              PrimaryScan       (Animation* anims, int & hitbox, float & simtime, float & best_damage, float min_dmg);
	std::vector<int>    GetHitboxesToScan (C_BasePlayer*);
	std::vector<Vector> GetMultipoints    (C_BasePlayer*, int, matrix3x4_t[128]);
	Vector              FullScan          (Animation* anims, int &hitbox, float &simtime, float &best_damage, float min_dmg);
	Vector              GetPoint          (C_BasePlayer * pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[128]);
	Vector              GetAimVector      (C_BasePlayer*, float &, Vector&, Animation*&, int&); //Repose aim to vector angles.
	Animation*          target_anims;
	QAngle              last_shot_angle;
	clock_t             last_shot_tick;

    private :
	CUserCmd*           CurrentCmd = nullptr;
};

extern       std::vector<ShotSnapshot> ShotSnapshots;
extern bool  CanHitHitbox(const Vector start, const Vector end, Animation* _animation, studiohdr_t* hdr, int box);

