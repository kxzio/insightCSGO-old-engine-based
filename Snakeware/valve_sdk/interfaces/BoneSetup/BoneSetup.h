#pragma once
#include "../../csgostructs.hpp"
#include "../../../features/ragebot/animation-system/animation-system.h"
//Classes : 
class C_BaseAnimating;
class CBoneAccessor;


class sBoneSetup : public Singleton<sBoneSetup> {
	//s = Snake :D
    public:
	 
		static bool HandleBoneSetup        (Animation&    record, const int bone_mask, matrix3x4_t* bone_out);
		static bool HandleBoneSetupReplace (C_BasePlayer* player, const int max_bones, const int bone_mask, matrix3x4_t* bone_out);
};


class CBoneAccessor
{
public:
	inline matrix3x4_t *get_bone_array_for_write(void) const {
		return m_pBones; // return original bone.
	}
	inline void set_bone_array_for_write(matrix3x4_t* bonearray) {
		m_pBones = bonearray; // return setuped bone's
	}
	alignas(16) matrix3x4_t * m_pBones;

	int m_ReadableBones;		// Which bones can be read.
	int m_WritableBones;		// Which bones can be written.
};