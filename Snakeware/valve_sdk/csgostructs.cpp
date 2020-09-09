#include "csgostructs.hpp"
#include "../Helpers/Math.hpp"
#include "../Helpers/Utils.hpp"
#include <iterator>
#include "../Protected/enginer.h"
#include "../helpers/memory-address.h"
//increase it if valve added some funcs to baseentity :lillulmoa:
constexpr auto VALVE_ADDED_FUNCS = 0ull;

bool C_BaseEntity::IsPlayer()
{

	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 157 + VALVE_ADDED_FUNCS)(this);
}

bool C_BaseEntity::IsLoot() {
	return GetClientClass()->m_ClassID == ClassId_CPhysPropAmmoBox ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropLootCrate ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropRadarJammer ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropWeaponUpgrade ||
		GetClientClass()->m_ClassID == ClassId_CDrone ||
		GetClientClass()->m_ClassID == ClassId_CDronegun ||
		GetClientClass()->m_ClassID == ClassId_CItem_Healthshot ||
		GetClientClass()->m_ClassID == ClassId_CItemCash || 
		GetClientClass()->m_ClassID == ClassId_CBumpMine;
}

bool C_BaseEntity::IsWeapon()
{

	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 165 + VALVE_ADDED_FUNCS)(this);
}


bool C_BaseEntity::IsPlantedC4()
{
	return GetClientClass()->m_ClassID == ClassId_CPlantedC4;
}

bool C_BaseEntity::IsDefuseKit()
{
	return GetClientClass()->m_ClassID == ClassId_CBaseAnimating;
}

CCSWeaponInfo* C_BaseCombatWeapon::GetCSWeaponData()
{
	// fixed
	return CallVFunction<CCSWeaponInfo*(__thiscall*)(void*)>(this, 460)(this);
}

bool C_BaseCombatWeapon::HasBullets()
{
	return !IsReloading() && m_iClip1() > 0;
}
bool C_BaseCombatWeapon::IsWeaponNonAim()
{
	int idx = this->m_Item().m_iItemDefinitionIndex();

	return (idx == WEAPON_C4 || idx == WEAPON_KNIFE || idx == WEAPON_KNIFE_BAYONET || idx == WEAPON_KNIFE_BUTTERFLY || idx == WEAPON_KNIFE_FALCHION
		|| idx == WEAPON_KNIFE_FLIP || idx == WEAPON_KNIFE_GUT || idx == WEAPON_KNIFE_KARAMBIT || idx == WEAPON_KNIFE_M9_BAYONET || idx == WEAPON_KNIFE_PUSH
		|| idx == WEAPON_KNIFE_SURVIVAL_BOWIE || idx == WEAPON_KNIFE_T || idx == WEAPON_KNIFE_TACTICAL || idx == WEAPON_FLASHBANG || idx == WEAPON_HEGRENADE
		|| idx == WEAPON_SMOKEGRENADE || idx == WEAPON_MOLOTOV || idx == WEAPON_DECOY || idx == WEAPON_INC); 
	// pasted too
}

bool C_BaseCombatWeapon::CanFire()
{
	auto owner = this->m_hOwnerEntity().Get();
	if (!owner)
		return false;

	if (IsReloading() || m_iClip1() <= 0)
		return false;

	auto flServerTime = owner->m_nTickBase() * g_GlobalVars->interval_per_tick;

	if (owner->m_flNextAttack() > flServerTime)
		return false;


	return m_flNextPrimaryAttack() <= flServerTime;
}

bool C_BaseCombatWeapon::IsGrenade()
{
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_GRENADE;
}
bool C_BaseCombatWeapon::CanShiftTickbase () {
	if (!this)
		return false;

	if (this->m_Item().m_iItemDefinitionIndex() == 64 || this->m_Item().m_iItemDefinitionIndex() == 31 || this->m_Item().m_iItemDefinitionIndex() == 69 || this->m_Item().m_iItemDefinitionIndex() == 49)
		return false;

	if (this->IsKnife())
		return false;

	if (this->GetCSWeaponData()->iWeaponType == 1 || this->GetCSWeaponData()->iWeaponType == 10)
		return false;

	return true;
}
bool C_BaseCombatWeapon::IsGun()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_C4:
		return false;
	case WEAPONTYPE_GRENADE:
		return false;
	case WEAPONTYPE_KNIFE:
		return false;
	case WEAPONTYPE_UNKNOWN:
		return false;
	default:
		return true;
	}
}

bool C_BaseCombatWeapon::IsKnife()
{
	if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_ZEUS) return false;
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_KNIFE;
}

bool C_BaseCombatWeapon::IsRifle()
{
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_RIFLE; // updated
}

bool C_BaseCombatWeapon::IsZeus()
{
	if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_ZEUS)
	{
		return true;
	}
	return false;
}

bool C_BaseCombatWeapon::IsPistol()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_PISTOL:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsAuto()
{
	auto id = this->m_Item().m_iItemDefinitionIndex();
	if (id == WEAPON_G3SG1 || id == WEAPON_SCAR20)
		return true;
	return false;
}

bool C_BaseCombatWeapon::IsShotgun()
{
	auto id = this->m_Item().m_iItemDefinitionIndex();
	if (id == WEAPON_MAG7 || id == WEAPON_SAWEDOFF || id == WEAPON_XM1014 || id == WEAPON_NOVA)
		return true;
	return false;
}

bool C_BaseCombatWeapon::IsSMG()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_SUBMACHINEGUN:
		return true;
	default:
		return false;
	}
}
bool C_BaseCombatWeapon::IsMashineGun()
{
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_MACHINEGUN;
}

bool C_BaseCombatWeapon::IsSniper()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_SNIPER_RIFLE:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsReloading()
{
	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90") + 2);
	return *(bool*)((uintptr_t)this + inReload);
}

float C_BaseCombatWeapon::GetInaccuracy()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 482 + VALVE_ADDED_FUNCS)(this);
}
Vector C_BaseCombatWeapon::CalculateSpread(int seed, float inaccuracy, float spread, bool revolver2 = false) {
	CCSWeaponInfo* wep_info;
	auto        item_def_index = g_LocalPlayer->m_hActiveWeapon();
	float      recoil_index, r1, r2, r3, r4, s1, c1, s2, c2;

	// if we have no bullets, we have no spread.
	wep_info = g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData();
	if (!wep_info || !wep_info->iBullets)
		return { };

	// get some data for later.

	// generate needed floats.

	RandomSeed((seed & 0xff) + 1);

	// revolver secondary spread.
	if (item_def_index->GetCSWeaponData()->bIsRevolver) {
		r1 = 1.f - (r1 * r1);
		r3 = 1.f - (r3 * r3);
	}

	// negev spread.
	else if (item_def_index->GetCSWeaponData()->iWeaponType == WEAPONTYPE_MACHINEGUN && recoil_index < 3.f) {
		for (int i{ 3 }; i > recoil_index; --i) {
			r1 *= r1;
			r3 *= r3;
		}

		r1 = 1.f - r1;
		r3 = 1.f - r3;
	}

	// get needed sine / cosine values.
	c1 = std::cos(r2);
	c2 = std::cos(r4);
	s1 = std::sin(r2);
	s2 = std::sin(r4);

	// calculate spread vector.
	return {
		(c1 * (r1 * inaccuracy)) + (c2 * (r3 * spread)),
		(s1 * (r1 * inaccuracy)) + (s2 * (r3 * spread)),
		0.f
	};
}
float C_BaseCombatWeapon::GetSpread()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 452 + VALVE_ADDED_FUNCS)(this);
}
float C_BaseCombatWeapon::Hitchance()
{
	float hitchance = 101.f;
	if (!this) return 0.f;

	float inaccuracy = GetInaccuracy();
	if (inaccuracy == 0) inaccuracy = 0.0000001;
	inaccuracy = 1 / inaccuracy;
	hitchance = inaccuracy;
	return hitchance;
}
void C_BaseCombatWeapon::UpdateAccuracyPenalty()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 483 + VALVE_ADDED_FUNCS)(this);
}

CUtlVector<IRefCounted*>& C_BaseCombatWeapon::m_CustomMaterials()
{	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "83 BE ? ? ? ? ? 7F 67") + 2) - 12;
	return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + inReload);
}

bool* C_BaseCombatWeapon::m_bCustomMaterialInitialized()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "C6 86 ? ? ? ? ? FF 50 04") + 2);
	return (bool*)((uintptr_t)this + currentCommand);
}

CUserCmd*& C_BasePlayer::m_pCurrentCommand()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "89 BE ? ? ? ? E8 ? ? ? ? 85 FF") + 2);
	return *(CUserCmd**)((uintptr_t)this + currentCommand);
}

int C_BasePlayer::GetNumAnimOverlays() {
	if (this == nullptr)
		return 13;
	return *(int*)((DWORD)this + 0x298C);
}
bool C_BasePlayer::IsTeam()
{
	return !this->InDangerzone() && this != g_LocalPlayer && this->m_iTeamNum() == g_LocalPlayer->m_iTeamNum();
}


AnimationLayer *C_BasePlayer::GetAnimOverlays()
{
	return *(AnimationLayer**)((DWORD)this + 0x2980);
}
AnimationLayer *C_BasePlayer::GetAnimOverlay(int i)
{
	if (i < 15)
		return &GetAnimOverlays()[i];
	return nullptr;
}

int C_BasePlayer::GetSequenceActivity(int sequence)
{
	auto hdr = g_MdlInfo->GetStudioModel(this->GetModel());

	if (!hdr)
		return -1;

	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Utils::PatternScan(GetModuleHandle(L"client.dll"), "55 8B EC 53 8B 5D 08 56 8B F1 83"));

	return get_sequence_activity(this, hdr, sequence);
}

CCSGOPlayerAnimState *C_BasePlayer::GetPlayerAnimState()
{
	if (!this || !this->IsAlive())
		return nullptr;
	return *reinterpret_cast<CCSGOPlayerAnimState**>(uintptr_t(this) + 0x3914);
}

bool C_BasePlayer::IsLocalPlayer() const
{
	return EntIndex() == g_EngineClient->GetLocalPlayer();
}
void C_BasePlayer::UpdateAnimationState(CCSGOPlayerAnimState *state, QAngle angle)
{
	
	using fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
	static auto ret = reinterpret_cast<fn>(Utils::PatternScan(GetModuleHandleA(solution::Get().Module),"55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24"));

	if (!ret || !state)
		return;

	ret(state, nullptr, 0.f, angle.yaw, angle.pitch, nullptr);
}




void C_BasePlayer::ResetAnimationState(CCSGOPlayerAnimState *state)
{
	using ResetAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandleA(Xor(solution::Get().Module)), Xor("56 6A 01 68 ? ? ? ? 8B F1"));
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}

void C_BasePlayer::CreateAnimationState(CCSGOPlayerAnimState *state)
{
	using CreateAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*, C_BasePlayer*);
	static auto CreateAnimState = (CreateAnimState_t)Utils::PatternScan(GetModuleHandleA(solution::Get().Module), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, this);
}

std::string C_BasePlayer::GetName() {
	return GetPlayerInfo().szName;
}

Vector C_BasePlayer::GetEyePos()
{
	return m_vecOrigin() + m_vecViewOffset();
}
void C_BasePlayer::SetAnimLayers(std::array<AnimationLayer, 15>& layers)
{
	
}
player_info_t C_BasePlayer::GetPlayerInfo()
{
	player_info_t info;
	g_EngineClient->GetPlayerInfo(EntIndex(), &info);
	return info;
}
bool C_BasePlayer::IsVisible(bool smokecheck) {
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = g_LocalPlayer;

	auto endpos = this->GetHitboxPos(8);

	ray.Init(g_LocalPlayer->GetEyePos(), endpos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	auto result = tr.hit_entity == this || tr.fraction > 0.97f;
	if (result && smokecheck && Utils::LineGoesThroughSmoke(g_LocalPlayer->GetEyePos(), endpos))
		return false;

	return result;
}

bool C_BasePlayer::IsAlive()
{
	return m_lifeState() == LIFE_ALIVE || this->m_iHealth() > 0;
}
bool C_BasePlayer::InDangerzone()
{
	static auto game_type = g_CVar->FindVar("game_type");
	return game_type->GetInt() == 6;
}

bool C_BasePlayer::IsEnemy()
{
	if (InDangerzone())
	{
		return this->m_nSurvivalTeam() != g_LocalPlayer->m_nSurvivalTeam() || g_LocalPlayer->m_nSurvivalTeam() == -1;
	}
	else
	{
		return this->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
	}
}
bool C_BasePlayer::IsFlashed()
{
	static auto m_flFlashMaxAlpha = NetvarSys::Get().GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");
	return *(float*)((uintptr_t)this + m_flFlashMaxAlpha - 0x8) > 200.0;
}

bool C_BasePlayer::HasC4()
{
	static auto fnHasC4
		= reinterpret_cast<bool(__thiscall*)(void*)>(
			Utils::PatternScan(GetModuleHandleW(L"client.dll"), "56 8B F1 85 F6 74 31")
			);

	return fnHasC4(this);
}

Vector C_BasePlayer::GetHitboxPos(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_MdlInfo->GetStudioModel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				auto
					min = Vector{},
					max = Vector{};

				Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
				Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

				return (min + max) / 2.0f;
			}
		}
	}
	return Vector{};
}

mstudiobbox_t* C_BasePlayer::GetHitbox(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_MdlInfo->GetStudioModel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				return hitbox;
			}
		}
	}
	return nullptr;
}


Vector C_BasePlayer::GetHitboxPosition(int hitbox_id, matrix3x4_t boneMatrix[MAXSTUDIOBONES])
{
	auto studio_model = g_MdlInfo->GetStudioModel(GetModel());
	if (studio_model) {
		auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
		if (hitbox) {
			auto res = Vector{};

			Math::VectorTransform((hitbox->bbmin + hitbox->bbmax) * .5f, boneMatrix[hitbox->bone], res);
			return res;
		}
	}
	return Vector{};
}

bool C_BasePlayer::GetHitboxPos(int hitbox, Vector &output)
{
	if (hitbox >= HITBOX_MAX)
		return false;

	const model_t *model = this->GetModel();
	if (!model)
		return false;

	studiohdr_t *studioHdr = g_MdlInfo->GetStudioModel(model);
	if (!studioHdr)
		return false;

	matrix3x4_t matrix[MAXSTUDIOBONES];
	if (!this->SetupBones(matrix, MAXSTUDIOBONES, 0x100, 0))
		return false;

	mstudiobbox_t *studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(hitbox);
	if (!studioBox)
		return false;

	Vector min, max;

	Math::VectorTransform(studioBox->bbmin, matrix[studioBox->bone], min);
	Math::VectorTransform(studioBox->bbmax, matrix[studioBox->bone], max);

	output = (min + max) * 0.5f;

	return true;
}
bool C_BasePlayer::PointVisible(Vector endpos) {
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(GetEyePos(), endpos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.fraction > 0.97f;
}

Vector C_BasePlayer::HitboxPosition(int hitbox_id, matrix3x4_t matrix[128]) {
	studiohdr_t * hdr = g_MdlInfo->GetStudioModel(this->GetModel());

	if (hdr) {
		mstudiobbox_t *hitbox = hdr->GetHitboxSet(this->m_nHitboxSet())->GetHitbox(hitbox_id);

		if (hitbox) {
			Vector
				min = Vector{ },
				max = Vector{ };

			Math::VectorTransform(hitbox->bbmin, matrix[hitbox->bone], min);
			Math::VectorTransform(hitbox->bbmax, matrix[hitbox->bone], max);

			return (min + max) / 2.0f;
		}
	}

	return Vector{ };
}

Vector C_BasePlayer::GetBonePos(int bone)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f)) {
		return boneMatrix[bone].at(3);
	}
	return Vector{};
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, int hitbox)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	auto endpos = player->GetHitboxPos(hitbox);

	ray.Init(GetEyePos(), endpos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, const Vector& pos)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(GetEyePos(), pos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}
bool C_BasePlayer::CanSeePoint(Vector endpos)
{
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;
	ray.Init(GetEyePos(), endpos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);
	return tr.fraction > 0.97f;
}
void C_BasePlayer::WeaponShootPos(Vector& pos)
{
	
	return CallVFunction<void(__thiscall*)(void*, Vector&)>(this, 284)(this, pos);
}

//Vector C_BasePlayer::CalculateSpread(int seed, float inaccuracy, float spread, bool revolver2 = false) {
//	CCSWeaponInfo *wep_info;
//	auto weapon     = m_hActiveWeapon();
//	int            item_def_index;
//	float          recoil_index, r1, r2, r3, r4, s1, c1, s2, c2;
//
//	// if we have no bullets, we have no spread.
//	wep_info = weapon->GetCSWeaponData();
//	if (!wep_info || !wep_info->iBullets)
//		return {};
//
//	// get some data for later.
//	item_def_index = weapon->m_Item().m_iItemDefinitionIndex();
//	recoil_index   = m_flRecoilIndex();
//
//	// seed randomseed.
//	RandomSeed((seed & 0xff) + 1);
//
//	// generate needed floats.
//	r1 = Math::RandomFloat(0.f, 1.f);
//	r2 = Math::RandomFloat(0.f, M_PI * 2);
//
//
//	if ( g_csgo.weapon_accuracy_shotgun_spread_patterns->GetInt() > 0)
//		g_csgo.GetShotgunSpread(item_def_index, 0, 0 /*bullet_i*/ + wep_info->iBullets * recoil_index, &r4, &r3);
//
//	else {
//		r3 = Math::RandomFloat(0.f, 1.f);
//		r4 = Math::RandomFloat(0.f, M_PI * 2);
//	}
//
//	// revolver secondary spread.
//	if (item_def_index == WEAPON_REVOLVER && revolver2) {
//		r1 = 1.f - (r1 * r1);
//		r3 = 1.f - (r3 * r3);
//	}
//
//	// negev spread.
//	else if (item_def_index == WEAPON_NEGEV && recoil_index < 3.f) {
//		for (int i{ 3 }; i > recoil_index; --i) {
//			r1 *= r1;
//			r3 *= r3;
//		}
//
//		r1 = 1.f - r1;
//		r3 = 1.f - r3;
//	}
//
//	// get needed sine / cosine values.
//	c1 = std::cos(r2);
//	c2 = std::cos(r4);
//	s1 = std::sin(r2);
//	s2 = std::sin(r4);
//
//	// calculate spread vector.
//	return {
//		(c1 * (r1 * inaccuracy)) + (c2 * (r3 * spread)),
//		(s1 * (r1 * inaccuracy)) + (s2 * (r3 * spread)),
//		0.f
//	};
//}



Vector C_BasePlayer::GetShootPos()
{
	Vector pos;
	WeaponShootPos(pos);
	pos.z -= m_vecViewOffset().z - floor(m_vecViewOffset().z);
	return pos;
}
void C_BasePlayer::UpdateClientSideAnimation()
{
	return CallVFunction<void(__thiscall*)(void*)>(this, 223)(this);
}
bool C_BasePlayer::IsNotTarget() {
	if (!this || this == g_LocalPlayer)
		return true;

	if (m_iHealth() <= 0)
		return true;

	if (m_bGunGameImmunity())
		return true;

	if (m_fFlags() & FL_FROZEN)
		return true;

	int entIndex = EntIndex();
	return entIndex > g_GlobalVars->maxClients;
}

void C_BasePlayer::InvalidateBoneCache()
{
	static DWORD addr = (DWORD)Utils::PatternScan(GetModuleHandleA(solution::Get().Module), "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");

	*(int*)((uintptr_t)this + 0xA30) = g_GlobalVars->framecount; //we'll skip occlusion checks now
	*(int*)((uintptr_t)this + 0xA28) = 0;//clear occlusion flags

	unsigned long g_iModelBoneCounter = **(unsigned long**)(addr + 10);
	*(unsigned int*)((DWORD)this + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}


void C_BasePlayer::SetSnakewareAngles(QAngle angles)
{
	// nixer paster esli 4o ))00)0
	static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
	*(QAngle*)((DWORD)this + deadflag + 0x4) = angles;
}


void C_BasePlayer::SetAbsAngles(const QAngle &angles)
{
	using SetAbsAnglesFn = void(__thiscall*)(void*, const QAngle &angles);
	static SetAbsAnglesFn SetAbsAngles = (SetAbsAnglesFn)Utils::PatternScan(GetModuleHandleA(solution::Get().Module), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8");
	// by @justice1337                                                                                      
	SetAbsAngles(this, angles);
}

const Vector C_BasePlayer::WorldSpaceCenter()
{
	Vector vecOrigin = m_vecOrigin();

	Vector min = this->GetCollideable()->OBBMins() + vecOrigin;
	Vector max = this->GetCollideable()->OBBMaxs() + vecOrigin;

	Vector size = max - min;
	size /= 2.f;
	size += min;

	return size;
}

void C_BasePlayer::SetAbsOrigin(const Vector &origin)
{
	using SetAbsOriginFn = void(__thiscall*) (void*, const Vector& origin);
	static SetAbsOriginFn SetAbsOrigin;

	if (!SetAbsOrigin)
		SetAbsOrigin = (SetAbsOriginFn)((DWORD)Utils::PatternScan(GetModuleHandleA(solution::Get().Module),("55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8")));

	SetAbsOrigin(this, origin);
}



void C_BasePlayer::InvalidatePhysics(int32_t flags)
{
	static const auto invalidate_physics_recursive = reinterpret_cast<void(__thiscall*)(C_BasePlayer*, int32_t)>(
		Utils::PatternScan(GetModuleHandleA(solution::Get().Module), "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56"));
	invalidate_physics_recursive(this, flags);
}

QAngle &C_BasePlayer::VisualAngles()
{
	return *(QAngle*)((uintptr_t)this + NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag") + 4);
}
int C_BasePlayer::m_nMoveType()
{
	return *(int*)((uintptr_t)this + 0x25C);
}

QAngle* C_BasePlayer::GetVAngles()
{
	static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
	return (QAngle*)((uintptr_t)this + deadflag + 0x4);
}

void C_BaseAttributableItem::SetGloveModelIndex(int modelIndex)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}

void C_BaseViewModel::SendViewModelMatchingSequence(int sequence)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 246 + VALVE_ADDED_FUNCS)(this, sequence);
}

float_t C_BasePlayer::m_flSpawnTime()
{
	return *(float_t*)((uintptr_t)this + 0xA370); // updated
}
