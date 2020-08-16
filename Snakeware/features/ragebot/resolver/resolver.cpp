
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

void Resolver::Resolve(C_BasePlayer* player,Animation * record) {
	if (!IsCheater(player)) return;
	if (!record || !player) return;
	auto state = player->GetPlayerAnimState();
	if (!state) return;
	
	std::stringstream ss;
	std::stringstream ss1;
	std::stringstream ss2;
	std::stringstream ss3;
	std::stringstream ss7;

	ss << "Playbackrate : " << record->layers[6].m_flPlaybackRate;
	ss2 << "flWeight[3] : " << record->layers[3].m_flWeight;
	ss3 << "flCycle[7] : " << record->layers[7].m_flCycle;
	ss7 << "flweight[7] : "   << record->layers[7].m_flWeight;
	

	Snakeware::Delta = ss.str();
	Snakeware::EyeDelta = ss7.str();
	Snakeware::Delta2 = ss2.str();
	Snakeware::Delta3 = ss3.str();
}