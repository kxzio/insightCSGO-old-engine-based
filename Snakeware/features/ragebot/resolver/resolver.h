#pragma once
#include "../../../valve_sdk/csgostructs.hpp"
#include "../../../helpers/math.hpp"
#include "../lagcompensation/lag-compensation.h"
#include "../animation-system/animation-system.h"
class QAngle;
class C_BasePlayer;

struct ResolveInfo{
	int m_flSide;
	int32_t m_nShotsMissed = 0;
};

class Resolver : public Singleton<Resolver>
{
public :
	ResolveInfo resolveInfo[65];

	int m_flSide = 0;

	void DetectDesyncSide(C_BasePlayer * player);

	void Resolve(Animation * anim);

	void ResolvePitch(C_BasePlayer* player);

};

