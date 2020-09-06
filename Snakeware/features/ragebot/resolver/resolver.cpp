
#include "resolver.h"
#include "../../../options.hpp"
#include "../autowall/ragebot-autowall.h"
#include "../ragebot.h"
#include <algorithm>
#include "../../../valve_sdk/interfaces/IGameEventmanager.hpp"
#include "../../event-logger/event-logger.h"

static float ResolvedYaw[65];

int shots_hit[65];
int shots_fire[65];
bool IsCheater(C_BasePlayer * player) {
     // credits : @LNK1181 aka platina300
	 const auto choked_ticks = std::max(0, TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime()) - 1);
	 if (choked_ticks >= 1)
		 return true;

	 return false;
}

float AngleDiffPidoras(float destAngle, float srcAngle) {
	float delta = fmodf(destAngle - srcAngle, 360.0f);

	if (destAngle > srcAngle) {
		if (delta >= 180)
			delta -= 360;
	}
	else {
		if (delta <= -180)
			delta += 360;
	}

	return delta;
}
inline float FixAngle(float angle) {
	return remainderf(angle, 360.0f);
}


	
	
	

inline float NormalizeFloat(float angle) {
	// by @llama & @sharklaser1
	return remainderf(angle, 360.0f);
}



void Resolver::ResolvePitch(C_BasePlayer * player) {

	if (IsCheater(player))
    player->m_angEyeAngles().pitch = std::clamp(FixAngle(player->m_angEyeAngles().pitch), -89.9f, 89.9f);

}
void Resolver::StoreAnimOverlays(C_BasePlayer * player) {
	//Store some layer's here.
	
}

void Resolver::OnPlayerHurt(IGameEvent* event) {
	    if (!g_Options.ragebot_enabled)        return;

	    if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;

	    const auto Target = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

		int index = Target->EntIndex();
		MissedShot2Resolver[index]--;
		MissedShot2Resolver[index] =
		std::clamp(MissedShot2Resolver[index], 0, 99);
}


void Resolver::OnBulletImpact(IGameEvent* event) {
	if (!g_Options.ragebot_enabled)        return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;

	const auto Target = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

	if (!Target || Target != g_LocalPlayer) return;

	Vector pos(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));


	auto MissedShotDueToSpread = [&](Vector pos)
	{
		Vector aim_eye_pos = LastEyePos;
		QAngle impact_angle = Math::CalcAngle(aim_eye_pos, pos);

		Vector forward, right, up, new_angle, end;
		Math::AngleVectors(impact_angle, forward, right, up);
		Math::VectorAngles2(forward, new_angle);

		// calculate end point of trace.
		Math::AngleVectors2(new_angle, end);

		if (LastMissedShotIndex > 0)
		{
			const auto Studio = g_MdlInfo->GetStudioModel(Target->GetModel());
			if (!Studio) return;

			if (!can_hit_hitbox(LastEyePos,LastEyePos + end.Normalize() * 8192.f, LastBones, Studio, LastHitbox)) {
				MissedShot2Spread[LastMissedShotIndex]++;
				MissedShot2Spread[LastMissedShotIndex] = std::clamp(MissedShot2Spread[LastMissedShotIndex], 0, 99);

				EventLogs::Get().Add("missed due to spread", Color(255,0,120));
		
			}
			else
			{
				MissedShot2Resolver[LastMissedShotIndex]++;
				MissedShot2Resolver[LastMissedShotIndex] = std::clamp(MissedShot2Resolver[LastMissedShotIndex], 0, 99);
				EventLogs::Get().Add("missed due to animation desync", Color(255, 0, 120));
			}
		}
		LastMissedShotIndex = 0;
	};
	MissedShotDueToSpread(pos);
}

float sub_1001F720(float a1) {
	float v1; // ST0C_4
	float v2; // ST08_4

	if (a1 <= 180.0f) {
		if (a1 < -180.0f) {
			v2 = std::roundf(a1 / 360.0f);
			a1 = (float)(v2 * -360.0f) + a1;
		}
	}
	else {
		v1 = std::roundf(a1 / 360.0f);
		a1 = a1 - (float)(v1 * 360.0f);
	}

	return a1;
}

float sub_1001F720_1(float st7_0, float a1)
{
	float v2; // ST0C_4
	float v3; // ST08_4

	if (a1 <= 180.0) {
		if (a1 < -180.0) {
			v3 = std::roundf(a1 / 360.0f);
			a1 = (float)(v3 * -360.0) + a1;
		}
	}
	else {
		v2 = std::roundf(a1 / 360.0f);
		a1 = a1 - (float)(v2 * 360.0);
	}

	return a1;
}

float __stdcall sub_1000F820235235253(float a1, float a2, float a3, float a4, float a5) {
	float v6; // [esp+8h] [ebp-4h]

	if (a2 != a3)
		return (float)((float)((float)((float)(a5 - a4) * (float)(a1 - a2)) / (float)(a3 - a2)) + a4);
	if (a1 < a3)
		v6 = a4;
	else
		v6 = a5;

	return v6;
}

void Resolver::GetFlags(C_BasePlayer* player, Animation* record)
{
	if (!record || !player) return;
	std::stringstream ss;
	std::stringstream ss1;
	std::stringstream ss2;
	std::stringstream ss3;
	std::stringstream ss7;

	ss << "ResolveIndex : " << iResolvingWay;
	

	Snakeware::Delta = ss.str();
	
}


void Resolver::SetResolveIndex (C_BasePlayer* player, int m_iSide) {
	// Safe-point resolver...
	// Credit's : @Snake && Onetap crack & v3 dump's

	auto state = player->GetPlayerAnimState();
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())  return;

	float m_flResolved = 0.f;

	if (player->m_fFlags() & FL_ONGROUND) {

		// Choked == 1 // Like onetap cheat
		if (IsCheater(player)) 		{

			if (m_iSide) {

				if (m_iSide <= 0)
					m_flResolved = player->m_angEyeAngles().yaw - 60.f; // viewangles.y - *&ConstFloat::60;
				else
					m_flResolved = player->m_angEyeAngles().yaw + 60.f; // viewangles.y + *&ConstFloat::60;

				Math::NormalizeYaw(m_flResolved); // Some normalize for yaw-angle
			}
			else {

				if (m_iSide <= 0)
					m_flResolved = player->m_angEyeAngles().yaw - 58.f;       // LeftDormancy
				else
					m_flResolved = player->m_angEyeAngles().yaw + 58.f;       // RightDormancy

				Math::NormalizeYaw(m_flResolved); // Some normalize for dormancy

			}

			state->m_flGoalFeetYaw = m_flResolved; // *(*(v10 + v11 - 16) + 0x80) = ResolvedYaw;
		}


	}

	Animations::Get().UpdatePlayer(player); // Update player animation's

	// Some rebulid's / update's / store here..
}


void Resolver::UpdateResolve(Animation * record ,C_BasePlayer* player) {
	// OneTap cheat reverse.
	// Credit's : @Snake.
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame()) return;

	auto EyeDiff = AngleDiffPidoras(player->m_angEyeAngles().yaw, record->anim_state->m_flGoalFeetYaw);
	auto Diff = EyeDiff < 0.0f;

	if (Diff)
	{
		std::memcpy(ResolvedAnimLayer[1], player->GetAnimOverlays(), 0x38 * player->GetNumAnimOverlays());
		std::memcpy(player->GetAnimOverlays(), ServerAnimLayer, 0x38      * player->GetNumAnimOverlays());
	}
	else
	{
		std::memcpy(ResolvedAnimLayer[2], player->GetAnimOverlays(), 0x38 * player->GetNumAnimOverlays());
		std::memcpy(player->GetAnimOverlays(), ServerAnimLayer, 0x38 * player->GetNumAnimOverlays());

	}
	std::memcpy (ResolvedAnimLayer[0],      player->GetAnimOverlays(), 0x38 * player->GetNumAnimOverlays());
	std::memcpy (player->GetAnimOverlays(), ServerAnimLayer, 0x38 * player->GetNumAnimOverlays          ());

	if (record) {

		WillUpdate = false;
		if (player->m_fFlags() & FL_ONGROUND || record->flags & FL_ONGROUND) {

			if (player->m_vecVelocity().Length2D() <= 0.1f) {

				if (ServerAnimLayer[3].m_flWeight == 0.0f && ServerAnimLayer[3].m_flCycle == 0.0f) {
					iResolvingWay = Math::Clamp((2 * (EyeDiff <= 0.f) - 1), -1, 1);
					WillUpdate = true;
				}
			}
			else {

				float Rate1 = fabsf(ServerAnimLayer[6].m_flPlaybackRate - ResolvedAnimLayer[0][6].m_flPlaybackRate);
				float Rate2 = fabsf(ServerAnimLayer[6].m_flPlaybackRate - ResolvedAnimLayer[1][6].m_flPlaybackRate);
				float Rate3 = fabsf(ServerAnimLayer[6].m_flPlaybackRate - ResolvedAnimLayer[2][6].m_flPlaybackRate);
				if (Rate1 < Rate3 || Rate2 <= Rate3 || (int)(float)(Rate3 * 1000.0f)) {
					if (Rate1 >= Rate2 && Rate3 > Rate2 && !(int)(float)(Rate2 * 1000.0f)) {
						
						iResolvingWay = 1;
						WillUpdate = true;
						
					}
				}
				else {
					iResolvingWay = -1;
					WillUpdate = true;
				}


			}

		}

	}
	//GetFlags(player, record);
	
}