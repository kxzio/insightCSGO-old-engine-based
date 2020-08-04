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


float AngleDiff1337(float destAngle, float srcAngle) {
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

	auto OldGoalFeet = anim->anim_state->m_flGoalFeetYaw;
	auto AyeAngles = Math::NormalizeYaw(anim->player->m_angEyeAngles().yaw);
	auto Left = AyeAngles - 60;
	auto Right = AyeAngles + 60;
	
	switch (Snakeware::MissedShots[anim->player->EntIndex()] % 3)
	{
	case 0: OldGoalFeet += Left;      break;
	case 1: OldGoalFeet += Right;     break;
	case 2: OldGoalFeet += AyeAngles; break;
	}
	Math::NormalizeYaw(OldGoalFeet);
	anim->anim_state->m_flGoalFeetYaw = OldGoalFeet;
	
}
