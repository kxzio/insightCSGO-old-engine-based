#pragma once
#include "../../valve_sdk/csgostructs.hpp"

class Nightmode {
public:
	void Run() ;
	void Apply() ;
	void Remove() ;
};

extern Nightmode g_Nightmode;