#include "ragebot.h"
#include "../../helpers/math.hpp"
#include "../../options.hpp"
#include "autowall/ragebot-autowall.h"
#include "resolver/resolver.h"


// Recoded rageb0t 
// From supremacy/pandora & onetap.idb

int curGroup;

void UpdateConfig() {

	// Snakeware nanotech
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



void AimPlayer::OnNetUpdate(C_BasePlayer* player) {
	bool reset = (!g_Options.ragebot_enabled || player->m_lifeState() == LIFE_DEAD || !player->IsEnemy());

	if (reset) {
		m_missed_shots = 0;
		m_last_resolve = 0;

		return;
	}

	// update player ptr.
	m_player = player;
}

void AimPlayer::OnRoundStart(C_BasePlayer* player) {
	m_player = player;
	m_shots = 0;
	m_missed_shots = 0;
	m_last_resolve = 0;
	m_delta = 0;

	m_hitboxes.clear();
	// IMPORTANT: DO NOT CLEAR LAST HIT SHIT.
}

void AimPlayer::SetupHitboxes(Animation* record, bool history) {
	if (!record) return;

	// reset hitboxes.
	m_hitboxes.clear();

	if (g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() == WEAPON_ZEUS) {
		// hitboxes for the zeus.
		m_hitboxes.push_back({ HITBOX_STOMACH, HitscanMode::PREFER });

		// why would we shoot for feet with the zeus??
		return;
	}

	// prefer, always.
	if (g_Options.ragebot_baim_if_lethal[curGroup])
		m_hitboxes.push_back({ HITBOX_STOMACH, HitscanMode::LETHAL });

	// prefer, in air.
	if (g_Options.ragebot_baim_in_air[curGroup] && !(record->flags & FL_ONGROUND))
		m_hitboxes.push_back({ HITBOX_STOMACH, HitscanMode::PREFER });

	bool only{ false };

	// only, always.
	if (g_Options.ragebot_adaptive_baim[curGroup] || Aimbot::Get().m_force_body) {
		only = true;
		m_hitboxes.push_back({ HITBOX_STOMACH, HitscanMode::PREFER });
	}

	// only, health.
	if (g_Options.ragebot_baim_if_hp[curGroup] && record->player->m_iHealth() <= 30) {
		only = true;
		m_hitboxes.push_back({ HITBOX_STOMACH, HitscanMode::PREFER });
	}

	// only, in air.
	if (g_Options.ragebot_baim_in_air[curGroup] && !(record->flags & FL_ONGROUND)) {
		only = true;
		m_hitboxes.push_back({ HITBOX_STOMACH, HitscanMode::PREFER });
	}

	// only baim conditions have been met.
	// do not insert more hitboxes.
	if (only)
		return;

	bool ignore_limbs = record->velocity.Length2D() > 71.f && g_Options.ragebot_ignore_limbs;

	if (g_Options.ragebot_hitbox[0][curGroup]) {

		if (g_Options.ragebot_safepoint) {
			m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::PREFER_SAFEPOINT, true });
		}

		m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::NORMAL, false });
	}

	if (g_Options.ragebot_hitbox[1][curGroup]) {
		m_hitboxes.push_back({ HITBOX_NECK, HitscanMode::NORMAL, false });
	}

	if (g_Options.ragebot_hitbox[2][curGroup]) {
		if (g_Options.ragebot_safepoint) {
			m_hitboxes.push_back({ HITBOX_LOWER_CHEST, HitscanMode::PREFER_SAFEPOINT, true });
		}

		m_hitboxes.push_back({ HITBOX_LOWER_CHEST, HitscanMode::NORMAL, true });
		m_hitboxes.push_back({ HITBOX_CHEST, HitscanMode::NORMAL, false });
		m_hitboxes.push_back({ HITBOX_UPPER_CHEST, HitscanMode::NORMAL, false });
	}
	if (g_Options.ragebot_hitbox[3][curGroup]) {

		if (g_Options.ragebot_safepoint) {
			m_hitboxes.push_back({ HITBOX_PELVIS, HitscanMode::PREFER_SAFEPOINT, true });
			m_hitboxes.push_back({ HITBOX_STOMACH, HitscanMode::PREFER_SAFEPOINT, true });
		}

		m_hitboxes.push_back({ HITBOX_PELVIS, HitscanMode::NORMAL, false });
		m_hitboxes.push_back({ HITBOX_STOMACH, HitscanMode::NORMAL, false });
	}

	if (g_Options.ragebot_hitbox[4][curGroup] && !ignore_limbs) {
		m_hitboxes.push_back({ HITBOX_LEFT_UPPER_ARM, HitscanMode::NORMAL, false });
		m_hitboxes.push_back({ HITBOX_RIGHT_UPPER_ARM, HitscanMode::NORMAL, false });
	}

	if (g_Options.ragebot_hitbox[5][curGroup]) {
		m_hitboxes.push_back({ HITBOX_LEFT_THIGH, HitscanMode::NORMAL, false });
		m_hitboxes.push_back({ HITBOX_RIGHT_THIGH, HitscanMode::NORMAL, false });
		m_hitboxes.push_back({ HITBOX_LEFT_CALF, HitscanMode::NORMAL, false });
		m_hitboxes.push_back({ HITBOX_RIGHT_CALF, HitscanMode::NORMAL, false });
	}

	if (g_Options.ragebot_hitbox[6][curGroup] && !ignore_limbs) {
		m_hitboxes.push_back({ HITBOX_LEFT_FOOT, HitscanMode::NORMAL, false });
		m_hitboxes.push_back({ HITBOX_RIGHT_FOOT, HitscanMode::NORMAL, false });
	}

}

void Aimbot::init() {
	// clear old targets.
	m_targets.clear();

	m_target = nullptr;
	m_aim = Vector{ };
	m_angle = QAngle{ };
	m_damage = 0.f;
	m_record = nullptr;
	m_stop = false;
	m_hitbox = -1;

	m_best_dist = std::numeric_limits< float >::max();
	m_best_fov = 180.f + 1.f;
	m_best_damage = 0.f;
	m_best_hp = 100 + 1;
	m_best_lag = std::numeric_limits< float >::max();
	m_best_height = std::numeric_limits< float >::max();
	m_current_matrix = nullptr;

	m_current_sphere.clear();
}

void Aimbot::StripAttack() {
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon) return;

	if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		return;
	else
		mcmd->buttons &= ~IN_ATTACK;
}

void Aimbot::Think(CUserCmd * cmd) {
	// do all startup routines.
	mcmd = cmd;
	init();

	auto m_weapon = g_LocalPlayer->m_hActiveWeapon();
	// sanity.
	if (!m_weapon)
		return;

	// no grenades or bomb.
	if (m_weapon->IsGrenade() || m_weapon->IsKnife() || !m_weapon->HasBullets())
		return;

	// we have no aimbot enabled.
	if (!g_Options.ragebot_enabled)
		return;

	if (!g_LocalPlayer->m_hActiveWeapon()->CanFire())
		StripAttack();

	// animation silent aim, prevent the ticks with the shot in it to become the tick that gets processed.
	// we can do this by always choking the tick before we are able to shoot.

	//REVOLVER DELETED
	//REVOLVER DELETED
	//REVOLVER DELETED  
	//REVOLVER DELETED

	//reasonj - lazy and false

	// we have a normal weapon or a non cocking revolver
	// choke if its the processing tick.


	// no point in aimbotting if we cannot fire this tick.
	if (!g_LocalPlayer->m_hActiveWeapon()->CanFire())
		return;

	// setup bones for all valid targets.
	for (int i{ 1 }; i <= g_GlobalVars->maxClients; ++i) {
		auto player = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!player || player == g_LocalPlayer)
			continue;

		if (!IsValidTarget(player))
			continue;

		AimPlayer* data = &m_players[i - 1];
		if (!data)
			continue;

		m_targets.push_back(data);
	}



	// scan available targets... if we even have any.
	find(cmd);

	// finally set data when shooting.
	apply();
}

void Autostope(CUserCmd cmd)
{
	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!g_Options.ragebot_autostop & g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	if (weapon->IsGrenade() || weapon->IsKnife()) //we dont wanna stop if we holdin a knife, grenade or zeus
		return;

	Vector Velocity = g_LocalPlayer->m_vecVelocity();
	static float Speed = 300.f;

	Vector Direction;
	Vector RealView;
	Math::VectorAngles(Velocity, QAngle(Direction.x, Direction.y, Direction.z));
	g_EngineClient->GetViewAngles(&QAngle(RealView.x, RealView.y, RealView.z));
	Direction.y = RealView.y - Direction.y;

	Vector Forward;
	Math::AngleVectors(QAngle(Direction.x, Direction.y, Direction.z), Forward);
	Vector NegativeDirection = Forward * -Speed;

	cmd.forwardmove = NegativeDirection.x;
	cmd.sidemove = NegativeDirection.y;
}

void Aimbot::find(CUserCmd* cmd) {
	struct BestTarget_t { C_BasePlayer* player; AimPlayer* target; Vector pos; float damage; float min_damage; Animation* record; int hitbox; };

	Vector       tmp_pos;
	float        tmp_damage;
	float		 tmp_min_damage;
	BestTarget_t best;
	best.player = nullptr;
	best.target = nullptr;
	best.damage = -1.f;
	best.pos = Vector{ };
	best.record = nullptr;
	best.hitbox = -1;

	if (m_targets.empty())
		return;

	// iterate all targets.
	for (const auto& t : m_targets) {
		if (!t->m_player)
			continue;

		const auto ideal = Animations::Get().get_latest_animation(t->m_player);
		if (!ideal.has_value() || ideal.value()->dormant || (ideal.value()->player && ideal.value()->player->m_bGunGameImmunity()))
			continue;

		t->SetupHitboxes(ideal.value(), false);
		if (t->m_hitboxes.empty())
			continue;

		// try to select best record as target.
		if (t->GetBestAimPosition(tmp_pos, tmp_damage, best.hitbox, ideal.value(), tmp_min_damage)) {
			if (SelectTarget(ideal.value(), tmp_pos, tmp_damage)) {
				// if we made it so far, set shit. 
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.record = ideal.value();
				best.min_damage = tmp_min_damage;
				best.target = t;
			}
		}

		const auto last = Animations::Get().get_oldest_animation(t->m_player);
		if (!last.has_value() || last.value() == ideal.value() || last.value()->dormant || (last.value()->player && last.value()->player->m_bGunGameImmunity()))
			continue;

		t->SetupHitboxes(last.value(), true);
		if (t->m_hitboxes.empty())
			continue;

		// rip something went wrong..
		if (t->GetBestAimPosition(tmp_pos, tmp_damage, best.hitbox, last.value(), tmp_min_damage)) {
			if (SelectTarget(last.value(), tmp_pos, tmp_damage)) {
				// if we made it so far, set shit.
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.record = last.value();
				best.min_damage = tmp_min_damage;
				best.target = t;
			}
		}

		// we found a target we can shoot at and deal damage? fuck yeah. (THIS IS TEMPORARY TILL WE REPLACE THE TARGET SELECTION)
		if (best.damage > 0.f && best.player && best.record)
			break;
	}

	// verify our target and set needed data.
	if (best.player && best.record && best.target) {
		// calculate aim angle.
		Math::VectorAngles(best.pos - g_LocalPlayer->GetShootPos(), m_angle);

		// set member vars.
		m_target = best.player;
		m_aim = best.pos;
		m_damage = best.damage;
		m_record = best.record;
		m_hitbox = best.hitbox;

		m_current_matrix = best.record->m_pMatrix_Resolved;

		if (!m_target || m_target->IsDormant() || m_record->dormant || !m_current_matrix || !m_damage || !(m_damage >= best.min_damage || (m_damage <= best.min_damage && m_damage >= m_target->m_iHealth())))
			return;

		//g_inputpred.Predict();

		if (best.target != m_old_target) {
			m_shoot_next_tick = false;
		}

		bool on = g_Options.ragebot_hitchance[curGroup] == 0;
		bool hit = ((cmd->buttons && IN_JUMP) && g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->iWeaponType == WEAPONTYPE_SNIPER_RIFLE && g_LocalPlayer->IsWeapon() && g_LocalPlayer->m_hActiveWeapon()->GetInaccuracy() < 0.009f) || (on && CheckHitchance(m_target, m_angle, m_record, best.hitbox));

		// set autostop shit.
		m_stop = !(cmd->buttons && IN_JUMP) && !hit;

		//autostop
		Autostope(*cmd);

		// if we can scope.
		bool can_scope = g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->iWeaponType == WEAPONTYPE_SNIPER_RIFLE && g_LocalPlayer->m_hActiveWeapon()->m_zoomLevel() == 0;

		if (can_scope) {
			// always.
			if (g_Options.ragebot_autoscope) {
				cmd->buttons |= IN_ATTACK2;
				return;
			}

		}

		if (hit || !on) {

			bool yadolbeb = true;
			// these conditions are so cancer
			if (yadolbeb) {
				if (g_Options.ragebot_autofire) {
					// right click attack.
					if (g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->bIsRevolver)
						 cmd->buttons |= IN_ATTACK2;

					// left click attack.
					else
						cmd->buttons |= IN_ATTACK;

					m_old_target = best.target;
				}
			}
		}
	}
}

bool Aimbot::CheckHitchance(C_BasePlayer* player, const QAngle& angle, Animation* record, int hitbox) {
	//note - nico; you might wonder why I changed HITCHANCE_MAX:
	//I made it require more seeds (while not double tapping) to hit because it ensures better accuracy
	//while double tapping/using double tap it requires less seeds now, so we might shoot the 2nd shot more often <.<
	auto pweapon = g_LocalPlayer->m_hActiveWeapon();
	if (!pweapon) return false;
	auto weaponinfo = pweapon->GetCSWeaponData();
	if (!weaponinfo) return false;

	float HITCHANCE_MAX = 82.f;
	constexpr int   SEED_MAX = 255;

	Vector     start{ g_LocalPlayer->GetShootPos() }, end, fwd, right, up, dir, wep_spread;
	float      inaccuracy, spread;
	float hitchance = g_Options.ragebot_hitchance[curGroup];

	if (m_shoot_next_tick)
		HITCHANCE_MAX += 25;

	size_t  total_hits{ }, needed_hits{ (size_t)std::ceil((hitchance * SEED_MAX) / HITCHANCE_MAX) };

	// get needed directional vectors.
	Math::AngleQ(angle, &fwd, &right, &up);

	// store off inaccuracy / spread ( these functions are quite intensive and we only need them once ).
	inaccuracy = pweapon->GetInaccuracy();
	spread = pweapon->GetSpread();

	// We will update accuracy in engine prediction like all p2c hack'ss

	// iterate all possible seeds.
	for (int i{ }; i <= SEED_MAX; ++i) {
		// get spread.
		wep_spread = pweapon->CalculateSpread(i, inaccuracy, spread);

		// get spread direction.
		dir = (fwd + (right * wep_spread.x) + (up * wep_spread.y)).Normalized();

		// get end of trace.
		end = start + (dir * weaponinfo->flRange);

		// check if we hit a valid player / hitgroup on the player and increment total hits.
		if (CanHit(start, end, record, hitbox))
			++total_hits;

		// we made it.
		if (total_hits >= needed_hits)
			return true;

		// we cant make it anymore.
		if ((SEED_MAX - i + total_hits) < needed_hits)
			return false;
	}

	return false;
}

bool AimPlayer::SetupHitboxPoints(Animation* record, BoneArray* bones, int index, std::vector< Vector >& points) {
	// reset points.
	points.clear();

	const model_t* model = record->player->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_MdlInfo->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(record->player->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->GetHitbox(index);
	if (!bbox)
		return false;

	// get hitbox scales.
	float scale = g_Options.ragebot_pointscale[curGroup] * 0.01f;

	// big inair fix.
	if (!(record->flags) & FL_ONGROUND)
		scale = 0.7f;

	float bscale = g_Options.ragebot_bodyscale[curGroup] * 0.01f;

	// these indexes represent boxes.
	if (bbox->m_flRadius <= 0.f) {
		// references: 
		//      https://developer.valvesoftware.com/wiki/Rotation_Tutorial
		//      CBaseAnimating::GetHitboxBonePosition
		//      CBaseAnimating::DrawServerHitboxes

		// convert rotation angle to a matrix.
		matrix3x4_t rot_matrix;
		Math::AngleMatrix(bbox->rotation, rot_matrix);

		// apply the rotation to the entity input space (local).
		matrix3x4_t matrix;
		Math::ConcatTransforms(bones[bbox->bone], rot_matrix, matrix);

		// extract origin from matrix.
		Vector origin = matrix.GetOrigin();

		// compute raw center point.
		Vector center = (bbox->bbmin + bbox->bbmax) / 2.f;

		// the feet hiboxes have a side, heel and the toe.
		if (index == HITBOX_RIGHT_FOOT || index == HITBOX_LEFT_FOOT) {
			float d1 = (bbox->bbmin.z - center.z) * 0.875f;

			// invert.
			if (index == HITBOX_LEFT_FOOT)
				d1 *= -1.f;

			// side is more optimal then center.
			points.push_back({ center.x, center.y, center.z + d1 });

				// get point offset relative to center point
				// and factor in hitbox scale.
				float d2 = (bbox->bbmax.x - center.x) * scale;
				float d3 = (bbox->bbmax.x - center.x) * scale;

				// heel.
				points.push_back({ center.x + d2, center.y, center.z });

				// toe.
				points.push_back({ center.x + d3, center.y, center.z });
			
		}

		// nothing to do here we are done.
		if (points.empty())
			return false;

		// rotate our bbox points by their correct angle
		// and convert our points to world space.
		for (auto& p : points) {
			// VectorRotate.
			// rotate point by angle stored in matrix.
			p = { p.Dot(matrix[0]), p.Dot(matrix[1]), p.Dot(matrix[2]) };

			// transform point to world space.
			p += origin;
		}
	}

	// these hitboxes are capsules.
	else {
		// factor in the pointscale.
		float r = bbox->m_flRadius * scale;
		float br = bbox->m_flRadius * bscale;

		// compute raw center point.
		Vector center = (bbox->bbmin + bbox->bbmax) / 2.f;

		// head has 5 points.
		if (index == HITBOX_HEAD) {
			// add center.
			points.push_back(center);

			bool pizda = true;

			if (pizda) {
				// rotation matrix 45 degrees.
				// https://math.stackexchange.com/questions/383321/rotating-x-y-points-45-degrees
				// std::cos( deg_to_rad( 45.f ) )
				constexpr float rotation = 0.70710678f;

				// top/back 45 deg.
				// this is the best spot to shoot at.
				points.push_back({ bbox->bbmax.x + (rotation * r), bbox->bbmax.y + (-rotation * r), bbox->bbmax.z });

				Vector right{ bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + r };

				// right.
				points.push_back(right);

				Vector left{ bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - r };

				// left.
				points.push_back(left);

				// back.
				points.push_back({ bbox->bbmax.x, bbox->bbmax.y - r, bbox->bbmax.z });

				// get animstate ptr.
				CCSGOPlayerAnimState* state = record->player->GetPlayerAnimState();

				// add this point only under really specific circumstances.
				// if we are standing still and have the lowest possible pitch pose.
				if (state && record->velocity.Length() <= 0.1f && record->eye_angles.pitch <= 89.9f)  {

					// bottom point.
					points.push_back({ bbox->bbmax.x - r, bbox->bbmax.y, bbox->bbmax.z });
				}
			}
		}

		// body has 5 points.
		else if (index == HITBOX_STOMACH) {
			// center.
			points.push_back(center);

			// back.
			points.push_back({ center.x, bbox->bbmax.y - br, center.z });
		}

		else if (index == HITBOX_PELVIS || index == HITBOX_UPPER_CHEST) {
			// back.
			points.push_back({ center.x, bbox->bbmax.y - r, center.z });
		}

		// other stomach/chest hitboxes have 2 points.
		else if (index == HITBOX_LOWER_CHEST || index == HITBOX_CHEST) {
			// add center.
			points.push_back(center);

			// add extra point on back.
				points.push_back({ center.x, bbox->bbmax.y - r, center.z });
		}

		else if (index == HITBOX_RIGHT_CALF || index == HITBOX_LEFT_CALF) {
			// add center.
			points.push_back(center);

			// half bottom.
				points.push_back({ bbox->bbmax.x - (bbox->m_flRadius / 2.f), bbox->bbmax.y, bbox->bbmax.z });
		}

		else if (index == HITBOX_RIGHT_THIGH || index == HITBOX_LEFT_THIGH) {
			// add center.
			points.push_back(center);
		}

		// arms get only one point.
		else if (index == HITBOX_RIGHT_UPPER_ARM || index == HITBOX_LEFT_UPPER_ARM) {
			// elbow.
			points.push_back({ bbox->bbmax.x + bbox->m_flRadius, center.y, center.z });
		}

		// nothing left to do here.
		if (points.empty())
			return false;

		// transform capsule points.
		for (auto& p : points)
			Math::VectorTransform(p, bones[bbox->bone], p);
	}

	return true;
}


bool AimPlayer::GetBestAimPosition(Vector& aim, float& damage, int& hitbox, Animation* record, float& tmp_min_damage) {
	bool                  done, pen;
	float                 dmg, pendmg;
	HitscanData_t         scan;
	std::vector< Vector > points;

	// get player hp.
	int hp = std::min(100, record->player->m_iHealth());

	m_matrix = record->m_pMatrix_Resolved;

	if (!m_matrix)
		return false;


	else {
		dmg = g_Options.ragebot_vis_mindamage[curGroup];


		if (GetAsyncKeyState(g_Options.ragebot_mindamage_override_key)) {
			dmg = g_Options.ragebot_mindamage_override[curGroup];
		}

		pendmg = g_Options.ragebot_mindamage[1];

		if (GetAsyncKeyState(g_Options.ragebot_mindamage_override_key)) {
			pendmg = g_Options.ragebot_mindamage_override[curGroup];
		}

		pen = g_Options.ragebot_autowall;
	}

	// backup player
	const auto backup_origin = record->player->m_vecOrigin();
	const auto backup_abs_origin = record->player->m_angAbsOrigin();
	const auto backup_abs_angles = record->player->m_angAbsAngles();
	const auto backup_obb_mins = record->player->m_vecMins();
	const auto backup_obb_maxs = record->player->m_vecMaxs();
	const auto backup_cache = record->player->get_bone_cache();

	auto restore = [&]() -> void {
		record->player->m_vecOrigin() = backup_origin;
		record->player->SetAbsOrigin(backup_abs_origin);
		record->player->SetAbsAngles(backup_abs_angles);
		record->player->m_vecMins() = backup_obb_mins;
		record->player->m_vecMaxs() = backup_obb_maxs;
		record->player->get_bone_cache() = backup_cache;
	};

	// iterate hitboxes.
	for (const auto& it : m_hitboxes) {
		done = false;

		// setup points on hitbox.
		if (!SetupHitboxPoints(record, m_matrix, it.m_index, points)) {
			continue;
		}

		// iterate points on hitbox.
		for (const auto& point : points) {
			WallPeneration::PenetrationInput_t in;

			in.m_damage     = dmg;
			in.m_damage_pen = pendmg;
			in.m_can_pen    = pen;
			in.m_target     = record->player;
			in.m_from       = g_LocalPlayer;
			in.m_pos        = point;

			// ignore mindmg.
			//if(/*(it.m_mode == HitscanMode::PREFER && it.m_safepoint) ||*/ it.m_mode == HitscanMode::LETHAL || it.m_mode == HitscanMode::LETHAL2 )
				//in.m_damage = in.m_damage_pen = 1.f;

			WallPeneration::PenetrationOutput_t out;

			// tests for intersection with unresolved matrix, if it returns true, the point should (!) be a safe point
			bool is_safepoint = Aimbot::Get().CanHit(g_LocalPlayer->GetShootPos(), point, record, it.m_index, true, record->m_pMatrix);

			// we only want safe pointable (nice word) hitboxes when forcing..
			if (!is_safepoint && g_Options.ragebot_safepoint)
				continue;

			//	if (is_safepoint && it.m_safepoint)
					//g_csgo.m_debug_overlay->AddLineOverlay(g_cl.m_shoot_pos, point, 255, 255, 255, false, 0.1f);

				// setup trace data
			record->player->m_vecOrigin() = record->origin;
			record->player->SetAbsOrigin(record->abs_origin);
			record->player->SetAbsAngles(record->abs_ang);
			record->player->m_vecMins() = record->obb_mins;
			record->player->m_vecMaxs() = record->obb_maxs;
			record->player->get_bone_cache() = reinterpret_cast<matrix3x4_t**>(m_matrix);

			// we can hit p!
			if (WallPeneration::Run(&in, &out)) {
				tmp_min_damage = dmg;

				// restore trace data
				restore();

				// nope we did not hit head..
				if (it.m_index == HITBOX_HEAD && out.m_hitgroup != HITGROUP_HEAD)
					continue;

				// prefered hitbox, just stop now.
				if (it.m_mode == HitscanMode::PREFER && !it.m_safepoint)
					done = true;

				// this hitbox requires lethality to get selected, if that is the case.
				// we are done, stop now.
				else if (it.m_mode == HitscanMode::LETHAL && out.m_damage >= record->player->m_iHealth())
					done = true;

				// 2 shots will be sufficient to kill.
				else if (it.m_mode == HitscanMode::LETHAL2 && (out.m_damage * 2.f) >= record->player->m_iHealth())
					done = true;

				// always prefer safe points if we want to.
				else if (it.m_mode == HitscanMode::PREFER_SAFEPOINT && it.m_safepoint && is_safepoint)
					done = true;

				// this hitbox has normal selection, it needs to have more damage.
				else if (it.m_mode == HitscanMode::NORMAL) {
					// we did more damage.
					if (out.m_damage > scan.m_damage) {
						// save new best data.
						scan.m_damage = out.m_damage;
						scan.m_pos = point;

						scan.m_hitbox = it.m_index;

						scan.m_safepoint = it.m_safepoint;

						// if the first point is lethal
						// screw the other ones.
						if (point == points.front() && out.m_damage >= record->player->m_iHealth())
							break;
					}
				}

				// we found a preferred / lethal hitbox.
				if (done) {
					// save new best data.
					scan.m_damage = out.m_damage;
					scan.m_pos = point;
					scan.m_hitbox = it.m_index;
					scan.m_safepoint = it.m_mode == HitscanMode::PREFER && it.m_safepoint;
					scan.m_mode = it.m_mode;

					break;
				}
			}
			else {
				// restore trace data
				restore();
			}
		}

		// ghetto break out of outer loop.
		if (done)
			break;
	}

	// we found something that we can damage.
	// set out vars.
	if (scan.m_damage > 0.f) {
		aim = scan.m_pos;
		damage = scan.m_damage;
		hitbox = scan.m_hitbox;

		Aimbot::Get().m_current_matrix = m_matrix;

		return true;
	}

	return false;
}

bool Aimbot::SelectTarget(Animation* record, const Vector& aim, float damage) {
	float dist, fov, height;
	int   hp;

	// fov check.
	if (g_Options.ragebot_limit_fov) {
		// if out of fov, retn false.
		if (math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, aim) > g_cfg[XOR("rage_aimbot_limit_fov_amount")].get<float>())
			return false;
	}

	switch (g_Options.ragebot_selection) {

		// distance.
	case 0:
		dist = g_LocalPlayer->GetShootPos().DistTo(record->origin);

		if (dist < m_best_dist) {
			m_best_dist = dist;
			return true;
		}

		break;

		// crosshair.
	case 1:
		fov = Math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, aim);

		if (fov < m_best_fov) {
			m_best_fov = fov;
			return true;
		}

		break;

		// damage.
	case 2:
		if (damage > m_best_damage) {
			m_best_damage = damage;
			return true;
		}

		break;

		// lowest hp.
	case 3:
		// fix for retarded servers?
		hp = std::min(100, record->player->m_iHealth());

		if (hp < m_best_hp) {
			m_best_hp = hp;
			return true;
		}

		break;

		// least lag.
	case 4:
		if (record->lag < m_best_lag) {
			m_best_lag = record->lag;
			return true;
		}

		break;

		// height.
	case 5:
		height = record->origin.z - g_LocalPlayer->m_vecOrigin().z;

		if (height < m_best_height) {
			m_best_height = height;
			return true;
		}

		break;

	default:
		return false;
	}

	return false;
}

void Aimbot::apply() {
	bool attack, attack2;

	// attack states.
	attack = (mcmd->buttons & IN_ATTACK);
	attack2 = (g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER && mcmd->buttons & IN_ATTACK2);

	// ensure we're attacking.
	if (attack || attack2) {
		// dont choke every shot.
		Snakeware::bSendPacket = true;

		if (m_shoot_next_tick)
			m_shoot_next_tick = false;

		if (m_target) {
			// make sure to aim at un-interpolated data.
			// do this so BacktrackEntity selects the exact record.
			if (m_record)
				mcmd->tick_count = TIME_TO_TICKS(m_record->sim_time + g_cl.m_lerp);

			// set angles to target.
			    mcmd->viewangles = m_angle;

			// if not silent aim, apply the viewangles.
			if (!g_Options.ragebot_silent)
				g_EngineClient->SetViewAngles(&m_angle);

			//g_visuals.DrawHitboxMatrix(m_record, colors::white, 10.f);
		}

		// nospread.
		if (g_Options.ragebot_remove_recoil && g_menu.main.config.mode.get() == 1)
			NoSpread();

		// norecoil.
		if (g_menu.main.aimbot.norecoil.get())
			mcmd->viewangles -= g_LocalPlayer->m_aimPunchAngle() * g_csgo.weapon_recoil_scale->GetFloat();

		// store fired shot.
		//g_shots.OnShotFire( m_target ? m_target : nullptr, m_target ? m_damage : -1.f, g_cl.m_weapon_info->m_bullets, m_target ? m_record : nullptr );

		// set that we fired.
		Snakeware::OnShot = true; // g_cl.m_shot = true;
	}
}

void Aimbot::NoSpread() {
	bool    attack2;
	Vector  spread, forward, right, up, dir;
	auto    weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!mcmd || !weapon)
		return;

	// revolver state.
	attack2 = (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER && (mcmd->buttons & IN_ATTACK2));

	// get spread.
	spread = weapon->CalculateSpread(mcmd->random_seed, attack2);

	// compensate.
	mcmd->viewangles -= { -Math::Rad2Deg(std::atan(spread.Length2D())), 0.f, Math::Rad2Deg(std::atan2(spread.x, spread.y)) };
}


bool Aimbot::CanHit(Vector start, Vector end, Animation* record, int box, bool in_shot, BoneArray* bones) {
	if (!record || !record->player)
		return false;

	// backup player
	const auto backup_origin = record->player->m_vecOrigin();
	const auto backup_abs_origin = record->player->m_angAbsOrigin();
	const auto backup_abs_angles = record->player->m_angAbsAngles();
	const auto backup_obb_mins = record->player->m_vecMins();
	const auto backup_obb_maxs = record->player->m_vecMaxs();
	const auto backup_cache = record->player->get_bone_cache();

	// always try to use our aimbot matrix first.
	auto matrix = m_current_matrix;

	// this is basically for using a custom matrix.
	if (in_shot)
		matrix = bones;

	if (!matrix)
		return false;

	const model_t* model = record->player->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_MdlInfo->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(record->player->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->GetHitbox(box);
	if (!bbox)
		return false;

	Vector min, max;
	const auto IsCapsule = bbox->m_flRadius != -1.f;

	if (IsCapsule) {
		Math::VectorTransform(bbox->bbmin, matrix[bbox->bone], min);
		Math::VectorTransform(bbox->bbmax, matrix[bbox->bone], max);
		const auto dist = Math::Segment2Segment(start, end, min, max);

		if (dist < bbox->m_flRadius) {
			return true;
		}
	}
	else {
		CGameTrace tr;

		// setup trace data
		record->player->m_vecOrigin() = record->origin;
		record->player->SetAbsOrigin   (record->abs_origin);
		record->player->SetAbsAngles   (record->abs_ang);
		record->player->m_vecMins()   = record->obb_mins;
		record->player->m_vecMaxs()   = record->obb_maxs;
		record->player->get_bone_cache() = reinterpret_cast<matrix3x4_t**>(matrix);

		// setup ray and trace.
		Ray_t tt;
		tt.Init(start, end);
		g_EngineTrace->ClipRayToEntity(tt, MASK_SHOT, record->player, &tr);

		record->player->m_vecOrigin() = backup_origin;
		record->player->SetAbsOrigin(backup_abs_origin);
		record->player->SetAbsAngles(backup_abs_angles);
		record->player->m_vecMins() = backup_obb_mins;
		record->player->m_vecMaxs() = backup_obb_maxs;
		record->player->get_bone_cache() = backup_cache;

		// check if we hit a valid player / hitgroup on the player and increment total hits.
		if (tr.hit_entity == record->player && Utils::IsValidHitgroup(tr.hitgroup))
			return true;
	}

	return false;
}

