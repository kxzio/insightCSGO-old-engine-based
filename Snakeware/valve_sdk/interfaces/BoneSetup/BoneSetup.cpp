#include "BoneSetup.h"
#include "../../../helpers/math.hpp"
// func
C_BasePlayer* GetPlayer(const int index) { return reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(index)); }

// Sdelau potom
bool sBoneSetup::HandleBoneSetup (Animation & record, const int bone_mask, matrix3x4_t * bone_out) {

	return false;
}
