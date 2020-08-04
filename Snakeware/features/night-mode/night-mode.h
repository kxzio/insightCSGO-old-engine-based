#pragma once
#include "../../valve_sdk/csgostructs.hpp"

class Nightmode {
public:
	void Run() noexcept;
	void Apply() noexcept;
	void Remove() noexcept;
};

extern Nightmode g_Nightmode;