#include "ragebot.h"
#include "../../helpers/math.hpp"
#include "../../helpers/utils.hpp"
#include "../../options.hpp"
#include "autowall/ragebot-autowall.h"

int curGroup;
void UpdateConfig() 
{
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

bool BoundingBoxCheck(C_BasePlayer* m_entity, matrix3x4_t matrix[MAXSTUDIOBONES]) {
	const auto collideable = m_entity->GetCollideable();
	if (!collideable)        return false;

	const auto bbmin = collideable->OBBMins() + m_entity->m_vecOrigin();
	const auto bbmax = collideable->OBBMaxs() + m_entity->m_vecOrigin();

	Vector points[7];
	points[0] = m_entity->GetHitboxPosition(HITBOX_HEAD, matrix);
	points[1] = (bbmin + bbmax) * 0.5f;
	points[2] = Vector((bbmax.x + bbmin.x) * 0.5f, (bbmax.y + bbmin.y) * 0.5f, bbmin.z);
	points[3] = bbmax;
	points[4] = Vector(bbmax.x, bbmin.y, bbmax.z);
	points[5] = Vector(bbmin.x, bbmin.y, bbmax.z);
	points[6] = Vector(bbmin.x, bbmax.y, bbmax.z);

	for (const auto& point : points) {
		if (g_LocalPlayer->CanSeePoint(point)) {
			return true;
		}
		else if (g_Options.ragebot_autowall) {
			if (g_AutoWall->autowall(g_LocalPlayer->GetShootPos(), point, g_LocalPlayer, m_entity).damage > 0)
				return true;
		}
	}

	return false;
};

std::optional<RageBot::AimInfo> RageBot::scan_record(C_BasePlayer* local, Animation* animation) {
	const auto weapon = local->m_hActiveWeapon().Get();

	if (!weapon) return std::nullopt;

	const auto info = weapon->GetCSWeaponData();

	if (!info)  return std::nullopt;

	if (!weapon->IsKnife()) return scan_record_gun(local, animation);
}

bool can_hit_hitbox(const Vector start, const Vector end, matrix3x4_t* bones, studiohdr_t* hdr, int box) {
	const auto studio_box = hdr->GetHitboxSet(0)->GetHitbox(box);

	if (!studio_box)
		return false;

	Vector min, max;

	const auto is_capsule = studio_box->m_flRadius != -1.f;

	if (is_capsule)
	{
		Math::VectorTransform(studio_box->bbmin, bones[studio_box->bone], min);
		Math::VectorTransform(studio_box->bbmax, bones[studio_box->bone], max);
		const auto dist = Math::Segment2Segment(start, end, min, max);

		if (dist < studio_box->m_flRadius)
			return true;
	}
	if (!is_capsule)
	{
		Math::VectorTransform(Math::VectorRotate(studio_box->bbmin, studio_box->rotation), bones[studio_box->bone], min);
		Math::VectorTransform(Math::VectorRotate(studio_box->bbmax, studio_box->rotation), bones[studio_box->bone], max);

		Math::vector_i_transform(start, bones[studio_box->bone], min);
		Math::vector_i_rotate(end, bones[studio_box->bone], max);

		if (Math::intersect_line_with_bb(min, max, studio_box->bbmin, studio_box->bbmax))
			return true;
	}

	return false;
}
static std::vector<std::tuple<float, float, float>> precomputed_seeds = {};


void build_seed_table() {
	if (!precomputed_seeds.empty()) return;

	for (auto i = 0; i < 255; i++) {
		RandomSeed(i + 1);

		const auto pi_seed = Math::RandomFloat(0.f, (M_PI * 2));

		precomputed_seeds.emplace_back(Math::RandomFloat(0.f, 1.f), sin(pi_seed), cos(pi_seed));
	}
}

bool can_hit(Animation* animation, const Vector position, const float chance, int box) {
	// generate look-up-table to enhance performance.
	build_seed_table();

	const auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)      return false;

	const auto info = weapon->GetCSWeaponData();

	if (!info)        return false;

	const auto studio_model = g_MdlInfo->GetStudioModel(animation->player->GetModel());

	if (!studio_model) return false;

	// performance optimization.
	if ((g_LocalPlayer->GetShootPos() - position).Length() > info->flRange) return false;

	// setup calculation parameters.
	const auto round_acc = [](const float accuracy) { return roundf(accuracy * 1000.f) / 1000.f; };

	const auto sniper = weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP || weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SCAR20
		|| weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_G3SG1 || weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08;
	const auto crouched = g_LocalPlayer->m_fFlags() & IN_DUCK;

	weapon->UpdateAccuracyPenalty();

	// calculate inaccuracy.
	const auto weapon_inaccuracy = weapon->GetInaccuracy();
	const auto weapon_spread = weapon->GetSpread();

	if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		return weapon_inaccuracy < (crouched ? .0020f : .0055f);

	// no need for hitchance, if we can't increase it anyway.
	if (crouched)
	{
		if (round_acc(weapon_inaccuracy) == round_acc(sniper ? info->flInaccuracyCrouchAlt : info->flInaccuracyCrouch))
			return true;
	}
	else
	{
		if (round_acc(weapon_inaccuracy) == round_acc(sniper ? info->flInaccuracyStandAlt : info->flInaccuracyStand))
			return true;
	}

	// calculate start and angle.
	const auto start = g_LocalPlayer->GetShootPos();
	const auto aim_angle = Math::CalcAngle(start, position);
	Vector forward, right, up;
	Math::AngleVectors(aim_angle, forward, right, up);

	// keep track of all traces that hit the enemy.
	auto current = 0;

	// setup calculation parameters.
	Vector total_spread, end;
	QAngle spread_angle;
	float inaccuracy, spread_x, spread_y;
	std::tuple<float, float, float>* seed;

	const auto get_bullet_location = [&](int seed) {
		RandomSeed(seed + 1);
		float a = Math::RandomFloat(0.f, 1.f);
		float b = Math::RandomFloat(0.f, 2.f * M_PI);
		float c = Math::RandomFloat(0.f, 1.f);
		float d = Math::RandomFloat(0.f, 2.f * M_PI);
		if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		{
			a = 1.f - a * a;
			c = 1.f - c * c;
		}
		const float generated_spread = a * weapon_spread;
		const float generated_cone = c * weapon_inaccuracy;

		const Vector spread = Vector(
			std::cos(b) * generated_spread + std::cos(d) * generated_cone,
			std::sin(b) * generated_spread + std::sin(d) * generated_cone,
			0.f
		);

		return Vector(forward + right * -spread.x + up * -spread.y).Normalized();
	};

	// use look-up-table to find average hit probability.
	for (auto i = 0u; i < 255; i++)  // NOLINT(modernize-loop-convert)
	{
		// get seed.
		seed = &precomputed_seeds[i];

		// calculate spread.
		inaccuracy = std::get<0>(*seed) * weapon_inaccuracy;
		spread_x = std::get<2>(*seed) * inaccuracy;
		spread_y = std::get<1>(*seed) * inaccuracy;
		total_spread = (forward + right * spread_x + up * spread_y).Normalized();

		// calculate angle with spread applied.
		Math::VectorAngles(total_spread, spread_angle);

		// calculate end point of trace.
		Math::AngleVectors(spread_angle, end);
		end = start + end.Normalized() * info->flRange;

		// did we hit the hitbox?
		if (can_hit_hitbox(start, end, animation->bones, studio_model, box))
			current++;

		// abort if hitchance is already sufficent.
		if (static_cast<float>(current) / static_cast<float>(255) >= chance)
			return true;

		// abort if we can no longer reach hitchance.
		if (static_cast<float>(current + 255 - i) / static_cast<float>(255) < chance)
			return false;
	}

	return static_cast<float>(current) / static_cast<float>(255) >= chance;
}

std::vector<RageBot::AimInfo> RageBot::select_multipoint(Animation* animation, int box)
{
	std::vector<AimInfo> points;

	if (box == HITBOX_NECK) return points;

	auto scale = 0.f;
	if (box == HITBOX_HEAD) {
		scale = g_Options.ragebot_pointscale[curGroup] / 100.f;
	}
	else if (box == HITBOX_STOMACH || box == HITBOX_PELVIS || box == HITBOX_UPPER_CHEST || box == HITBOX_CHEST || box == HITBOX_LOWER_CHEST) {
		scale = g_Options.ragebot_bodyscale[curGroup] / 100.f;
	}
	else {
		scale = g_Options.ragebot_otherscale[curGroup] / 100.f;
	}

	const auto model = animation->player->GetModel();

	if (!model) return points;

	const auto studio_model = g_MdlInfo->GetStudioModel(model);

	if (!studio_model) return points;

	if (!g_LocalPlayer) return points;

	const auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon) return points;

	const auto anim = Animations::Get().GetAnimInfo(animation->player);

	if (!anim) return points;

	const auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(box);

	if (!hitbox) return points;

	const auto is_zeus = weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_ZEUS;

	if (is_zeus) return points;

	auto& mat = animation->bones[hitbox->bone];

	Vector min, max;
	Math::VectorTransform(hitbox->bbmax, mat, max);
	Math::VectorTransform(hitbox->bbmin, mat, min);

	const auto center = (min + max) * 0.5f;
	const auto cur_angles = Math::CalcAngle(center, g_LocalPlayer->GetShootPos());

	Vector forward;
	Math::AngleVectors(cur_angles, forward);

	auto rs = hitbox->m_flRadius * scale;

	if (rs <= 0.f || g_LocalPlayer->CanSeePoint(center))
		return points;

	const auto right = forward.Cross(Vector(0.f, 0.f, 1.f)) * rs;
	const auto left = Vector(-right.x, -right.y, right.z);
	const auto top = Vector(0.f, 0.f, 1.f) * rs;

	const auto delta = (max - min).Normalized();
	QAngle angle;
	Math::VectorAngles(delta, angle);
	angle -= cur_angles;
	Math::Normalize3(angle);

	const auto is_horizontal = angle.pitch < 45.f && angle.pitch > -45.f;
	const auto is_flipped = angle.yaw < 0.f;

	if (box == HITBOX_HEAD || (box != HITBOX_LEFT_FOOT && box != HITBOX_RIGHT_FOOT)) {
		points.emplace_back(max + top, 0.f, animation, false, center, hitbox->m_flRadius, rs, box);
		points.emplace_back(min - top, 0.f, animation, false, center, hitbox->m_flRadius, rs, box);
	}

	points.emplace_back(max - (is_horizontal ? Vector() - top : left), 0.f, animation, false, center, hitbox->m_flRadius, rs, box);
	points.emplace_back(max - (is_horizontal ? is_flipped ? left : right : right), 0.f, animation, false, center, hitbox->m_flRadius, rs, box);

	if (box != HITBOX_LEFT_FOOT && box != HITBOX_RIGHT_FOOT) {
		points.emplace_back(min - (is_horizontal ? top : left),
			0.f, animation, false, center, hitbox->m_flRadius, rs, box);
		points.emplace_back(min + (is_horizontal ? is_flipped ? left : right : left),
			0.f, animation, false, center, hitbox->m_flRadius, rs, box);
	}

	return points;
}


std::optional<RageBot::AimInfo> RageBot::scan_record_gun(C_BasePlayer* local, Animation* animation) {
	if (!animation || !animation->player)  return std::nullopt;

	const auto info = Animations::Get().GetAnimInfo(animation->player);

	if (!info) return std::nullopt;

	AimInfo best_match = { Vector(), 0.f, nullptr, false, Vector(), 0.f, 0.f, HITBOX_HEAD };

	int health = animation->player->m_iHealth();

	const auto scan_box = [&](int hitbox)
	{
		auto box = animation->player->GetHitboxPosition(hitbox, const_cast<matrix3x4_t*>(animation->bones));

		if (box == Vector(0, 0, 0))
			return;

		auto points = select_multipoint(animation, hitbox);
		points.emplace_back(box, 0.f, animation, false, box, 0.f, 0.f, hitbox);

		for (auto& point : points)
		{
			if (local->CanSeePoint(point.position))
			{

				const auto wall = g_TraceSystem->wall_penetration(local->GetShootPos(), point.position, animation);

				if (!wall.has_value())
					continue;

				point.damage = wall.value().damage;
				if ((point.damage > g_Options.ragebot_vis_mindamage[curGroup] || point.damage > health) && point.damage > best_match.damage)
					best_match = point;
			}
			else
			{
				const auto wall = g_TraceSystem->wall_penetration(local->GetShootPos(), point.position, animation);
				if (!wall.has_value())
					continue;

				point.damage = wall.value().damage;
				if ((point.damage > g_Options.ragebot_mindamage[curGroup] || point.damage > health) && point.damage > best_match.damage)
					best_match = point;
			}
		}
	};

	std::vector<int> hitboxes;

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


	for (auto hitbox : hitboxes)
	{
		scan_box(hitbox);
	}

	if (local->CanSeePoint(best_match.position))
	{
		if (best_match.damage > g_Options.ragebot_vis_mindamage[curGroup] || best_match.damage > animation->player->m_iHealth())
			return best_match;
	}
	else
	{
		if (best_match.damage > g_Options.ragebot_mindamage[curGroup] || best_match.damage > animation->player->m_iHealth())
			return best_match;
	}
	return std::nullopt;
}

bool RageBot::is_breaking_lagcomp(Animation* animation) {
	static constexpr auto teleport_dist = 64 * 64;

	const auto info = Animations::Get().GetAnimInfo(animation->player);

	if (!info || info->frames.size() < 2)  return false;

	if (info->frames[0]->dormant)          return false;

	auto prev_org = info->frames[0]->origin;
	auto skip_first = true;

	// walk context looking for any invalidating event
	for (auto& record : info->frames)
	{
		if (skip_first)
		{
			skip_first = false;
			continue;
		}

		if (record->dormant)
			break;

		auto delta = record->origin - prev_org;
		if (delta.Length2D() > teleport_dist)
		{
			// lost track, too much difference
			return true;
		}

		// did we find a context smaller than target time?
		if (record->sim_time <= animation->sim_time)
			break; // hurra, stop

		prev_org = record->origin;
	}

	return false;
}

void RageBot::CreateMove(C_BasePlayer* local, CUserCmd* cmd, bool& send_packet)
{

	last_pitch = std::nullopt;
	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected()) return;
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())                   return;
	if (!g_Options.ragebot_enabled)                                    return;
	auto weapon = local->m_hActiveWeapon().Get();
	if (!weapon)            return;

	auto wpn_info = weapon->GetCSWeaponData();
	if (!wpn_info)          return;

	if (!weapon->CanFire()) return;

	std::vector<AimInfo> hitpoints = {};

	for (int i = 1; i < g_GlobalVars->maxClients; i++) {
		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

		if (!IsViable(player))
			continue;

		const auto latest = Animations::Get().get_latest_animation(player);

		if (!latest.has_value()) continue;

		const auto rtt = 2.f * g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
		const auto breaking_lagcomp = latest.value()->lag && latest.value()->lag <= 16 && is_breaking_lagcomp(latest.value());
		const auto can_delay_shot = (latest.value()->lag > TIME_TO_TICKS(rtt) + g_GlobalVars->interval_per_tick);
		const auto delay_shot = (TIME_TO_TICKS(rtt) + TIME_TO_TICKS(g_GlobalVars->curtime - latest.value()->sim_time)
			+ g_GlobalVars->interval_per_tick >= latest.value()->lag);

		const auto oldest = Animations::Get().get_oldest_animation(player);

		if (breaking_lagcomp && delay_shot && can_delay_shot) return;

		if (breaking_lagcomp && !can_delay_shot)              return;

		if (FovToPlayer(player->GetHitboxPos(HITBOX_HEAD)) > g_Options.ragebot_fov) return;

		std::optional<AimInfo> aimbot_info;
		if (!aimbot_info.has_value())
		{
			if (oldest.has_value() && g_Options.ragebot_position_adj)
			{
				const auto alternative = scan_record(local, oldest.value());
				if (alternative.has_value())
					aimbot_info = alternative;
			}
			else
			{
				const auto alternative = scan_record(local, latest.value());
				if (alternative.has_value())
					aimbot_info = alternative;
			}
		}
		else {
			if (oldest.has_value() && g_Options.ragebot_position_adj) {
				const auto alternative = scan_record(local, oldest.value());
				if (alternative.has_value() && (aimbot_info.value().damage < alternative.value().damage))
					aimbot_info = alternative;
			}
			else
			{
				const auto alternative = scan_record(local, latest.value());
				if (alternative.has_value() && (aimbot_info.value().damage < alternative.value().damage))
					aimbot_info = alternative;
			}
		}

		if (aimbot_info.has_value())
			hitpoints.push_back(aimbot_info.value());
	}

	AimInfo best_match = { Vector(), 0.f, nullptr, false, Vector(), 0.f, 0.f, HITBOX_HEAD };

	// find best target spot of all valid spots.
	for (auto& hitpoint : hitpoints)
		if (hitpoint.damage > best_match.damage)
			best_match = hitpoint;

	// stop if no target found.
	if (best_match.damage <= 0.f)
		return;

	// calculate angle.
	auto angle = Math::CalcAngle(local->GetShootPos(), best_match.position);

	// store pitch for eye correction.
	last_pitch = angle.pitch;

	if (g_Options.ragebot_autoscope[curGroup] && weapon->IsSniper() && !local->m_bIsScoped())
	{
		cmd->buttons |= IN_ZOOM;
	}
	if (g_Options.ragebot_autostop[curGroup])
	{

			auto CurrentVelocityLength = g_LocalPlayer->m_vecVelocity().Length();

			float speed;
			int AccuracyMode = g_Options.ragebot_autostop_type[curGroup];
			int scalespeed;
			auto weapon = g_LocalPlayer->m_hActiveWeapon();

			switch (AccuracyMode)
			{
			case 0:  speed = 25 / (CurrentVelocityLength / 17.4); break;//default accuracy
			case 1:  speed = 25 / (CurrentVelocityLength / 19.8); break;//most of slowwalk accuracy
			case 2:  speed = 25 / (CurrentVelocityLength / 13.6); break;//lowlest accuracy accuracy
			case 3:  speed = 0.3;                                 break;//full stop 
			}

			bool r8 = weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER;


			float min_speed = (float)(Math::FASTSQRT((cmd->forwardmove) * (cmd->forwardmove) + (cmd->sidemove) * (cmd->sidemove) + (cmd->upmove) * (cmd->upmove)));
			if (min_speed <= 3.f) return;



			if (cmd->buttons & IN_DUCK)
				speed *= 2.94117647f;

			if (min_speed <= speed) return;

			float finalSpeed = (speed / min_speed);

			cmd->forwardmove *= finalSpeed;
			cmd->sidemove *= finalSpeed;
			cmd->upmove *= finalSpeed;


	}
	if (!can_hit(best_match.animation, best_match.center, g_Options.ragebot_hitchance[curGroup] / 100.f, best_match.hitbox))
	{
		if (g_Options.ragebot_autoscope[curGroup] && weapon->IsSniper() && !local->m_bIsScoped())
		{
			cmd->buttons |= IN_ZOOM;
		}
		if (g_Options.ragebot_autostop[curGroup])
		{
			if (g_Options.ragebot_autostop_type[curGroup] == 0)
			{
				//тут т=должен быть автостоп полный
			}
			else if (g_Options.ragebot_autostop_type[curGroup] == 2)
			{
				//тут т=должен быть автостоп слоувочный
			}
		}
		return;
	}

	// store shot info for resolver.

	// set correct information to user_cmd.

	cmd->viewangles = angle;

	if (!g_Options.ragebot_silent)
		g_EngineClient->SetViewAngles(&cmd->viewangles);


	if (g_Options.ragebot_autofire || cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2)
	{
		Snakeware::bSendPacket = true;
		cmd->tick_count = TIME_TO_TICKS(best_match.animation->sim_time) + TIME_TO_TICKS(LagCompensation::Get().GetLerpTime());
		if (!(cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2))
		{
			cmd->buttons |= best_match.alt_attack ? IN_ATTACK2 : IN_ATTACK;
		}
		if (g_Options.ragebot_remove_recoil)
		{
			angle -= local->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();
		}
	}

}

float RageBot::FovToPlayer(Vector AimPos)
{
	QAngle viewAngles;
	g_EngineClient->GetViewAngles(&viewAngles);
	CONST FLOAT MaxDegrees = 360.0f;
	Vector Delta(0, 0, 0);
	Vector Forward(0, 0, 0);
	Math::AngleVectors(viewAngles, Forward);
	VectorSubtractForFOV(AimPos, g_LocalPlayer->GetShootPos(), Delta);
	Normalize(Delta, Delta);
	FLOAT DotProduct = Forward.Dot(Delta);
	return (acos(DotProduct) * (MaxDegrees / PI_F));
}

bool RageBot::IsViable(C_BasePlayer* entity) {
	if (entity && !entity->IsDormant() && entity->IsAlive() && entity->EntIndex() != g_LocalPlayer->EntIndex()) {
		if (entity->m_iTeamNum() != g_LocalPlayer->m_iTeamNum()) {
			if (!entity->m_bGunGameImmunity()) {
				return true;
			}
		}
	}

	return false;
}