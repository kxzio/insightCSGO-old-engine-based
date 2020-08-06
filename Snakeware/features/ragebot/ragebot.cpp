#include "ragebot.h"
#include "../../helpers/math.hpp"
#include "../../helpers/utils.hpp"
#include "../../options.hpp"
#include "autowall/ragebot-autowall.h"

#define square( x ) ( x * x )
int curGroup;
static const int total_seeds = 255;
std::vector<ShotSnapshot> ShotSnapshots;
static std::vector<std::tuple<float, float, float>> precomputed_seeds = {};
void UpdateConfig () {
	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon) return;
	if (weapon->IsPistol()) curGroup = WEAPON_GROUPS::PISTOLS;
	else if (weapon->IsRifle() || weapon->IsMashineGun()) curGroup = WEAPON_GROUPS::RIFLES;
	else if (weapon->IsSMG()) curGroup = WEAPON_GROUPS::SMG;
	else if (weapon->IsShotgun()) curGroup = WEAPON_GROUPS::SHOTGUNS;
	else if (weapon->IsAuto()) curGroup = WEAPON_GROUPS::AUTO;
	else if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08) curGroup = WEAPON_GROUPS::SCOUT;
	else if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP) curGroup = WEAPON_GROUPS::AWP;
	else curGroup = WEAPON_GROUPS::UNKNOWN;
}


int RageBot::GetCurrentPriorityHitbox(C_BasePlayer* pEntity) {
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon)
		return -1;

	if (!g_LocalPlayer->IsAlive())
		return -1;

	bool can_baim_on_miss = g_Options.ragebot_max_miss > 0
		&& Snakeware::MissedShots[pEntity->EntIndex()] > g_Options.ragebot_max_miss;

	if (!pEntity->IsAlive())
		return -1;
	if (weapon->IsZeus())
		return (int)HITBOX_PELVIS;
	if (g_Options.ragebot_baim_in_air[curGroup] && !(pEntity->m_fFlags() & FL_ONGROUND))
		return (int)HITBOX_PELVIS;
	if (g_Options.ragebot_baim_in_move[curGroup] && pEntity->m_fFlags() & FL_ONGROUND && pEntity->m_vecVelocity().Length2D() >= 169)
		return (int)HITBOX_PELVIS;
	if (can_baim_on_miss) // sh1ttt
		return (int)HITBOX_PELVIS;
	if (pEntity->m_iHealth() <= g_Options.ragebot_baim_if_hp[curGroup])
		return (int)HITBOX_PELVIS;
	if (GetAsyncKeyState(g_Options.ragebot_baim_key))
		return (int)HITBOX_PELVIS;
	if (pEntity->BadMatrix())
		return (int)HITBOX_PELVIS;
	return 0;
}
std::vector<int> RageBot::GetHitboxesToScan(C_BasePlayer* pEntity) {
	
	std::vector< int > hitboxes;
	auto weapon = g_LocalPlayer->m_hActiveWeapon(); 
	if (weapon->IsZeus()) {
		hitboxes.push_back((int)HITBOX_CHEST);
		hitboxes.push_back((int)HITBOX_STOMACH);
		hitboxes.push_back((int)HITBOX_PELVIS);
		return hitboxes;
	}

	if (GetCurrentPriorityHitbox(pEntity) == (int)HITBOX_PELVIS) // baim
	{
		if (g_Options.ragebot_baimhitbox[0][curGroup])
		{
			hitboxes.push_back((int)HITBOX_CHEST);
		}

		if (g_Options.ragebot_baimhitbox[1][curGroup])
		{
			hitboxes.push_back((int)HITBOX_STOMACH);
		}
		if (g_Options.ragebot_baimhitbox[2][curGroup])
		{
			hitboxes.push_back((int)HITBOX_PELVIS);
		}

		return hitboxes;
	}

	if (g_Options.ragebot_hitbox[0][curGroup])
	{
		hitboxes.push_back((int)HITBOX_HEAD);
	}
	if (g_Options.ragebot_hitbox[1][curGroup])
	{
		hitboxes.push_back((int)HITBOX_NECK);
	}
	if (g_Options.ragebot_hitbox[2][curGroup]) {
		hitboxes.push_back((int)HITBOX_UPPER_CHEST);
		hitboxes.push_back((int)HITBOX_CHEST);
	}
	if (g_Options.ragebot_hitbox[3][curGroup]) {
		hitboxes.push_back((int)HITBOX_PELVIS);
		hitboxes.push_back((int)HITBOX_STOMACH);
	}
	if (g_Options.ragebot_hitbox[4][curGroup]) {
		hitboxes.push_back((int)HITBOX_RIGHT_UPPER_ARM);
		hitboxes.push_back((int)HITBOX_LEFT_UPPER_ARM);

		hitboxes.push_back((int)HITBOX_RIGHT_FOREARM);
		hitboxes.push_back((int)HITBOX_LEFT_FOREARM);

		hitboxes.push_back((int)HITBOX_RIGHT_HAND);
		hitboxes.push_back((int)HITBOX_LEFT_HAND);

	}
	if (g_Options.ragebot_hitbox[5][curGroup]) {
		hitboxes.push_back((int)HITBOX_RIGHT_THIGH);
		hitboxes.push_back((int)HITBOX_LEFT_THIGH);

		hitboxes.push_back((int)HITBOX_RIGHT_CALF);
		hitboxes.push_back((int)HITBOX_LEFT_CALF);

		hitboxes.push_back((int)HITBOX_RIGHT_FOOT);
		hitboxes.push_back((int)HITBOX_LEFT_FOOT);

	}



	return hitboxes;
}





void RageBot::BackupPlayer(Animation* anims) {
	auto i = anims->player->EntIndex();
	backup_anims[i].origin = anims->player->m_vecOrigin();
	backup_anims[i].abs_origin = anims->player->m_angAbsOrigin();
	backup_anims[i].obb_mins = anims->player->m_vecMins();
	backup_anims[i].obb_maxs = anims->player->m_vecMaxs();

}
void RageBot::SetAnims(Animation* anims) {

	anims->player->m_vecOrigin() = anims->origin;
	anims->player->SetAbsOrigin(anims->abs_origin);
	anims->player->m_vecMins() = anims->obb_mins;
	anims->player->m_vecMaxs() = anims->obb_maxs;
	
}
void RageBot::RestorePlayer(Animation* anims) {
	auto i = anims->player->EntIndex();
	anims->player->m_vecOrigin() = backup_anims[i].origin;
	anims->player->SetAbsOrigin(backup_anims[i].abs_origin);
	anims->player->m_vecMins() = backup_anims[i].obb_mins;
	anims->player->m_vecMaxs() = backup_anims[i].obb_maxs;

}


Vector RageBot::GetPoint(C_BasePlayer* pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[128])
{
	std::vector<Vector> vPoints;

	if (!pBaseEntity) return Vector(0, 0, 0);

	studiohdr_t* pStudioModel = g_MdlInfo->GetStudioModel(pBaseEntity->GetModel());
	mstudiohitboxset_t* set = pStudioModel->GetHitboxSet(0);

	if (!set) return Vector(0, 0, 0);

	mstudiobbox_t* untransformedBox = set->GetHitbox(iHitbox);
	if (!untransformedBox)
		return Vector(0, 0, 0);

	Vector vecMin = { 0, 0, 0 };
	Math::VectorTransformWrapper(untransformedBox->bbmin, BoneMatrix[untransformedBox->bone], vecMin);

	Vector vecMax = { 0, 0, 0 };
	Math::VectorTransformWrapper(untransformedBox->bbmax, BoneMatrix[untransformedBox->bone], vecMax);

	float mod = untransformedBox->m_flRadius != -1.f ? untransformedBox->m_flRadius : 0.f;
	Vector max;
	Vector min;

	Math::VectorTransform(untransformedBox->bbmax + mod, BoneMatrix[untransformedBox->bone], max);
	Math::VectorTransform(untransformedBox->bbmin - mod, BoneMatrix[untransformedBox->bone], min);

	return (min + max) * 0.5f;
}




void BulidSeeds()
{
	if (!precomputed_seeds.empty())
		return;

	for (auto i = 0; i < total_seeds; i++) {
		RandomSeed(i + 1);

		const auto pi_seed = Math::RandomFloat(0.f, M_PI * 2);

		precomputed_seeds.emplace_back(Math::RandomFloat(0.f, 1.f),
			sin(pi_seed), cos(pi_seed));
	}
}

bool HitTraces(Animation* _animation, const Vector position, const float chance, int box)
{
	BulidSeeds();

	const auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon) return false;

	const auto info = weapon->GetCSWeaponData();

	if (!info) return false;

	const auto studio_model = g_MdlInfo->GetStudioModel(_animation->player->GetModel());

	if (!studio_model) return false;

	// performance optimization.
	if ((g_LocalPlayer->GetEyePos() - position).Length2D() > info->flRange)
		return false;

	// setup calculation parameters.
	const auto id = weapon->m_Item().m_iItemDefinitionIndex();
	const auto round_acc = [](const float accuracy) { return roundf(accuracy * 1000.f) / 1000.f; };
	const auto sniper    = weapon->IsSniper();
	const auto crouched  = g_LocalPlayer->m_fFlags() & FL_DUCKING;

	// calculate inaccuracy.
	const auto weapon_inaccuracy = weapon->GetInaccuracy();

	if (id == WEAPON_REVOLVER)
		return weapon_inaccuracy < (crouched ? .0020f : .0055f);

	// calculate start and angle.
	auto start = g_LocalPlayer->GetEyePos();
	const auto aim_angle = Math::CalculateAngle(start, position);
	Vector forward, right, up;
	Math::AngleVectorS(aim_angle, &forward, &right, &up); //Wrong, need fix.

	// keep track of all traces that hit the enemy.
	auto current = 0;

	// setup calculation parameters.
	Vector total_spread, spread_angle, end;
	float inaccuracy, spread_x, spread_y;
	std::tuple<float, float, float>* seed;

	// use look-up-table to find average hit probability.
	for (auto i = 0u; i < total_seeds; i++)  // NOLINT(modernize-loop-convert)
	{
		// get seed.
		seed = &precomputed_seeds[i];

		// calculate spread.
		inaccuracy = std::get<0>(*seed) * weapon_inaccuracy;
		spread_x = std::get<2>(*seed) * inaccuracy;
		spread_y = std::get<1>(*seed) * inaccuracy;
		total_spread = (forward + right * spread_x + up * spread_y).NormalizeAng();

		// calculate angle with spread applied.
		Math::VectorAngles2(total_spread, spread_angle);

		// calculate end point of trace.
		Math::AngleVectors2(spread_angle, end);
		end = start + end.Normalize() * info->flRange;

		// did we hit the hitbox?
		if (g_Options.ragebot_hitchance_consider
			&& box != (int)HITBOX_LEFT_FOOT
			&& box != (int)HITBOX_RIGHT_FOOT) {
			if (CanHitHitbox(start, end, _animation, studio_model, box))
				current++;
		}
		else
		{
			trace_t tr;
			Ray_t ray;

			ray.Init(start, end);
			g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, _animation->player, &tr);

			if (auto ent = tr.hit_entity; ent)
			{
				if (ent == _animation->player)
					current++;
			}
		}
		// abort if hitchance is already sufficent.
		if (static_cast<float>(current) / static_cast<float>(total_seeds) >= chance)
			return true;

		// abort if we can no longer reach hitchance.
		if (static_cast<float>(current + total_seeds - i) / static_cast<float>(total_seeds) < chance)
			return false;
	}

	return static_cast<float>(current) / static_cast<float>(total_seeds) >= chance;
}

std::vector<Vector> RageBot::GetMultipoints(C_BasePlayer* pBaseEntity, int iHitbox, matrix3x4_t BoneMatrix[128]) {
	std::vector<Vector> vPoints;

	if (!pBaseEntity)
		return vPoints;

	studiohdr_t* pStudioModel = g_MdlInfo->GetStudioModel(pBaseEntity->GetModel());
	mstudiohitboxset_t* set = pStudioModel->GetHitboxSet(0);

	if (!set)
		return vPoints;

	mstudiobbox_t* untransformedBox = set->GetHitbox(iHitbox);
	if (!untransformedBox)
		return vPoints;

	Vector vecMin = { 0, 0, 0 };
	Math::VectorTransformWrapper(untransformedBox->bbmin, BoneMatrix[untransformedBox->bone], vecMin);

	Vector vecMax = { 0, 0, 0 };
	Math::VectorTransformWrapper(untransformedBox->bbmax, BoneMatrix[untransformedBox->bone], vecMax);

	float mod = untransformedBox->m_flRadius != -1.f ? untransformedBox->m_flRadius : 0.f;
	Vector max;
	Vector min;

	float ps = 0.75f;
	if (pBaseEntity->m_vecVelocity().Length() > 300.f && iHitbox > 0)
		ps = 0.f;
	else {
		if (iHitbox <= (int)HITBOX_NECK)
			ps = g_Options.ragebot_pointscale[curGroup] / 100;
		else if (iHitbox <= (int)HITBOX_RIGHT_THIGH)
			ps = g_Options.ragebot_bodyscale[curGroup] / 100;
	}

	Math::VectorTransform(untransformedBox->bbmax + mod, BoneMatrix[untransformedBox->bone], max);
	Math::VectorTransform(untransformedBox->bbmin - mod, BoneMatrix[untransformedBox->bone], min);

	auto center = (min + max) * 0.5f;
	if (ps <= 0.05f) {
		vPoints.push_back(center);
		return vPoints;
	}

	auto clamp_shit = [](float val, float min, float max) {
		if (val < min)
			return min;
		if (val > max)
			return max;
		return val;
	};
	Vector curAngles = Math::CalculateAngle(center, g_LocalPlayer->GetEyePos());
	Vector forward;
	Math::AngleVectors2(curAngles, forward);
	Vector right = forward.Cross(Vector(0, 0, 1));
	Vector left = Vector(-right.x, -right.y, right.z);
	if (iHitbox == 0) {
		for (auto i = 0; i < 4; ++i)
			vPoints.push_back(center);
		vPoints[1].x += untransformedBox->m_flRadius * clamp_shit(0.f, ps - 0.2f, 0.87f); // near left ear
		vPoints[2].x -= untransformedBox->m_flRadius * clamp_shit(0.f, ps - 0.2f, 0.87f); // near right ear
		vPoints[3].z += untransformedBox->m_flRadius * ps - 0.05f; // forehead
	}
	else if (iHitbox == (int)HITBOX_NECK)
		vPoints.push_back(center);
	else if (iHitbox == (int)HITBOX_RIGHT_THIGH ||
		iHitbox      == (int)HITBOX_LEFT_THIGH ||
		iHitbox      == (int)HITBOX_RIGHT_CALF ||
		iHitbox      == (int)HITBOX_LEFT_CALF ||
		iHitbox      == (int)HITBOX_RIGHT_FOOT ||
		iHitbox      == (int)HITBOX_LEFT_FOOT) {

		if (iHitbox == (int)HITBOX_RIGHT_THIGH ||
			iHitbox == (int)HITBOX_LEFT_THIGH) {
			vPoints.push_back(center);
		}
		else if (iHitbox == (int)HITBOX_RIGHT_CALF ||
			iHitbox      == (int)HITBOX_LEFT_CALF) {
			vPoints.push_back(center);
		}
		else if (iHitbox == (int)HITBOX_RIGHT_FOOT ||
			iHitbox == (int)HITBOX_LEFT_FOOT) {
			vPoints.push_back(center);
			vPoints[0].z += 5.f;
		}
	}
	else if (iHitbox == (int)HITBOX_RIGHT_HAND ||
		iHitbox      == (int)HITBOX_LEFT_HAND ||
		iHitbox      == (int)HITBOX_RIGHT_UPPER_ARM ||
		iHitbox      == (int)HITBOX_RIGHT_FOREARM ||
		iHitbox      == (int)HITBOX_LEFT_UPPER_ARM ||
		iHitbox      == (int)HITBOX_LEFT_FOREARM) {
		vPoints.push_back(center);
	}
	else {
		for (auto i = 0; i < 3; ++i)
			vPoints.push_back(center);
		vPoints[1] += right * (untransformedBox->m_flRadius * ps);
		vPoints[2] += left  * (untransformedBox->m_flRadius * ps);
	}


	return vPoints;
}

Vector RageBot::HeadScan(Animation* anims, int& hitbox, float& best_damage, float min_dmg) {
	Vector best_point = Vector(0, 0, 0);
	memcpy(BoneMatrix, anims->bones, sizeof(matrix3x4_t[128]));
	SetAnims(anims);
	int health = anims->player->m_iHealth();
	if (min_dmg > health)
		min_dmg = health + 1;
	std::vector<Vector> Points = GetMultipoints(anims->player, 0, BoneMatrix);
	for (auto HitBox : Points) {

		auto info = AutoWall::Get().Think(HitBox, anims->player);
		if (info.m_damage > min_dmg && info.m_damage > best_damage)
		{
			hitbox = 0;
			best_point = HitBox;
			best_damage = info.m_damage;
		}
	}
	RestorePlayer(anims);
	return best_point;
}

Vector RageBot::PrimaryScan(Animation* anims, int& hitbox, float& simtime, float& best_damage, float min_dmg) {
	memcpy(BoneMatrix, anims->bones, sizeof(matrix3x4_t[128]));
	simtime = anims->sim_time;
	SetAnims(anims);

	best_damage = -1;
	const auto damage = min_dmg;
	auto best_point = Vector(0, 0, 0);
	auto health = anims->player->m_iHealth();
	if (min_dmg > health)
		min_dmg = health + 1;
	auto priority_hitbox = GetCurrentPriorityHitbox(anims->player);

	static const std::vector<int> hitboxes = {
		(int)HITBOX_HEAD,
		(int)HITBOX_CHEST,
		(int)HITBOX_STOMACH,
		(int)PELVIS,
		(int)HITBOX_LEFT_CALF,
		(int)HITBOX_RIGHT_CALF,
	};

	for (auto HitboxID : hitboxes)
	{
		auto point = GetPoint(anims->player, HitboxID, BoneMatrix);
		auto info = AutoWall::Get().Think(point, anims->player);
		if ((info.m_damage > min_dmg && info.m_damage > best_damage) || info.m_damage > health)
		{
			hitbox = HitboxID;
			best_point = point;
			best_damage = info.m_damage;
		}
	}
	RestorePlayer(anims);
	return best_point;
}

Vector RageBot::FullScan (Animation* anims, int& hitbox, float& simtime, float& best_damage, float min_dmg) {
	memcpy(BoneMatrix, anims->bones, sizeof(matrix3x4_t[128]));
	simtime = anims->sim_time;
	best_damage = -1;
	Vector best_point = Vector(0, 0, 0);
	SetAnims(anims);
	int priority_hitbox = GetCurrentPriorityHitbox(anims->player);
	int health = anims->player->m_iHealth();
	if (min_dmg > health)
		min_dmg = health + 1;
	auto hitboxes = GetHitboxesToScan(anims->player);

	static const std::vector<int> upper_hitboxes = {
		(int)HITBOX_HEAD,
		(int)HITBOX_NECK,
		(int)HITBOX_UPPER_CHEST,
		(int)HITBOX_CHEST,
	};
	static const std::vector<int> baim_hitboxes = {
		(int)HITBOX_CHEST,
		(int)HITBOX_STOMACH,
		(int)PELVIS,
	};
	bool baim_if_lethal = g_Options.ragebot_baim_if_lethal[curGroup];
	if (baim_if_lethal || g_Options.ragebot_adaptive_baim[curGroup]) {
		for (auto HitboxID : baim_hitboxes) {
			std::vector<Vector> Points = GetMultipoints(anims->player, HitboxID, BoneMatrix);
			for (int k = 0; k < Points.size(); k++)
			{
				auto info = AutoWall::Get().Think(Points[k], anims->player);
				if ((info.m_damage > min_dmg && info.m_damage > best_damage))
				{
					hitbox = HitboxID;
					best_point = Points[k];
					best_damage = info.m_damage;
				}
			}
		}
		if (baim_if_lethal && best_damage > health + 2) {
			target_lethal = true;
			RestorePlayer(anims);
			return best_point;
		}
		if (best_damage > 0 && g_Options.ragebot_adaptive_baim[curGroup]) {
		
				if (best_damage < health)
					target_lethal = false;
				RestorePlayer(anims);
				return best_point;
			

		}
	}

	for (auto HitboxID : hitboxes)
	{
		std::vector<Vector> Points = GetMultipoints(anims->player, HitboxID, BoneMatrix);
		for (int k = 0; k < Points.size(); k++)
		{
			auto info = AutoWall::Get().Think(Points[k], anims->player);
			if ((info.m_damage > min_dmg && info.m_damage > best_damage))
			{
				hitbox = HitboxID;
				best_point = Points[k];
				best_damage = info.m_damage;
			}
		}
	}

	if (best_damage > anims->player->m_iHealth() + 2)
		target_lethal = true;
	RestorePlayer(anims);
	return best_point;
}



Vector RageBot::GetAimVector(C_BasePlayer *pTarget, float &simtime, Vector &, Animation *&best_anims, int &hitbox) {
	if (GetHitboxesToScan(pTarget).size() == 0) return Vector(0, 0, 0);

	float m_damage = 0.f;
	if (GetAsyncKeyState(g_Options.ragebot_mindamage_override_key)) {
		m_damage = g_LocalPlayer->m_hActiveWeapon()->IsZeus() ? 100.f : g_Options.ragebot_mindamage_override[curGroup];
	}
	else
		m_damage = g_LocalPlayer->m_hActiveWeapon()->IsZeus() ? 100.f : g_Options.ragebot_mindamage[curGroup];

	auto latest_animation = Animations::Get().get_latest_animation(pTarget);
	auto record           = latest_animation;
	if (!record.has_value() || !record.value()->player) return Vector(0, 0, 0);
	BackupPlayer(record.value());
	if (!g_Options.ragebot_position_adj) {
		if (record.has_value()) {
			float damage = -1.f;
			best_anims = record.value();

			return FullScan(record.value(), hitbox, simtime, damage, m_damage);
		}
	}

	if (g_Options.ragebot_backshoot && !GetAsyncKeyState(g_Options.ragebot_baim_key)) {
		record = Animations::Get().get_latest_firing_animation(pTarget);
		if (record.has_value() && record.value()->player) {
			float damage = -1.f;
			best_anims = record.value();
			simtime = record.value()->sim_time;
			Vector backshoot = HeadScan(record.value(), hitbox, damage, m_damage);
			if (backshoot != Vector(0, 0, 0))
				return backshoot;
		}

	}

	auto oldest_animation = Animations::Get().get_oldest_animation(pTarget);
	Vector latest_origin = Vector(0, 0, 0);
	float best_damage_0 = -1.f, best_damage_1 = -1.f;

	record = latest_animation;
	if (record.has_value()) {
		latest_origin = record.value()->origin;
		float damage = -1.f;
		Vector full = PrimaryScan(record.value(), hitbox, simtime, damage, m_damage);
		if (full != Vector(0, 0, 0))
			best_damage_0 = damage;
	}

	record = oldest_animation;

	if (record.has_value() && record.value()->resolver == latest_animation.value()->resolver) {
		// WTF ? wrong, fix this
		float damage = -1.f;
		Vector full = PrimaryScan(record.value(), hitbox, simtime, damage, m_damage);
		if (full != Vector(0, 0, 0))
			best_damage_1 = damage;
	}

	if (best_damage_0 >= best_damage_1)
		record = latest_animation;
	else
		record = oldest_animation;

	if (record.has_value())
	{
		float damage = -1.f;
		best_anims = record.value();
		return FullScan(record.value(), hitbox, simtime, damage, m_damage);
	}
	return Vector(0, 0, 0);

}

bool RageBot::Hitchance(Vector Aimpoint, bool backtrack, Animation* best, int& hitbox) {
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon)  return false;
	bool r8 = weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER;
	if (g_Options.ragebot_alternative_hitchance || r8)
		return weapon->Hitchance() > g_Options.ragebot_hitchance[curGroup] * (1.7 * (1.f - r8));
	else
		return HitTraces(best, Aimpoint, weapon->IsZeus() ? 0.8f : g_Options.ragebot_hitchance[curGroup] / 100.f, hitbox);
}
#include "../tickbase-shift/tickbase-exploits.h"
bool RageBot::IsAbleToShoot()  {
	//Can shoot check's
	auto time = EnginePrediction::Get().get_curtime();
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || !weapon) return false;

	const auto is_zeus = weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_ZEUS;
	const auto is_knife = !is_zeus && weapon->GetCSWeaponData()->iWeaponType == WEAPONTYPE_KNIFE;
	const auto weapontype = weapon->GetCSWeaponData()->iWeaponType;
	if (weapontype == WEAPONTYPE_C4 || weapon->IsGrenade()) return false;
	if (weapon->m_iClip1() < 1 && !is_knife)    return false;

	if (weapon->IsReloading())                  return false;

	if (weapon->m_flNextPrimaryAttack() > time) return false;

	if (g_LocalPlayer->m_flNextAttack() > time) return false;


	return true;

}

void RageBot::DropTarget () {

	target_index = -1;
	best_distance = INT_MAX;
	fired_in_that_tick = false;
	current_aim_position = Vector();
	shot = false;
	AutoWall::Get().reset();
}

std::string HitboxToString(int id) {
	switch (id) {
	case 0: return "head";  break;
	case 1: return "neck"; break;
	case 2: return "pelvis"; break;
	case 3: return "stomach"; break;
	case 4: return "lower chest"; break;
	case 5: return "chest"; break;
	case 6: return "upper chest"; break;
	case 7: return "right thigh"; break;
	case 8: return "left thigh"; break;
	case 9: return "right leg"; break;
	case 10: return "left leg"; break;
	case 11: return "right foot"; break;
	case 12: return "left foot"; break;
	case 13: return "right hand"; break;
	case 14: return "left hand"; break;
	case 15: return "right arm"; break;
	case 16: return "left arm"; break;
	case 17: return "right lower arm"; break;
	case 18: return "right upper arm"; break;
		break;
	}
}
void RageBot::QuickStop() {

	auto Velocity = EnginePrediction::Get().get_unpred_vel();
	const auto speed = Velocity.Length();
	if (speed > 15.f) {
		QAngle dir;
		Math::VectorAngles(Velocity, dir);
		dir.yaw = CurrentCmd->viewangles.yaw - dir.yaw;

		Vector new_move;
		Math::FixVectors(dir, &new_move);
		const auto max = std::max(std::fabs(CurrentCmd->forwardmove), std::fabs(CurrentCmd->sidemove));
		const auto mult = 450.f / max;
		new_move *= -mult;

		CurrentCmd->forwardmove = new_move.x;
		CurrentCmd->sidemove = new_move.y;
	}
	else {
		CurrentCmd->forwardmove = 0.f;
		CurrentCmd->sidemove = 0.f;
	}
}

void RageBot::CreateMove(CUserCmd* cmd) {

	if (!g_Options.ragebot_enabled || !g_EngineClient->IsInGame() || !g_EngineClient->IsConnected()) return;

	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	CurrentCmd = cmd;
	if (!weapon) return;
	UpdateConfig();
	int curhitbox;
	Animation* best_anims = nullptr;
	int hitbox = -1;

	float simtime = 0;
	Vector minus_origin = Vector(0, 0, 0);
	Animation* anims = nullptr;
	int box;

	RageBot::Get().shot = false;

	

	bool in_air = !(g_LocalPlayer->m_fFlags() & FL_ONGROUND);
	

	for (auto i = 1; i <= g_GlobalVars->maxClients; i++) {
		auto pEntity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!pEntity) continue;

		if (!pEntity->IsPlayer())      continue;
		if  (pEntity == nullptr)       continue;
		if  (pEntity == g_LocalPlayer) continue;
		if  (!pEntity->IsAlive()) {
			Snakeware::MissedShots[pEntity->EntIndex()] = 0;
			continue;
		}
		if (pEntity->m_iHealth() <= 0)                            continue;
		if (pEntity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) continue;
		if (pEntity->IsDormant())                                 continue;
		if (pEntity->m_bGunGameImmunity())                        continue;
		if (g_Options.ragebot_delayshot[curGroup]) {
			if (pEntity->m_flSimulationTime() == pEntity->m_flOldSimulationTime()) continue;
		}
		target_lethal = false;
		Vector aim_position = GetAimVector(pEntity, simtime, minus_origin, anims, box);

		if (!anims) continue;
		int health = pEntity->m_iHealth();
		if (best_distance > health
			&& anims->player == pEntity && aim_position != Vector(0, 0, 0))
		{
			best_distance = health;
			target_index = i;
			current_aim_position = aim_position;
			current_aim_simulationtime = simtime;
			current_aim_player_origin = minus_origin;
			best_anims = anims;
			hitbox = box;
			target_anims = best_anims;
		}
	}

	static int delay = 0;
	did_dt = false;

		

	if (hitbox != -1 && target_index != -1 && best_anims && current_aim_position != Vector(0, 0, 0)) {

	     // С этого момента чек не проходит получчч
		if (g_Options.ragebot_autoscope[curGroup] && weapon->IsSniper() && !g_LocalPlayer->m_bIsScoped()) { 
			cmd->buttons |= IN_ZOOM;
			return;
		}
	

		bool htchance = Hitchance(current_aim_position, false, best_anims, hitbox);

	
		static int dt_shot_tick = 20;
		auto wpn_info = weapon->GetCSWeaponData();
		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND && !GetAsyncKeyState(g_Options.misc_slowwalk_key)) {
			if (!weapon->IsZeus() && g_Options.ragebot_autostop[curGroup]) {
				// Quick stop call.
				QuickStop();

			}
		}

		if (g_Options.ragebot_autofire[curGroup]) {

			if (htchance &&  IsAbleToShoot()) {
				
				Snakeware::bSendPacket = true;
				cmd->buttons |= IN_ATTACK;
			}
		}
		if (htchance &&  IsAbleToShoot()) {

			if (cmd->buttons & IN_ATTACK) {

				cmd->viewangles = Math::CalcAngle(g_LocalPlayer->GetEyePos(), current_aim_position) - g_LocalPlayer->m_aimPunchAngle() * 2.f;
				cmd->tick_count = TIME_TO_TICKS(best_anims->sim_time + LagCompensation::Get().GetLerpTime()) - 1;
				last_shot_angle = cmd->viewangles;

				ShotSnapshot snapshot;
				//tick_record record;
				snapshot.entity = best_anims->player;
				snapshot.hitbox_where_shot = HitboxToString(hitbox);
			//	snapshot.resolver = ResolverMode[best_anims->player->EntIndex()];
				snapshot.time = g_GlobalVars->interval_per_tick * g_LocalPlayer->m_nTickBase();
				snapshot.first_processed_time = 0.f;
				snapshot.bullet_impact = false;
				snapshot.weapon_fire = false;
				snapshot.damage = -1;
				snapshot.start = g_LocalPlayer->GetEyePos();
				snapshot.hitgroup_hit = -1;
				snapshot.backtrack = TIME_TO_TICKS(fabsf(best_anims->player->m_flSimulationTime() - current_aim_simulationtime));
				snapshot.eyeangles = Math::NormalizeAng(best_anims->player->m_angEyeAngles());
				snapshot.hitbox = hitbox;
				snapshot.record = best_anims;
				ShotSnapshots.push_back(snapshot);
				shot = true;
				last_shot_tick = clock();
				//csgo->firedshots[best_anims->player->EntIndex()]++;
				
				last_tick_shooted = true;
			}
		}
	}
	if (IsAbleToShoot() && cmd->buttons & IN_ATTACK)
		shot = true;
}

std::string ShotSnapshot::get_info()
{
	return std::string();
}

bool CanHitHitbox(const Vector start, const Vector end, Animation * _animation, studiohdr_t * hdr, int box)
{
	studiohdr_t* pStudioModel = g_MdlInfo->GetStudioModel(_animation->player->GetModel());
	mstudiohitboxset_t* set = pStudioModel->GetHitboxSet(0);

	if (!set)
		return false;

	mstudiobbox_t* studio_box = set->GetHitbox(box);
	if (!studio_box)
		return false;

	Vector min, max;

	const auto is_capsule = studio_box->m_flRadius != -1.f;

	if (is_capsule)
	{
		Math::VectorTransform(studio_box->bbmin, _animation->bones[studio_box->bone], min);
		Math::VectorTransform(studio_box->bbmax, _animation->bones[studio_box->bone], max);
		const auto dist = Math::Segment2Segment(start, end, min, max);

		if (dist < studio_box->m_flRadius)
			return true;
	}

	if (!is_capsule)
	{
		Math::VectorTransform(Math::VectorRotate(studio_box->bbmin, studio_box->bbmin), _animation->bones[studio_box->bone], min);
		Math::VectorTransform(Math::VectorRotate(studio_box->bbmax, studio_box->rotation), _animation->bones[studio_box->bone], max);

		Math::vector_i_transform(start, _animation->bones[studio_box->bone], min);
		Math::vector_i_rotate(end, _animation->bones[studio_box->bone], max);

		if (Math::intersect_line_with_bb(min, max, studio_box->bbmin, studio_box->bbmax))
			return true;
	}

	return false;
}
