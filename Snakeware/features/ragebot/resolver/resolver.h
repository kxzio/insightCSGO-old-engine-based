#pragma once
#include "../../../valve_sdk/csgostructs.hpp"
#include "../../../helpers/math.hpp"
#include "../lagcompensation/lag-compensation.h"
#include "../animation-system/animation-system.h"
class QAngle;
class C_BasePlayer;


class Resolver : public Singleton<Resolver>
{
public :


	int                     m_flSide[65];
	AnimationLayer          ResolvedAnimLayer[3][15];

	void DetectDesyncSide   (Animation * animation);

	void Resolve            (C_BasePlayer * player);

	void Antifreestand();

	void ResolvePitch       (C_BasePlayer* player);

	void StoreAnimOverlays  (C_BasePlayer * player);

	void                     OnPlayerHurt  (IGameEvent * event);
	void                     OnBulletImpact(IGameEvent * event);

	//StoreMissedShot's like rifk cheat
	int      LastHitbox;
	int      LastMissedShotIndex;
	int      MissedShot2Spread  [65];
	int      MissedShot2Resolver[65];
	matrix3x4_t LastBones       [128];
	Vector   LastEyePos;

};

