#include "ragebot-autowall.h"
#include "../../../helpers/math.hpp"
#include "../../../valve_sdk/math/Vector.hpp"
#include "../../../options.hpp"
#include "../../../Protected/enginer.h"

#define HITGROUP_GENERIC   0
#define HITGROUP_HEAD      1
#define HITGROUP_CHEST     2
#define HITGROUP_STOMACH   3
#define HITGROUP_LEFTARM   4
#define HITGROUP_RIGHTARM  5
#define HITGROUP_LEFTLEG   6
#define HITGROUP_RIGHTLEG  7
#define HITGROUP_GEAR      10
#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1
#define DAMAGE_YES		2
#define DAMAGE_AIM		3
#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E'
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z'

returninfo CAutoWall::autowall(Vector start, Vector end, C_BasePlayer* from_ent, C_BasePlayer* to_ent, int hitgroup) {
	returninfo rt;

	fbdata bullet;
	bullet.start = start;
	bullet.end = end;
	bullet.pos = start;
	bullet.thickness = 0.f;
	bullet.walls = 3; // Кол-во стен автоволла

	Math_AngleVectors(Math_CalcAngle(start, end), bullet.dir);

	auto flt_player = CTraceFilterOneEntity();
	flt_player.pEntity = to_ent;

	auto flt_self = CTraceFilter();
	flt_self.pSkip = from_ent;

	if (to_ent)
		bullet.filter = &flt_player;
	else
		bullet.filter = &flt_self;

	auto wep = from_ent->m_hActiveWeapon();
	if (!wep)
		return rt;

	auto inf = wep->GetCSWeaponData();
	if (!inf)
		return rt;

	end = start + bullet.dir * (wep->IsKnife() ? 45.f : inf->flRange);
	bullet.damage = inf->iDamage;

	while (bullet.damage > 0 && bullet.walls > 0) {
		rt.walls = bullet.walls;

		Ray_t ray;
		ray.Init(bullet.pos, end);

		CTraceFilter filter;
		filter.pSkip = from_ent;

		g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &bullet.trace);
		this->clip_trace_to_player(bullet.pos, bullet.pos + bullet.dir * 40.f, to_ent, MASK_SHOT | CONTENTS_GRATE, bullet.filter, &bullet.trace);

		bullet.damage *= powf(inf->flRangeModifier, (bullet.trace.endpos - start).Length() / 500.f);

		if (bullet.trace.fraction == 1.f) {
			if (to_ent && hitgroup != INT_MIN) {
				this->scale_damage(to_ent, inf, hitgroup, bullet.damage);

				rt.damage = bullet.damage;
				rt.hitgroup = hitgroup;
				rt.end = bullet.trace.endpos;
				rt.ent = to_ent;
			}
			else {
				rt.damage = bullet.damage;
				rt.hitgroup = INT_MIN;
				rt.end = bullet.trace.endpos;
				rt.ent = nullptr;
			}
		}

		if (bullet.trace.hitgroup > 0 && bullet.trace.hitgroup <= 7) {
			if (to_ent && bullet.trace.hit_entity != to_ent) {
				rt.damage = INT_MIN;

				return rt;
			}
			C_BasePlayer* player = (C_BasePlayer*)bullet.trace.hit_entity;
			if (hitgroup != INT_MIN)
				scale_damage(player, inf, hitgroup, bullet.damage);
			else
				scale_damage(player, inf, bullet.trace.hitgroup, bullet.damage);

			rt.damage = bullet.damage;
			rt.hitgroup = bullet.trace.hitgroup;
			rt.end = bullet.trace.endpos;
			rt.ent = bullet.trace.hit_entity;

			break;
		}

		if (!this->handle_bullet_penetration(inf, bullet))
			break;

		rt.did_penetrate_wall = true;
	}

	rt.walls = bullet.walls;
	return rt;
}

void CAutoWall::clip_trace_to_player(Vector& start, Vector& end, C_BasePlayer* ent, unsigned int mask, ITraceFilter* filter, trace_t* trace) {
	if (!ent)
		return;

	Vector mins = ent->GetCollideable()->OBBMins();
	Vector maxs = ent->GetCollideable()->OBBMaxs();

	Vector dir(end - start);
	dir.Normalized();

	Vector
		center = (maxs + mins) / 2,
		pos(center + ent->m_vecOrigin());

	Vector to = pos - start;
	float range_along = dir.Dot(to);

	float range;
	if (range_along < 0.f)
		range = -to.Length();
	else if (range_along > dir.Length())
		range = -(pos - end).Length();
	else {
		auto ray(pos - (dir * range_along + start));
		range = ray.Length();
	}

	if (range <= 60.f) {
		trace_t newtrace;

		Ray_t ray;
		ray.Init(start, end);

		g_EngineTrace->ClipRayToEntity(ray, mask, ent, &newtrace);

		if (trace->fraction > newtrace.fraction)
			*trace = newtrace;
	}
}

void CAutoWall::scale_damage(C_BasePlayer* e, CCSWeaponInfo* weapon_info, int& hitgroup, float& current_damage) {
	bool has_heavy_armor = false;
	int armor_value = e->m_ArmorValue();

	auto is_armored = [&e, &hitgroup]()-> bool {
		C_BasePlayer* target_entity = e;
		switch (hitgroup) {
		case HITGROUP_HEAD:
			return target_entity->m_bHasHelmet();
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (hitgroup) {
	case HITGROUP_HEAD:
		current_damage *= has_heavy_armor ? 2.f : 4.f;
		break;
	case HITGROUP_STOMACH:
		current_damage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage *= 0.75f;
		break;
	default:
		break;
	}

	if (armor_value > 0 && is_armored()) {
		float bonus_value = 1.f, armor_bonus_ratio = 0.5f, armor_ratio = weapon_info->flArmorRatio / 2.f;

		if (has_heavy_armor) {
			armor_bonus_ratio = 0.33f;
			armor_ratio *= 0.5f;
			bonus_value = 0.33f;
		}

		auto new_damage = current_damage * armor_ratio;

		if (has_heavy_armor)
			new_damage *= 0.85f;

		if (((current_damage - (current_damage * armor_ratio)) * (bonus_value * armor_bonus_ratio)) > armor_value)
			new_damage = current_damage - (armor_value / armor_bonus_ratio);


		current_damage = new_damage;

	}
}

bool CAutoWall::handle_bullet_penetration(CCSWeaponInfo* inf, fbdata& bullet) {
	trace_t trace;

	static auto dbp = g_CVar->FindVar("ff_damage_bullet_penetration");

	auto sdata = g_PhysSurface->GetSurfaceData(bullet.trace.surface.surfaceProps);
	auto mat = sdata->game.material;

	auto sp_mod = sdata->game.flPenetrationModifier;
	auto dmg_mod = 0.f;
	auto pnt_mod = 0.f;

	auto solid_surf = (bullet.trace.contents >> 3)& CONTENTS_SOLID;
	auto light_surf = (bullet.trace.surface.flags >> 7)& SURF_LIGHT;

	if (bullet.walls <= 0
		|| inf->flPenetration <= 0.f
		|| !this->trace_to_exit(&bullet.trace, bullet.trace.endpos, bullet.dir, &trace)
		&& (g_EngineTrace->GetPointContents(bullet.trace.endpos, MASK_SHOT_HULL, NULL) & (MASK_SHOT_HULL)))
		return false;

	auto e_sdata = g_PhysSurface->GetSurfaceData(trace.surface.surfaceProps);
	auto e_mat = e_sdata->game.material;
	auto e_sp_mod = e_sdata->game.flPenetrationModifier;

	if (mat == CHAR_TEX_GRATE || mat == CHAR_TEX_GLASS)
	{
		pnt_mod = 3.f;
		dmg_mod = 0.05f;
	}
	else if (light_surf || solid_surf)
	{
		pnt_mod = 1.f;
		dmg_mod = 0.16f;
	}
	else if (mat == CHAR_TEX_FLESH)
	{
		pnt_mod = dbp->GetFloat();
		dmg_mod = 0.16f;
	}
	else
	{
		pnt_mod = (sp_mod + e_sp_mod) / 2.f;
		dmg_mod = 0.16f;
	}

	if (mat == e_mat)
	{
		if (e_mat == CHAR_TEX_CARDBOARD || e_mat == CHAR_TEX_WOOD)
			pnt_mod = 3.f;
		else if (e_mat == CHAR_TEX_PLASTIC)
			pnt_mod = 2.f;
	}

	auto thickness = (trace.endpos - bullet.trace.endpos).LengthSqr();
	auto modifier = fmaxf(0.f, 1.f / pnt_mod);

	auto lost_damage = fmaxf(
		((modifier * thickness) / 24.f)
		+ ((bullet.damage * dmg_mod)
			+ (fmaxf(3.75 / inf->flPenetration, 0.f) * 3.f * modifier)), 0.f);

	if (lost_damage > bullet.damage)
		return false;

	if (lost_damage > 0.f)
		bullet.damage -= lost_damage;

	if (bullet.damage < 1.f)
		return false;

	bullet.pos = trace.endpos;
	bullet.walls--;

	return true;
}

bool CAutoWall::trace_to_exit(trace_t* enter_trace, Vector& start, Vector& dir, trace_t* exit_trace) {
	auto end = Vector();
	auto distance = 0.f;
	auto distance_check = 23;
	auto first_contents = 0;

	do {
		distance += 4.f;
		end = start + dir * distance;

		if (!first_contents)
			first_contents = g_EngineTrace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		int point_contents = g_EngineTrace->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		if (!(point_contents & (MASK_SHOT_HULL | CONTENTS_HITBOX)) || point_contents & CONTENTS_HITBOX && point_contents != first_contents) {
			Vector new_end = end - (dir * 4.f);

			Ray_t ray;
			ray.Init(end, new_end);

			g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, nullptr, exit_trace);

			if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX) {
				Ray_t ray1;
				ray1.Init(start, end);

				CTraceFilter filter;
				filter.pSkip = exit_trace->hit_entity;

				g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, exit_trace);

				if (exit_trace->DidHit() && !exit_trace->startsolid)
					return true;

				continue;
			}
			C_BasePlayer* player = (C_BasePlayer*)enter_trace->hit_entity;
			if (exit_trace->DidHit() && !exit_trace->startsolid) {
				if (enter_trace->surface.flags & SURF_NODRAW || !(exit_trace->surface.flags & SURF_NODRAW)) {
					if (exit_trace->plane.normal.Dot(dir) <= 1.f)
						return true;

					continue;
				}

				if (this->is_breakable(player) && this->is_breakable(player))
					return true;

				continue;
			}

			if (exit_trace->surface.flags & SURF_NODRAW) {
				if (this->is_breakable(player) && this->is_breakable(player))
					return true;
				else if (!(enter_trace->surface.flags & SURF_NODRAW))
					continue;
			}

			if ((!enter_trace->hit_entity || enter_trace->hit_entity->EntIndex() == 0) && this->is_breakable(player)) {
				exit_trace = enter_trace;
				exit_trace->endpos = start + dir;
				return true;
			}

			continue;
		}

		distance_check--;
	} while (distance_check);

	return false;
}

bool CAutoWall::is_breakable(C_BasePlayer* e) {
	using func = bool(__fastcall*)(C_BasePlayer*);
	static auto fn = reinterpret_cast<func>(Utils::PatternScan("client.dll", "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE"));

	if (!e || !e->EntIndex())
		return false;

	auto take_damage{ (char*)((uintptr_t)e + *(size_t*)((uintptr_t)fn + 38)) };
	auto take_damage_backup{ *take_damage };

	auto* cclass = g_CHLClient->GetAllClasses();

	if ((cclass->m_pNetworkName[1]) != 'F'
		|| (cclass->m_pNetworkName[4]) != 'c'
		|| (cclass->m_pNetworkName[5]) != 'B'
		|| (cclass->m_pNetworkName[9]) != 'h')
		*take_damage = 2;

	bool breakable = fn(e);
	*take_damage = take_damage_backup;

	return breakable;
}

float CAutoWall::get_estimated_point_damage(Vector point) {
	if (!g_LocalPlayer)
		return -1.f;

	fbdata bullet;
	auto filter = CTraceFilter();
	filter.pSkip = g_LocalPlayer;

	bullet.filter = &filter;
	bullet.start = g_LocalPlayer->GetEyePos();
	bullet.end = point;
	bullet.pos = g_LocalPlayer->GetEyePos();
	Math_AngleVectors(Math_CalcAngle(bullet.start, point), bullet.dir);
	bullet.trace.startpos = bullet.start;
	bullet.trace.endpos = point;

	auto wep = g_LocalPlayer->m_hActiveWeapon();
	if (!wep)
		return -2.f;

	bullet.walls = 1;
	bullet.thickness = 0.f;

	auto inf = wep->GetCSWeaponData();
	if (!inf)
		return -3.f;

	bullet.damage = inf->iDamage;

	Ray_t ray;
	ray.Init(bullet.start, bullet.end);

	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &bullet.trace);

	if (bullet.trace.fraction == 1.f)
		return -4.f;

	if (this->handle_bullet_penetration(inf, bullet))
		return bullet.damage;

	return -5.f;
}

std::optional<CTraceSystem::wall_pen> CTraceSystem::wall_penetration(const Vector src, const Vector end,
	Animation* target, C_BasePlayer* override_player) const
{
	static CCSWeaponInfo override_gun{};
	override_gun.iDamage = 15000.f;
	override_gun.flRangeModifier = 1.0f;
	override_gun.flPenetration = 10.0f;
	override_gun.flArmorRatio = 2.0f;
	override_gun.flRange = 8192.f;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return std::nullopt;

	const auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon && !override_player)
		return std::nullopt;

	const auto data = override_player ? &override_gun :
		weapon->GetCSWeaponData();

	if (!data)
		return std::nullopt;

	std::optional<wall_pen> result = std::nullopt;

	if (!override_player)
		run_emulated(target, [&]() -> void
			{
				// setup trace filter.
				CTraceFilter filter;
				filter.pSkip = g_LocalPlayer;

				// run bullet simulation
				result = fire_bullet(data, src, end,
					&filter, target->player);
			});
	else
	{
		// setup trace filter.
		CTraceFilter filter;

		// run bullet simulation
		result = fire_bullet(data, src, end, &filter, override_player, true);
	}

	// filter low dmg.
	if (result.has_value() && result.value().damage < 1.f)
		return std::nullopt;

	// report result
	return result;
}

void CTraceSystem::run_emulated(Animation* target, const std::function<void()> fn)
{
	// backup player
	const auto backup_origin = target->player->m_vecOrigin();
	const auto backup_abs_origin = target->player->m_angAbsOrigin();
	const auto backup_obb_mins = target->player->GetCollideable()->OBBMins();
	const auto backup_obb_maxs = target->player->GetCollideable()->OBBMaxs();
	const auto backup_cache = target->player->get_bone_cache();

	// setup trace data
	target->player->m_vecOrigin() = target->origin;
	target->player->SetAbsOrigin(target->origin);
	target->player->GetCollideable()->OBBMins() = target->obb_mins;
	target->player->GetCollideable()->OBBMaxs() = target->obb_maxs;
	target->player->get_bone_cache() = reinterpret_cast<matrix3x4_t**>(target->bones);

	// run emulation
	fn();

	// restore trace data
	target->player->m_vecOrigin() = backup_origin;
	target->player->SetAbsOrigin(backup_abs_origin);
	target->player->GetCollideable()->OBBMins() = backup_obb_mins;
	target->player->GetCollideable()->OBBMaxs() = backup_obb_maxs;
	target->player->get_bone_cache() = backup_cache;
}

void clip_ray_to_player(Vector& src, Vector& end, const uint32_t mask, C_BasePlayer* player, CTraceFilter* filter, trace_t* t)
{
	if (filter && !filter->ShouldHitEntity(player, mask))
		return;

	trace_t t_new{};
	Ray_t r{};
	r.Init(src, end);

	g_EngineTrace->ClipRayToEntity(r, mask, player, &t_new);
	if (t_new.fraction < t->fraction)
		*t = t_new;
}

void ClipTraceToPlayers(const Vector& absStart, const Vector absEnd, unsigned int mask, ITraceFilter* filter, CGameTrace* tr)
{
	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	static DWORD dwAddress = (DWORD)Utils::PatternScan(GetModuleHandleA("client.dll"), "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10");

	if (!dwAddress)
		return;

	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, absEnd
		LEA		ECX, absStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}


std::optional<CTraceSystem::wall_pen> CTraceSystem::fire_bullet(CCSWeaponInfo* data, Vector src,
	const Vector pos, CTraceFilter* filter, C_BasePlayer* target, bool point)
{
	QAngle angles;
	Math::VectorAngles(pos - src, angles);

	if (angles.IsZero())
		return std::nullopt;

	Vector direction;
	Math::AngleVectors(angles, direction);

	if (!direction.IsValid())
		return std::nullopt;

	direction.Normalized();

	auto penetrate_count = 5;
	auto length = 0.f, damage = static_cast<float>(data->iDamage);
	trace_t enter_trace{};

	const auto start = src;

	while (penetrate_count > 0 && damage >= 1.0f)
	{
		const auto length_remaining = data->flRange - length;
		auto end = src + direction * length_remaining;

		Ray_t r;
		r.Init(src, end);
		g_EngineTrace->TraceRay(r, MASK_SHOT_HULL | CONTENTS_HITBOX, filter, &enter_trace);

		if (enter_trace.fraction == 1.f && !point)
			break;

		if (point && (enter_trace.fraction == 1.f ||
			(start - enter_trace.endpos).Length() > (start - pos).Length()))
			return wall_pen{
				scale_damage(target, damage, data->flArmorRatio, HITGROUP_HEAD, false),
				HITBOX_HEAD, HITGROUP_HEAD };

		auto end_extended = end + direction * 40.f;

		//clip_ray_to_player(src, end_extended, MASK_SHOT_HULL | CONTENTS_HITBOX, target, filter, &enter_trace);
		ClipTraceToPlayers(src, end_extended, MASK_SHOT_HULL | CONTENTS_HITBOX, filter, &enter_trace);

		length += enter_trace.fraction * length_remaining;
		damage *= std::powf(data->flRangeModifier, length * .002f);

		if (enter_trace.hitgroup <= 7 && enter_trace.hitgroup > 0)
		{
			if (!enter_trace.hit_entity || enter_trace.hit_entity != target)
				break;

			// we have reached our target!
			return wall_pen{
				scale_damage(target, damage, data->flArmorRatio, enter_trace.hitgroup, false),
				enter_trace.hitbox, enter_trace.hitgroup };
		}

		const auto enter_surface = g_PhysSurface->GetSurfaceData(enter_trace.surface.surfaceProps);

		if (!enter_surface || enter_surface->game.flPenetrationModifier < .1f)
			break;

		if (!handle_bullet_penetration(data, enter_trace, src, direction, penetrate_count, damage, data->flPenetration))
			break;
	}

	// nothing found
	return std::nullopt;
}

bool CTraceSystem::handle_bullet_penetration(CCSWeaponInfo* weapon_data, trace_t& enter_trace,
	Vector& eye_position, const Vector direction, int& penetrate_count, float& current_damage, const float penetration_power)
{
	static const auto ff_damage_reduction_bullets = g_CVar->FindVar("ff_damage_reduction_bullets");
	static const auto ff_damage_bullet_penetration = g_CVar->FindVar("ff_damage_bullet_penetration");

	const auto damage_reduction_bullets = ff_damage_reduction_bullets->GetFloat();
	const auto damage_bullet_penetration = ff_damage_bullet_penetration->GetFloat();

	trace_t exit_trace{};
	auto enemy = reinterpret_cast<C_BasePlayer*>(enter_trace.hit_entity);
	const auto enter_surface_data = g_PhysSurface->GetSurfaceData(enter_trace.surface.surfaceProps);
	const int enter_material = enter_surface_data->game.material;

	const auto enter_surf_penetration_modifier = enter_surface_data->game.flPenetrationModifier;
	float final_damage_modifier, combined_penetration_modifier;
	const bool is_solid_surf = enter_trace.contents >> 3 & CONTENTS_SOLID;
	const bool is_light_surf = enter_trace.surface.flags >> 7 & SURF_LIGHT;

	if ((!penetrate_count && !is_light_surf && !is_solid_surf && enter_material != char_tex_grate && enter_material != char_tex_glass)
		|| weapon_data->flPenetration <= 0.f
		|| (!trace_to_exit(enter_trace, exit_trace, enter_trace.endpos, direction, weapon_data->iDamage > 10000.f)
			&& !(g_EngineTrace->GetPointContents(enter_trace.endpos, MASK_SHOT_HULL) & MASK_SHOT_HULL)))
		return false;

	const auto exit_surface_data = g_PhysSurface->GetSurfaceData(exit_trace.surface.surfaceProps);
	const auto exit_material = exit_surface_data->game.material;
	const auto exit_surf_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	if (enter_material == char_tex_grate || enter_material == char_tex_glass)
	{
		combined_penetration_modifier = 3.f;
		final_damage_modifier = 0.05f;
	}
	else if (is_solid_surf || is_light_surf)
	{
		combined_penetration_modifier = 1.f;
		final_damage_modifier = 0.16f;
	}
	else if (enter_material == char_tex_flesh && (enemy->m_iTeamNum() == g_LocalPlayer->m_iTeamNum() && damage_reduction_bullets == 0.f))
	{
		if (damage_bullet_penetration == 0.f)
			return false;

		combined_penetration_modifier = damage_bullet_penetration;
		final_damage_modifier = 0.16f;
	}
	else
	{
		combined_penetration_modifier = (enter_surf_penetration_modifier + exit_surf_penetration_modifier) / 2.f;
		final_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == char_tex_cardboard || exit_material == char_tex_wood)
			combined_penetration_modifier = 3.f;
		else if (exit_material == char_tex_plastic)
			combined_penetration_modifier = 2.f;
	}

	auto thickness = (exit_trace.endpos - enter_trace.endpos).Length();
	thickness *= thickness;
	thickness *= fmaxf(0.f, 1.0f / combined_penetration_modifier);
	thickness /= 24.0f;

	const auto lost_damage = fmaxf(0.0f, current_damage * final_damage_modifier + fmaxf(0.f, 1.0f / combined_penetration_modifier)
		* 3.0f * fmaxf(0.0f, 3.0f / penetration_power) * 1.25f + thickness);

	if (lost_damage > current_damage)
		return false;

	if (lost_damage > 0.f)
		current_damage -= lost_damage;

	if (current_damage < 1.f)
		return false;

	eye_position = exit_trace.endpos;
	--penetrate_count;
	return true;
}

bool BreakableEntity(IClientEntity* entity)
{
	ClientClass* pClass = (ClientClass*)entity->GetClientClass();

	if (!pClass)
		return false;

	if (pClass == nullptr)
		return false;

	return pClass->m_ClassID == ClassId_CBreakableProp || pClass->m_ClassID == ClassId_CBreakableSurface;
}

bool CTraceSystem::trace_to_exit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, Vector direction, const bool is_local)
{
	Vector start, end;
	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
	int firstContents = 0;

	while (currentDistance <= maxDistance)
	{
		currentDistance += rayExtension;

		start = startPosition + direction * currentDistance;

		if (!firstContents)
			firstContents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		int pointContents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		if (!(pointContents & MASK_SHOT_HULL) || pointContents & CONTENTS_HITBOX && pointContents != firstContents)
		{
			end = start - (direction * rayExtension);

			Ray_t r;
			r.Init(start, end);
			CTraceFilter filter;
			filter.pSkip = g_LocalPlayer;
			g_EngineTrace->TraceRay(r, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &exitTrace);

			if (exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
			{
				Ray_t r;
				r.Init(start, startPosition);
				CTraceFilter filter;
				filter.pSkip = exitTrace.hit_entity;
				g_EngineTrace->TraceRay(r, MASK_SHOT_HULL, &filter, &exitTrace);


				if (exitTrace.DidHit() && !exitTrace.startsolid)
				{
					start = exitTrace.endpos;
					return true;
				}
				continue;
			}

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{

				if (BreakableEntity(enterTrace.hit_entity) && BreakableEntity(exitTrace.hit_entity))
					return true;

				if (enterTrace.surface.flags & SURF_NODRAW || !(exitTrace.surface.flags & SURF_NODRAW) && (exitTrace.plane.normal.Dot(direction) <= 1.f))
				{
					float multAmount = exitTrace.fraction * 4.f;
					start -= direction * multAmount;
					return true;
				}

				continue;
			}

			if (!exitTrace.DidHit() || exitTrace.startsolid)
			{
				if (enterTrace.DidHitNonWorldEntity() && BreakableEntity(enterTrace.hit_entity))
				{
					exitTrace = enterTrace;
					exitTrace.endpos = start + direction;
					return true;
				}

				continue;
			}
		}
	}

	return false;
}

float CTraceSystem::scale_damage(C_BasePlayer* target, float damage, const float weapon_armor_ratio, int hitgroup, bool is_zeus)
{
	const auto is_armored = [&]() -> bool
	{
		if (target->m_ArmorValue() > 0.f)
		{
			switch (hitgroup)
			{
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
				return true;
			case HITGROUP_HEAD:
				return target->m_bHasHelmet();
			default:
				break;
			}
		}

		return false;
	};

	if (!is_zeus)
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			if (target->m_bHasHeavyArmor())
				damage = (damage * 4.f) * .5f;
			else
				damage *= 4.f;
			break;
		case HITGROUP_STOMACH:
			damage *= 1.25f;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			damage *= .75f;
			break;
		default:
			break;
		}

	if (is_armored())
	{
		auto modifier = 1.f, armor_bonus_ratio = .5f, armor_ratio = weapon_armor_ratio * .5f;

		if (target->m_bHasHeavyArmor())
		{
			armor_bonus_ratio = 0.33f;
			armor_ratio = (weapon_armor_ratio * 0.5f) * 0.5f;
			modifier = 0.33f;
		}

		auto new_damage = damage * armor_ratio;

		if (target->m_bHasHeavyArmor())
			new_damage *= 0.85f;

		if ((damage - damage * armor_ratio) * (modifier * armor_bonus_ratio) > target->m_ArmorValue())
			new_damage = damage - target->m_ArmorValue() / armor_bonus_ratio;

		damage = new_damage;

	}

	return damage;
}