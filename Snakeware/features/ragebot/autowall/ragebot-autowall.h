#pragma once

#include "../../../valve_sdk/csgostructs.hpp"
#include "../../../helpers/math.hpp"
#include "../animation-system/animation-system.h"
#include <optional>
#pragma once


struct returninfo {
	int damage = -1;
	int hitgroup = -1;
	int walls = 4;
	bool did_penetrate_wall = false;
	float thickness = 1.f;

	IClientEntity* ent = nullptr;
	Vector end = Vector();
};

struct fbdata {
	Vector start = Vector();
	Vector end = Vector();
	Vector pos = Vector();
	Vector dir = Vector();

	ITraceFilter* filter = nullptr;
	trace_t trace;

	float thickness = 1.f;
	float damage = 1.f;
	int walls = 4;
};


class CAutoWall : public Singleton<CAutoWall>
{
public:
	float get_estimated_point_damage(Vector point);
	void  TraceLine(Vector & start, Vector & end, unsigned int mask, C_BasePlayer * ignore, trace_t * trace);
	bool CanHitFloatingPoint(const Vector & point, const Vector & source);
	returninfo autowall(Vector start, Vector end, C_BasePlayer* from_ent = nullptr, C_BasePlayer* to_ent = nullptr, int hitgroup = -1);

private:
	inline void Math_AngleVectors(const Vector& angles, Vector& forward)
	{
		float sp, sy, cp, cy;
		Math::SinCos(DEG2RAD(angles[1]), &sy, &cy);
		Math::SinCos(DEG2RAD(angles[0]), &sp, &cp);
		forward.x = cp * cy;
		forward.y = cp * sy;
		forward.z = -sp;
	}
	inline Vector Math_CalcAngle(Vector src, Vector dst)
	{
		auto ret = Vector();
		auto delta = src - dst;
		double hyp = delta.Length2D();
		ret.y = (atan(delta.y / delta.x) * 57.295779513082f);
		ret.x = (atan(delta.z / hyp) * 57.295779513082f);
		ret[2] = 0.00;

		if (delta.x >= 0.0)
			ret.y += 180.0f;

		return ret;
	}
	void clip_trace_to_player(Vector& start, Vector& end, C_BasePlayer* ent, unsigned int mask, ITraceFilter* filter, trace_t* trace);
	void scale_damage(C_BasePlayer* ent, CCSWeaponInfo* inf, int& hitgroup, float& damage);
	bool handle_bullet_penetration(CCSWeaponInfo* inf, fbdata& bullet);
	bool trace_to_exit(trace_t* enter_trace, Vector& start, Vector& dir, trace_t* exit_trace);
	bool is_breakable(C_BasePlayer* e);
};
inline CAutoWall* g_AutoWall;

#define char_tex_concrete 'C'
#define char_tex_metal 'M'
#define char_tex_dirt 'D'
#define char_tex_vent 'V'
#define char_tex_grate 'G'
#define char_tex_tile 'T'
#define char_tex_slosh 'S'
#define char_tex_wood 'W'
#define char_tex_computer 'P'
#define char_tex_glass 'Y'
#define char_tex_flesh 'F'
#define char_tex_snow 'N'
#define char_tex_plastic 'L'
#define char_tex_cardboard 'U'

class CTraceSystem
{
public:
	struct wall_pen
	{
		float damage;
		int hitbox;
		int32_t hitgroup;
	};

	std::optional<wall_pen> wall_penetration(Vector src, Vector end,
		Animation* target, C_BasePlayer* override_player = nullptr) const;

	static void run_emulated(Animation* target, std::function<void()> fn);

private:
	static std::optional<wall_pen> fire_bullet(CCSWeaponInfo* data, Vector src,
		Vector pos, CTraceFilter* filter, C_BasePlayer* target = nullptr, bool point = false);

	static bool handle_bullet_penetration(CCSWeaponInfo* weapon_data, trace_t& enter_trace,
		Vector& eye_position, Vector direction, int& penetrate_count,
		float& current_damage, float penetration_power);
	static bool trace_to_exit(trace_t& enter_trace, trace_t& exit_trace, Vector start_position, Vector direction, bool is_local = false);

	static float scale_damage(C_BasePlayer* target, float damage, float weapon_armor_ratio, int hitgroup, bool is_zeus);
};

inline CTraceSystem* g_TraceSystem;