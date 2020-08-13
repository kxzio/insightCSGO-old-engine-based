
#include "resolver.h"
#include "../../../options.hpp"
#include "../autowall/ragebot-autowall.h"
#include "../ragebot.h"
#include <algorithm>
#include "../../../valve_sdk/interfaces/IGameEventmanager.hpp"
#include "../../event-logger/event-logger.h"



bool IsCheater(C_BasePlayer * player) {
     // credits : @LNK1181 aka platina300
	 const auto choked_ticks = std::max(0, TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime()) - 1);
	 if (choked_ticks > 1)
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


	void Resolver::Antifreestand() {
		if (!g_LocalPlayer->IsAlive())
			return;

		// pasted trash

		for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i)
		{
			C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

			if (!player || !player->IsAlive() || player->IsDormant() || player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
				continue;

			bool Autowalled = false, HitSide1 = false, HitSide2 = false;
			auto idx = player->EntIndex();
			float angToLocal = Math::CalculateAngle(g_LocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y;
			Vector ViewPoint = g_LocalPlayer->m_vecOrigin() + Vector(0, 0, 90);
			Vector2D Side1 = { (45 * sin(DEG2RAD(angToLocal))),(45 * cos(DEG2RAD(angToLocal))) };
			Vector2D Side2 = { (45 * sin(DEG2RAD(angToLocal + 180))) ,(45 * cos(DEG2RAD(angToLocal + 180))) };

			Vector2D Side3 = { (50 * sin(DEG2RAD(angToLocal))),(50 * cos(DEG2RAD(angToLocal))) };
			Vector2D Side4 = { (50 * sin(DEG2RAD(angToLocal + 180))) ,(50 * cos(DEG2RAD(angToLocal + 180))) };

			Vector Origin = player->m_vecOrigin();

			Vector2D OriginLeftRight[] = { Vector2D(Side1.x, Side1.y), Vector2D(Side2.x, Side2.y) };

			Vector2D OriginLeftRightLocal[] = { Vector2D(Side3.x, Side3.y), Vector2D(Side4.x, Side4.y) };

			for (int side = 0; side < 2; side++)
			{
				Vector OriginAutowall = { Origin.x + OriginLeftRight[side].x,  Origin.y - OriginLeftRight[side].y , Origin.z + 90 };
				Vector ViewPointAutowall = { ViewPoint.x + OriginLeftRightLocal[side].x,  ViewPoint.y - OriginLeftRightLocal[side].y , ViewPoint.z };

				if (g_AutoWall->CanHitFloatingPoint(OriginAutowall, ViewPoint))
				{
					if (side == 0)
					{
						HitSide1 = true;
						m_flSide[idx] = -1;
					}
					else if (side == 1)
					{
						HitSide2 = true;
						m_flSide[idx] = 1;
					}

					Autowalled = true;
				}
				else
				{
					for (int sidealternative = 0; sidealternative < 2; sidealternative++)
					{
						Vector ViewPointAutowallalternative = { Origin.x + OriginLeftRight[sidealternative].x,  Origin.y - OriginLeftRight[sidealternative].y , Origin.z + 90 };
						
						if (g_AutoWall->CanHitFloatingPoint(ViewPointAutowallalternative, ViewPointAutowall))
						{
							if (sidealternative == 0)
							{
								HitSide1 = true;
								m_flSide[idx] = -1;
								
							}
							else if (sidealternative == 1)
							{
								HitSide2 = true;
								m_flSide[idx] = 1;
								
							}

							Autowalled = true;
						}
					}
				}
			}
		}
	}

	
	

inline float NormalizeFloat(float angle) {
	// by @llama & @sharklaser1
	return remainderf(angle, 360.0f);
}



void Resolver::ResolvePitch(C_BasePlayer * player) {

	if (IsCheater(player))
    player->m_angEyeAngles().pitch = std::clamp(FixAngle(player->m_angEyeAngles().pitch), -89.f, 89.f);

}
void Resolver::StoreAnimOverlays(C_BasePlayer * player) {
	//Store some layer's here.
	
}

void Resolver::OnPlayerHurt(IGameEvent* event) {
	    if (!event || !g_Options.ragebot_enabled)        return;

	    if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;

	    const auto Target = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

		int index = Target->EntIndex();
		MissedShot2Resolver[index]--;
		MissedShot2Resolver[index] =
		std::clamp(MissedShot2Resolver[index], 0, 99);
}


void Resolver::OnBulletImpact(IGameEvent* event) {
	if (!event || !g_Options.ragebot_enabled)        return;

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
			else {
				MissedShot2Resolver[LastMissedShotIndex]++;
				MissedShot2Resolver[LastMissedShotIndex] = std::clamp(MissedShot2Resolver[LastMissedShotIndex], 0, 99);

				EventLogs::Get().Add("missed due to animation desync", Color(255, 0, 120));
			}
		}
		LastMissedShotIndex = 0;
	};
	MissedShotDueToSpread(pos);
}

void Resolver::Resolve(C_BasePlayer* player) {
	if (!IsCheater(player)) return;
	const auto Missed = MissedShot2Resolver[player->EntIndex()];
	const auto CurShot = Missed % 3;
	float      Resolved = player->m_angEyeAngles().yaw;
	auto       state = player->GetPlayerAnimState();
	if (!state) return;
	//if(!Missed)
	//Antifreestand();
	// beta

}