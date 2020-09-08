#pragma once
#include "../../../valve_sdk/csgostructs.hpp"
#include "../../../helpers/math.hpp"
#include "../lagcompensation/lag-compensation.h"
#include "../animation-system/animation-system.h"
class QAngle;
class C_BasePlayer;

struct ResolveInfo {

	ResolveInfo() { };

};
class Resolver : public Singleton<Resolver>
{
public :


	int                     iResolvingWay;
	bool                    WillUpdate;
	AnimationLayer          ResolvedAnimLayer[3][15];
	AnimationLayer          ServerAnimLayer  [15];

	void GetFlags(C_BasePlayer * player, Animation * record);

	void StoreAntifreestand();

	void ResolveFreestand(C_BasePlayer * player);

	void SetResolveIndex    (C_BasePlayer * player, int m_flSide);

	void UpdateResolve      (Animation * record,C_BasePlayer * player);



	void ResolvePitch       (C_BasePlayer* player);

	void StoreAnimOverlays  (C_BasePlayer * player);

	void                     OnPlayerHurt  (IGameEvent * event);
	void                     OnBulletImpact(IGameEvent * event);

	//StoreMissedShot's like rifk cheat

	 int shots_missed[65];
	 int shots_hit[65];
	 int shots_fire[65];

	int      LastHitbox;
	int      LastMissedShotIndex;
	int      MissedShot2Spread  [65];
	int      MissedShot2Resolver[65];
	matrix3x4_t LastBones       [128];
	int m_iFreestandSide[64];
	bool WillUpdated;
	Vector   LastEyePos;
	int resolvermode;

};

