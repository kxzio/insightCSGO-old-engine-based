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


void Resolver::Resolve(Animation * anim) {


	auto animState = anim->player->GetPlayerAnimState();;
	if (!animState) return;
	float relative = Math::CalcAngle(g_LocalPlayer->GetEyePos(), anim->player->m_vecOrigin()).yaw + 180.f;
	float Left2 = animState->m_flEyeYaw + std::abs(relative - anim->player->m_angEyeAngles().yaw); // 
	float Right2 = animState->m_flEyeYaw - std::abs(relative - anim->player->m_angEyeAngles().yaw); //
	// static checks
	if (relative < anim->player->m_angEyeAngles().yaw)
	{
		animState->m_flGoalFeetYaw = Math::NormalizeYaw(Left2);
	}
	else if (relative > anim->player->m_angEyeAngles().yaw)
	{
		animState->m_flGoalFeetYaw = Math::NormalizeYaw(Right2);

	}
}
