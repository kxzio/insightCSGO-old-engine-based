
#include "resolver.h"
#include "../../../options.hpp"
#include "../autowall/ragebot-autowall.h"
#include <algorithm>


float AngleNormalize(float angle) {
	angle = fmodf(angle, 360.0f);
	if (angle > 180)
	{
		angle -= 360;
	}
	if (angle < -180)
	{
		angle += 360;
	}
	return angle;
}


float AngleDiff2(float destAngle, float srcAngle) {
	float delta;

	delta = fmodf(destAngle - srcAngle, 360.0f);
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

bool IsCheater(C_BasePlayer * player) {
     // credits : @LNK1181 aka platina300
	 const auto choked_ticks = std::max(0, TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime()) - 1);
	 if (choked_ticks > 1)
		 return true;
	 return false;
}


void Resolver::Resolve(C_BasePlayer * player) {

	auto state = player->GetPlayerAnimState();
	auto yaw = Math::NormalizeYaw(player->m_angEyeAngles().yaw);
	auto Left = Math::NormalizeYaw(player->m_angEyeAngles().yaw - 70.f);
	auto Right = Math::NormalizeYaw(player->m_angEyeAngles().yaw + 70.f);
	auto &record = resolveInfo[player->EntIndex()];

	if (!IsCheater(player)) return;

	auto missed = record.m_nShotsMissed;
	if (missed > 1 && player->m_vecVelocity().Length2D() < 170) {
		switch (missed) {
		case 0: 
			yaw = Left; break;
		case 1:
			yaw = Right; break;
		case 2:
			yaw = yaw; break;
		}
		Math::NormalizeAngleYaw(state->m_flGoalFeetYaw);
		state->m_flGoalFeetYaw = yaw;
	}
}

void Resolver::ResolvePitch(C_BasePlayer * player) {
	if (IsCheater(player));
   player->m_angEyeAngles().pitch = std::clamp( Math::NormalizeAngleYaw(player->m_angEyeAngles().pitch), -89.f, 89.f);

}

