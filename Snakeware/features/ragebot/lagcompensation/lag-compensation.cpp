#include "../../../valve_sdk/csgostructs.hpp"
#include "../../../helpers/math.hpp"
#include "../player-log/player-log.h"
#include "lag-compensation.h"




bool LagCompensation::ValidSimtime(const float & simtime) {
	const auto nci = g_EngineClient->GetNetChannelInfo();
	if (!nci)
		return false;

	float correct = 0;

	correct += nci->GetLatency(FLOW_OUTGOING);
	correct += nci->GetLatency(FLOW_INCOMING);
	correct += GetLerpTime();

	const auto delta_time = correct - (g_GlobalVars->curtime - simtime);

	return fabsf(delta_time) <= 0.2f && correct < 1.f;
}

float LagCompensation::GetLerpTime(){
	

		static ConVar* updaterate = g_CVar->FindVar("cl_updaterate");
		static ConVar* minupdate = g_CVar->FindVar("sv_minupdaterate");
		static ConVar* maxupdate = g_CVar->FindVar("sv_maxupdaterate");
		static ConVar* lerp = g_CVar->FindVar("cl_interp");
		static ConVar* cmin = g_CVar->FindVar("sv_client_min_interp_ratio");
		static ConVar* cmax = g_CVar->FindVar("sv_client_max_interp_ratio");
		static ConVar* ratio = g_CVar->FindVar("cl_interp_ratio");

		float lerpurmom = lerp->GetFloat(), maxupdateurmom = maxupdate->GetFloat(),
			ratiourmom = ratio->GetFloat(), cminurmom = cmin->GetFloat(), cmaxurmom = cmax->GetFloat();
		int updaterateurmom = updaterate->GetInt(),
			sv_maxupdaterate = maxupdate->GetInt(), sv_minupdaterate = minupdate->GetInt();

		if (sv_maxupdaterate && sv_minupdaterate)
			updaterateurmom = maxupdateurmom;

		if (ratiourmom == 0)
			ratiourmom = 1.0f;

		if (cmin && cmax && cmin->GetFloat() != 1)
			ratiourmom = std::clamp(ratiourmom, cminurmom, cmaxurmom);

		return std::max(lerpurmom, ratiourmom / updaterateurmom);
	
}
