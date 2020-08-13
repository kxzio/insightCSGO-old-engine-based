#pragma once
#include "../../valve_sdk/csgostructs.hpp"
#include "../../engine-prediction/engine-prediction.h"
#include "lagcompensation/lag-compensation.h"
#include "animation-system/animation-system.h"
#include <time.h>



#define CHECK_VALID( _v ) 0
#define square( x ) ( x * x )
class RageBot : public Singleton <RageBot> {
public:
	struct AimInfo {
		AimInfo(const Vector position, const float damage, Animation* animation, const bool alt_attack,
			const Vector center, const float radius, const float rs, const int hitbox)
			: position(position), damage(damage), animation(animation), alt_attack(alt_attack),
			center(center), radius(radius), rs(rs), hitbox(hitbox) { }

		Vector     position{};
		float      damage{};
		Animation* animation{};
		bool       alt_attack{};
		Vector     center{};
		float      radius{}, rs{};
		int        hitbox{};
	};
	C_BaseCombatWeapon* local_weapon;
	QAngle              engine_angles;
	static void VectorSubtractForFOV(const Vector& a, const Vector& b, Vector& c) {
		CHECK_VALID(a);
		CHECK_VALID(b);
		c.x = a.x - b.x;
		c.y = a.y - b.y;
		c.z = a.z - b.z;
	}
	static void Normalize(Vector& vIn, Vector& vOut) {
		float flLen = vIn.Length();
		if (flLen == 0) {
			vOut.Init(0, 0, 1);
			return;
		}
		flLen = 1 / flLen;
		vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);
	}

	static float FovToPlayer(Vector AimPos);
	static bool  IsViable(C_BasePlayer* entity);
	static void  QuickStop(CUserCmd* pCmd);
	static bool CanShoot ();
	static void  CreateMove(C_BasePlayer* local, CUserCmd* cmd, bool& send_packet);
	static std::vector<AimInfo> select_multipoint(Animation* animation, int box);
	static std::optional<AimInfo> scan_record_gun(C_BasePlayer* local, Animation* animation);
	inline static std::optional<float> last_pitch = std::nullopt;
private:
	static std::optional<AimInfo> scan_record(C_BasePlayer* local, Animation* animation);
	static bool                   is_breaking_lagcomp(Animation* animation);
};
extern bool can_hit_hitbox(const Vector start, const Vector end, matrix3x4_t* bones, studiohdr_t* hdr, int box);