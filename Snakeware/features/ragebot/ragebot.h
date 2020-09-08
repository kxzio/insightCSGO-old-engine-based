#pragma once
#include "../../valve_sdk/csgostructs.hpp"
#include "../../engine-prediction/engine-prediction.h"
#include "lagcompensation/lag-compensation.h"
#include "animation-system/animation-system.h"
#include <time.h>
#include "../../helpers/unique_vector.h"

// Recode ragebot from supremacy & onetap.idb
// by @Snake


class Sphere;

// Hit-scanselection
enum HitscanMode : int {
	NORMAL = 0, LETHAL = 1, LETHAL2 = 3, PREFER = 4, PREFER_SAFEPOINT = 5,
};

// Anim backup & info
struct AnimationBackup_t {
	Vector           m_origin, m_abs_origin, m_velocity, m_abs_velocity;
	int              m_flags, m_eflags;
	float            m_duck, m_body;
	AnimationLayer   m_layers[13];
};

struct HitscanData_t {
	float  m_damage;
	Vector m_pos;
	int    m_hitbox;
	bool   m_safepoint;
	int    m_mode;

	__forceinline HitscanData_t() : m_damage{ 0.f }, m_pos{ }, m_hitbox{ }, m_safepoint{}, m_mode{} {}
};

struct HitscanBox_t {
	int         m_index;
	HitscanMode m_mode;
	bool        m_safepoint;

	__forceinline bool operator==(const HitscanBox_t& c) const {
		return m_index == c.m_index && m_mode == c.m_mode && m_safepoint == c.m_safepoint;
	}
};

class AimPlayer : public Singleton <AimPlayer> {
public:
	using hitboxcan_t = stdpp::unique_vector< HitscanBox_t >;

public:
	// essential data.
	C_BasePlayer* m_player;
	float	      m_spawn;

	// aimbot data.
	hitboxcan_t m_hitboxes;

	// resolve data.
	int       m_shots;
	int       m_missed_shots;
	float     m_delta;
	float	  m_last_resolve;
	bool      m_extending;

	float m_abs_angles;

	BoneArray* m_matrix;

public:

	void OnNetUpdate        (C_BasePlayer* player);
	void OnRoundStart       (C_BasePlayer* player);
	void SetupHitboxes      (Animation* record, bool history);
	bool SetupHitboxPoints  (Animation* record, BoneArray* bones, int index, std::vector< Vector >& points);
	bool GetBestAimPosition (Vector& aim, float& damage, int& hitbox, Animation* record, float& min_damage);

public:
	void reset() {
		m_player = nullptr;
		m_spawn = 0.f;
		m_shots = 0;
		m_missed_shots = 0;

		m_hitboxes.clear();
	}
};

class Aimbot {
private:
	struct target_t {
		C_BasePlayer* m_player;
		AimPlayer*    m_data;
	};

	struct knife_target_t {
		target_t  m_target;
	};

	struct table_t {
		uint8_t swing[2][2][2]; // [ first ][ armor ][ back ]
		uint8_t stab[2][2];		  // [ armor ][ back ]
	};

	const table_t m_knife_dmg{ { { { 25, 90 }, { 21, 76 } }, { { 40, 90 }, { 34, 76 } } }, { { 65, 180 }, { 55, 153 } } };

	std::array< QAngle, 12 > m_knife_ang{
		QAngle{ 0.f, 0.f, 0.f },   QAngle{ 0.f, -90.f, 0.f },   QAngle{ 0.f, 90.f, 0.f },   QAngle{ 0.f, 180.f, 0.f },
		QAngle{ -80.f, 0.f, 0.f }, QAngle{ -80.f, -90.f, 0.f }, QAngle{ -80.f, 90.f, 0.f }, QAngle{ -80.f, 180.f, 0.f },
		QAngle{ 80.f, 0.f, 0.f },  QAngle{ 80.f, -90.f, 0.f },  QAngle{ 80.f, 90.f, 0.f },  QAngle{ 80.f, 180.f, 0.f }
	};

public:
	std::array< AimPlayer, 64 > m_players;
	std::vector< AimPlayer* >   m_targets;

	AimPlayer* m_old_target;
	CUserCmd * mcmd;
	// target selection stuff.
	float m_best_dist;
	float m_best_fov;
	float m_best_damage;
	int   m_best_hp;
	float m_best_lag;
	float m_best_height;

	// found target stuff.
	C_BasePlayer* m_target;
	QAngle        m_angle;
	Vector        m_aim;
	float         m_damage;
	Animation*    m_record;

	int m_hitbox;
	bool m_stop;
	bool m_override_damage;
	bool m_force_body;
	bool m_shoot_next_tick;
	bool m_force_safepoint;
	BoneArray* m_current_matrix;

	std::vector<Sphere> m_current_sphere;
public:
	__forceinline void reset() {
		// reset aimbot data.
		init();

		// reset all players data.
		for (auto& p : m_players)
			p.reset();
	}

	__forceinline bool IsValidTarget(C_BasePlayer* player) {
		if (!player)
			return false;

		if (!player->IsPlayer())
			return false;

		if (!player->IsAlive())
			return false;

		if (player->IsLocalPlayer())
			return false;

		if (!player->IsEnemy())
			return false;

		return true;
	}

	bool CanHit(Vector start, Vector end, Animation* record, int box, bool in_shot = false, BoneArray* bones = nullptr);

public:
	// aimbot.
	void init();
	void StripAttack();
	void Think(CUserCmd * cmd);
	void find();
	bool CheckHitchance(C_BasePlayer* player, const QAngle& angle, Animation* record, int hitbox);
	bool SelectTarget(Animation* record, const Vector& aim, float damage);
	void apply();
	void NoSpread();

	// knifebot.
	void knife();
	bool CanKnife(Animations* record, QAngle angle, bool& stab);
	bool KnifeTrace(Vector dir, bool stab, CGameTrace* trace);
	bool KnifeIsBehind(Animations* record);
};