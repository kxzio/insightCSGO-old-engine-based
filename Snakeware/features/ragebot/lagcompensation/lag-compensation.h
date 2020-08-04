#pragma once
#include "../../../valve_sdk/csgostructs.hpp"
#include <optional>
#include <array>


class LagCompensation : public Singleton<LagCompensation>
{
public:
	static int   FixTickount   (const float& simtime);
	static bool  ValidSimtime  (const float & simtime);
	static bool  CheckPing     (const float & simtime, int * wish_sequence_nr, int * wish_reliable_state);
	static float GetLerpTime   ();
	static void  Extrapolate   (C_BasePlayer* player, Vector& origin, Vector& velocity, int& flags, bool wasonground);
};
