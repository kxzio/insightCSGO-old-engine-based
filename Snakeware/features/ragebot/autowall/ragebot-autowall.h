#pragma once

#include "../../../valve_sdk/csgostructs.hpp"
#include "../../../helpers/math.hpp"
#include "../animation-system/animation-system.h"
#include <optional>


namespace WallPeneration {
	struct PenetrationInput_t {
		C_BasePlayer* m_from;
		C_BasePlayer* m_target;
		Vector        m_pos;
		float	      m_damage;
		float         m_damage_pen;
		bool	      m_can_pen;
	};

	struct PenetrationOutput_t {
		C_BasePlayer* m_target;
		float         m_damage;
		int           m_hitgroup;
		bool          m_pen;

		__forceinline PenetrationOutput_t() : m_target{ nullptr }, m_damage{ 0.f }, m_hitgroup{ -1 }, m_pen{ false } {}
	};

	float Scale(C_BasePlayer* player, float damage, float armor_ratio, int hitgroup);
	bool  IsBreakable(C_BasePlayer * e);
	bool  TraceToExit(const Vector& start, const Vector& dir, Vector& out, CGameTrace* enter_trace, CGameTrace* exit_trace);
	void  ClipTraceToPlayer(const Vector& start, const Vector& end, uint32_t mask, CGameTrace* tr, C_BasePlayer* player, float min);
	bool  Run(PenetrationInput_t* in, PenetrationOutput_t* out);
}