#include "ragebot-autowall.h"
#include "../../../helpers/math.hpp"
#include "../../../valve_sdk/math/Vector.hpp"
#include "../../../options.hpp"
#include "../../../Protected/enginer.h"

// Supreamcy autowall + skeet.idn & otv2 code
// Credit's : @Snake

// CharTex && Damage define's
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



bool IsValidHitgroup(int index) {
	if ((index >= HITGROUP_HEAD && index <= HITGROUP_RIGHTLEG) || index == HITGROUP_GEAR)
		return true;

	return false;
}

float WallPeneration::Scale(C_BasePlayer* player, float damage, float armor_ratio, int hitgroup) {

	bool  has_heavy_armor;
	int   armor;
	float heavy_ratio, bonus_ratio, ratio, new_damage;

	static auto is_armored = [](C_BasePlayer* player, int armor, int hitgroup) {
		// the player has no armor.
		if (armor <= 0)
			return false;

		// if the hitgroup is head and the player has a helment, return true.
		// otherwise only return true if the hitgroup is not generic / legs / gear.
		if (hitgroup == HITGROUP_HEAD && player->m_bHasHelmet())
			return true;

		else if (hitgroup >= HITGROUP_CHEST && hitgroup <= HITGROUP_RIGHTARM)
			return true;

		return false;
	};

	// check if the player has heavy armor, this is only really used in operation stuff.
	has_heavy_armor = player->m_bHasHeavyArmor();

	// scale damage based on hitgroup.
	switch (hitgroup) {
	case HITGROUP_HEAD:
		if (has_heavy_armor)
			damage = (damage * 4.f) * 0.5f;
		else
			damage *= 4.f;
		break;

	case HITGROUP_STOMACH:
		damage *= 1.25f;
		break;

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		damage *= 0.75f;
		break;

	default:
		break;
	}

	// grab amount of player armor.
	armor = player->m_ArmorValue();

	// check if the ent is armored and scale damage based on armor.
	if (is_armored(player, armor, hitgroup)) {
		heavy_ratio = 1.f;
		bonus_ratio = 0.5f;
		ratio = armor_ratio * 0.5f;

		// player has heavy armor.
		if (has_heavy_armor) {
			// calculate ratio values.
			bonus_ratio = 0.33f;
			ratio = armor_ratio * 0.25f;
			heavy_ratio = 0.33f;

			// calculate new damage.
			new_damage = (damage * ratio) * 0.85f;
		}

		// no heavy armor, do normal damage calculation.
		else
			new_damage = damage * ratio;

		if (((damage - new_damage) * (heavy_ratio * bonus_ratio)) > armor)
			new_damage = damage - (armor / bonus_ratio);

		damage = new_damage;
	}

	return std::floor(damage);
}
bool WallPeneration::IsBreakable(C_BasePlayer* pPlayer) {
	if (!pPlayer || !pPlayer->EntIndex())
		return false;

	static auto is_breakable_fn = reinterpret_cast<bool(__thiscall*)(C_BasePlayer*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 51 56 8B F1 85 F6 74 68"));

	const auto result = is_breakable_fn(pPlayer);
	auto ClassId = pPlayer->GetClientClass()->m_ClassID;
	if (!result &&
		(ClassId == 10 ||
			ClassId == 31 ||
			(ClassId == 11 && pPlayer->GetCollideable()->GetSolid() == 1)))
		return true;

	return result;
}

void UTIL_ClipTraceToPlayers(const Vector& start, const Vector& end, uint32_t mask, ITraceFilter* filter, CGameTrace* tr, float range) {
	static auto func = Utils::PatternScan(GetModuleHandleA("client.dll"), ("53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 0F 57 C9"));
	if (!func)
		return;

	__asm {
		mov  ecx, start
		mov	 edx, end
		push range
		push tr
		push filter
		push mask
		call func
		add	 esp, 16
	}
}

bool WallPeneration::TraceToExit(const Vector &start, const Vector& dir, Vector& out, CGameTrace* enter_trace, CGameTrace* exit_trace) {
	static CTraceFilter filter{};

	float  dist{};
	Vector new_end;
	int    contents, first_contents{};
	auto   sv_clip_penetration_traces_to_players = g_CVar->FindVar("sv_clip_penetration_traces_to_players");
	// max pen distance is 90 units.
	while (dist <= 90.f) {
		// step forward a bit.
		dist += 4.f;

		// set out pos.
		out = start + (dir * dist);

		if (!first_contents)
			first_contents = g_EngineTrace->GetPointContents(out, MASK_SHOT, nullptr);

		contents = g_EngineTrace->GetPointContents(out, MASK_SHOT, nullptr);

		if ((contents & MASK_SHOT_HULL) && (!(contents & CONTENTS_HITBOX) || (contents == first_contents)))
			continue;

		// move end pos a bit for tracing.
		new_end = out - (dir * 4.f);
		Ray_t r;
		r.Init(out, new_end);

		Ray_t r2;
		r2.Init(out, start);
		// do first trace.
		g_EngineTrace->TraceRay(r, MASK_SHOT, nullptr, exit_trace);

		// note - dex; this is some new stuff added sometime around late 2017 ( 10.31.2017 update? ).
		if (sv_clip_penetration_traces_to_players->GetInt())
			UTIL_ClipTraceToPlayers(out, new_end, MASK_SHOT, nullptr, exit_trace, -60.f);

		// we hit an ent's hitbox, do another trace.
		if (exit_trace->startsolid && (exit_trace->surface.flags & SURF_HITBOX)) {
			filter.pSkip = exit_trace->hit_entity;

			g_EngineTrace->TraceRay(r2, MASK_SHOT_HULL, (ITraceFilter *)&filter, exit_trace);

			if (exit_trace->DidHit() && !exit_trace->startsolid) {
				out = exit_trace->endpos;
				return true;
			}

			continue;
		}

		if (!exit_trace->DidHit() || exit_trace->startsolid) {
			if (IsBreakable(enter_trace->hit_entity)) {
				*exit_trace = *enter_trace;
				exit_trace->endpos = start + dir;
				return true;
			}

			continue;
		}

		if ((exit_trace->surface.flags & SURF_NODRAW)) {
			// note - dex; ok, when this happens the game seems to not ignore world?
			if (IsBreakable(exit_trace->hit_entity) && IsBreakable(enter_trace->hit_entity)) {
				out = exit_trace->endpos;
				return true;
			}

			if (!(enter_trace->surface.flags & SURF_NODRAW))
				continue;
		}

		if (exit_trace->plane.normal.Dot(dir) <= 1.f) {
			out -= (dir * (exit_trace->fraction * 4.f));
			return true;
		}
	}

	return false;
}


void WallPeneration::ClipTraceToPlayer(const Vector& start, const Vector& end, uint32_t mask, CGameTrace* tr, C_BasePlayer* player, float min) {
	Vector     pos, to, dir, on_ray;
	float      len, range_along, range;
	Ray_t      ray;
	CGameTrace new_trace;

	// reference: https://github.com/alliedmodders/hl2sdk/blob/3957adff10fe20d38a62fa8c018340bf2618742b/game/shared/util_shared.h#L381

	// set some local vars.
	pos = player->m_vecOrigin() + ((player->m_vecMins() + player->m_vecMaxs()) * 0.5f);
	to = pos - start;
	dir = start - end;
	len = dir.Normalize();
	range_along = dir.Dot(to);

	// off start point.
	if (range_along < 0.f)
		range = -(to).Length();

	// off end point.
	else if (range_along > len)
		range = -(pos - end).Length();

	// within ray bounds.
	else {
		on_ray = start + (dir * range_along);
		range = (pos - on_ray).Length();
	}

	if (range <= 60.f) {
		// clip to player.
		ray.Init(start, end);
		g_EngineTrace->ClipRayToEntity(ray, mask, player, &new_trace);

		if (tr->fraction > new_trace.fraction)
			*tr = new_trace;
	}
}

bool WallPeneration::Run(PenetrationInput_t* in, PenetrationOutput_t* out) {
	static CTraceFilterSimple filter{};

	int			  pen{ 4 }, enter_material, exit_material;
	float		  damage, penetration, penetration_mod, player_damage, remaining, trace_len{}, total_pen_mod, damage_mod, modifier, damage_lost;
	surfacedata_t *enter_surface, *exit_surface;
	bool		  nodraw, grate;
	Vector		  start, dir, end, pen_end;
	CGameTrace	  trace, exit_trace;
	C_BaseCombatWeapon  * oWeapon;
	CCSWeaponInfo       * oWeaponInfo;


	// if we are tracing from our local player perspective.
	if (in->m_from == g_LocalPlayer) {
		oWeapon     = g_LocalPlayer->m_hActiveWeapon();
		oWeaponInfo = g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData();
		start       = g_LocalPlayer->GetShootPos();
	}

	// not local player.
	else {
		oWeapon = in->m_from->m_hActiveWeapon();
		if (!oWeapon)
			return false;

		// get weapon info.
		// maybe wrong.
		oWeaponInfo = oWeapon->GetCSWeaponData();
		if (!oWeaponInfo)
			return false;

		// set trace start.
		start = in->m_from->GetShootPos();
	}

	// get some weapon data.
	damage = (float)oWeaponInfo->iDamage;
	penetration = oWeaponInfo->flPenetration;

	// used later in calculations.
	penetration_mod = std::max(0.f, (3.f / penetration) * 1.25f);

	// get direction to end point.
	dir = (in->m_pos - start).Normalized();

	// setup trace filter for later.
	filter.SetPassEntity(in->m_from);
	// filter.SetPassEntity2( nullptr );

	while (damage > 0.f) {
		// calculating remaining len.
		remaining = oWeaponInfo->flRange - trace_len;

		// set trace end.
		end = start + (dir * remaining);

		// setup ray and trace.
		// TODO; use UTIL_TraceLineIgnoreTwoEntities?
		Ray_t rRay;
		rRay.Init(start, end);
		g_EngineTrace->TraceRay(rRay, MASK_SHOT | CONTENTS_HITBOX, (ITraceFilter *)&filter, &trace);

		// we didn't hit anything.
		if (trace.fraction == 1.f)
			return false;

		// check for player hitboxes extending outside their collision bounds.
		// if no target is passed we clip the trace to a specific player, otherwise we clip the trace to any player.
		if (in->m_target)
			ClipTraceToPlayer(start, end + (dir * 40.f), MASK_SHOT | CONTENTS_HITBOX, &trace, in->m_target, -60.f);

		else
			UTIL_ClipTraceToPlayers(start, end + (dir * 40.f), MASK_SHOT | CONTENTS_HITBOX, (ITraceFilter*)&filter, &trace, -60.f);

		// calculate damage based on the distance the bullet traveled.
		trace_len += trace.fraction * remaining;
		damage *= std::pow(oWeaponInfo->flRangeModifier, trace_len / 500.f);

		// if a target was passed.
		if (in->m_target) {

			// validate that we hit the target we aimed for.
			if (trace.hit_entity && trace.hit_entity == in->m_target && IsValidHitgroup(trace.hitgroup)) {
				int group = (oWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_ZEUS) ? HITGROUP_GENERIC : trace.hitgroup;

				// scale damage based on the hitgroup we hit.
				player_damage = Scale(in->m_target, damage, oWeaponInfo->flArmorRatio, group);

				// set result data for when we hit a player.
				out->m_pen = pen != 4;
				out->m_hitgroup = group;
				out->m_damage = player_damage;
				out->m_target = in->m_target;

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}
		}

		// no target was passed, check for any player hit or just get final damage done.
		else {
			out->m_pen = pen != 4;

			// todo - dex; team checks / other checks / etc.
			if (trace.hit_entity && trace.hit_entity->IsPlayer() && IsValidHitgroup(trace.hitgroup)) {
				int group = (oWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_ZEUS) ? HITGROUP_GENERIC : trace.hitgroup;

				player_damage = Scale(trace.hit_entity->Ass< C_BasePlayer* >(), damage, oWeaponInfo->flArmorRatio, group);

				// set result data for when we hit a player.
				out->m_hitgroup = group;
				out->m_damage   = player_damage;
				out->m_target   = trace.hit_entity->Ass< C_BasePlayer* >();

				// non-penetrate damage.
				if (pen == 4)
					return player_damage >= in->m_damage;

				// penetration damage.
				return player_damage >= in->m_damage_pen;
			}

			// if we've reached here then we didn't hit a player yet, set damage and hitgroup.
			out->m_damage = damage;
		}

		// don't run pen code if it's not wanted.
		if (!in->m_can_pen)
			return false;

		// get surface at entry point.
		enter_surface = g_PhysSurface->GetSurfaceData(trace.surface.surfaceProps);

		// this happens when we're too far away from a surface and can penetrate walls or the surface's pen modifier is too low.
		if ((trace_len > 3000.f && penetration) || enter_surface->game.flPenetrationModifier < 0.1f)
			return false;

		// store data about surface flags / contents.
		nodraw = (trace.surface.flags & SURF_NODRAW);
		grate = (trace.contents & CONTENTS_GRATE);

		// get material at entry point.
		enter_material = enter_surface->game.material;

		// note - dex; some extra stuff the game does.
		if (!pen && !nodraw && !grate && enter_material != CHAR_TEX_GRATE && enter_material != CHAR_TEX_GLASS)
			return false;

		// no more pen.
		if (penetration <= 0.f || pen <= 0)
			return false;

		// try to penetrate object.
		if (!TraceToExit(trace.endpos, dir, pen_end, &trace, &exit_trace)) {
			if (!(g_EngineTrace->GetPointContents(pen_end, MASK_SHOT_HULL) & MASK_SHOT_HULL))
				return false;
		}

		// get surface / material at exit point.
		exit_surface  = g_PhysSurface->GetSurfaceData(exit_trace.surface.surfaceProps);
		exit_material = exit_surface->game.material;

		// todo - dex; check for CHAR_TEX_FLESH and ff_damage_bullet_penetration / ff_damage_reduction_bullets convars?
		//             also need to check !isbasecombatweapon too.
		if (enter_material == CHAR_TEX_GRATE || enter_material == CHAR_TEX_GLASS) {
			total_pen_mod = 3.f;
			damage_mod = 0.05f;
		}

		else if (nodraw || grate) {
			total_pen_mod = 1.f;
			damage_mod = 0.16f;
		}

		else {
			total_pen_mod = (enter_surface->game.flPenetrationModifier + exit_surface->game.flPenetrationModifier) * 0.5f;
			damage_mod = 0.16f;
		}

		// thin metals, wood and plastic get a penetration bonus.
		if (enter_material == exit_material) {
			if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
				total_pen_mod = 3.f;

			else if (exit_material == CHAR_TEX_PLASTIC)
				total_pen_mod = 2.f;
		}

		// set some local vars.
		trace_len = (exit_trace.endpos - trace.endpos).LengthSqr();
		modifier = fmaxf(1.f / total_pen_mod, 0.f);

		// this calculates how much damage we've lost depending on thickness of the wall, our penetration, damage, and the modifiers set earlier
		damage_lost = fmaxf(
			((modifier * trace_len) / 24.f)
			+ ((damage * damage_mod)
				+ (fmaxf(3.75 / penetration, 0.f) * 3.f * modifier)), 0.f);

		// subtract from damage.
		damage -= std::max(0.f, damage_lost);
		if (damage < 1.f)
			return false;

		// set new start pos for successive trace.
		start = exit_trace.endpos;

		// decrement pen.
		--pen;
	}

	return false;
}

// Feel the snakeware
// Recoded autowall
// Credit's : @Snake