#include "animation-system.h"
#include "../../../options.hpp"
#include "../resolver/resolver.h"
#include "../player-log/player-log.h"


float CalculateLerp() {
	static auto cl_interp = g_CVar->FindVar("cl_interp");
	static auto cl_updaterate = g_CVar->FindVar("cl_updaterate");
	const auto update_rate = cl_updaterate->GetInt();
	const auto interp_ratio = cl_interp->GetFloat();

	auto lerp = interp_ratio / update_rate;

	if (lerp <= interp_ratio)
		lerp = interp_ratio;

	return lerp;
}

bool Animation::is_valid(float range = .2f, float max_unlag = .2f)
{
	if (!g_EngineClient->GetNetChannelInfo() || !valid) return false;

	const auto correct = std::clamp(g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_INCOMING)
		+ g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING)
		+ CalculateLerp(), 0.f, max_unlag);
	//Fixed..
	return fabsf(correct - (g_GlobalVars->curtime - sim_time)) <= range;
}

Animation::Animation(C_BasePlayer* player) {
	const auto weapon = player->m_hActiveWeapon();

	this->player = player;
	index = player->EntIndex();
	dormant = player->IsDormant();
	velocity = player->m_vecVelocity();
	origin = player->m_vecOrigin();
	abs_origin = player->m_angAbsOrigin();
	obb_mins = player->m_vecMins();
	obb_maxs = player->m_vecMaxs();
	memcpy(layers, player->GetAnimOverlays(), sizeof(AnimationLayer) * 13);
	poses = player->m_flPoseParameter();
	//if ((has_anim_state = player->GetPlayerAnimState()))
	anim_state = player->GetPlayerAnimState();
	//anim_time = player->GetOldSimulationTime() + interfaces.global_vars->interval_per_tick;
	sim_time = player->m_flSimulationTime();
	interp_time = 0.f;
	last_shot_time = weapon ? weapon->m_fLastShotTime() : 0.f;
	duck = player->m_flDuckAmount();
	lby = player->m_flLowerBodyYawTarget();
	eye_angles = player->m_angEyeAngles();
	abs_ang = player->m_angAbsAngles();
	flags = player->m_fFlags();
	eflags = player->m_iEFlags();
	effects = player->GetEffect();

	lag = TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime());

	// animations are off when we enter pvs, we do not want to shoot yet.
	valid = lag >= 0 && lag <= 17;

	// clamp it so we don't interpolate too far : )
	lag = std::clamp(lag, 0, 17);
}


Animation::Animation(C_BasePlayer* player, QAngle last_reliable_angle) : Animation(player) {
	this->last_reliable_angle = last_reliable_angle;
}



void Animation::Restore(C_BasePlayer* player) const {
	player->m_vecVelocity() = velocity;
	player->m_fFlags() = flags;
	player->m_flDuckAmount() = duck;
	memcpy(player->GetAnimOverlays(), layers, sizeof(AnimationLayer) * 13);
	player->m_flLowerBodyYawTarget() = lby;
	player->m_vecOrigin() = origin;
	player->SetAbsOrigin(abs_origin);
	player->m_flPoseParameter() = poses;
	player->m_vecMins() = obb_mins;
	player->m_vecMaxs() = obb_maxs;
}

void Animation::Apply(C_BasePlayer* player) const {
	player->m_flPoseParameter() = poses;
	player->m_angEyeAngles() = eye_angles;
	player->m_vecVelocity() = velocity;
	player->m_flLowerBodyYawTarget() = lby;
	player->m_flDuckAmount() = duck;
	player->m_fFlags() = flags;
	player->m_vecOrigin() = origin;
	player->m_vecMins() = obb_mins;
	player->m_vecMaxs() = obb_maxs;
	player->SetAbsOrigin(origin);
	/*if (anim_state) {
		player->SetAnimState(anim_state);
	}*/
}

void Animation::BulidServerBones(C_BasePlayer* player) {

	const auto backup_occlusion_flags      = player->GetOcclusionFlags();
	const auto backup_occlusion_framecount = player->GetOcclusionFramecount();
	auto jiggle_bones = g_CVar->FindVar("r_jiggle_bones");
	auto old_jiggle_bones_value = jiggle_bones->GetInt();
	player->GetOcclusionFlags()      = 0;
	player->GetOcclusionFramecount() = 0;
	player->GetReadableBones() = player->GetWritableBones() = 0;
	player->GetLastBoneSetupTime() = -FLT_MAX;
	
	
	jiggle_bones->SetValue(0);
		

	player->GetEffect () |= 0x8;

	const auto backup_bone_array   = player->GetBoneArrayForWrite();

	player->GetBoneArrayForWrite() = bones;

	player->SetupBones (nullptr, -1, 0x7FF00, g_GlobalVars->curtime);

	player->GetBoneArrayForWrite()   = backup_bone_array;
	player->GetOcclusionFlags()      = backup_occlusion_flags;
	player->GetOcclusionFramecount() = backup_occlusion_framecount;
	jiggle_bones->SetValue(old_jiggle_bones_value);

	player->GetEffect () &= ~0x8;
}


void Animations::AnimationInfo::UpdateAnimations(Animation* record, Animation* from) {
	


	if (!from) {
		// set velocity and layers.
		  record->velocity = player->m_vecVelocity();

		// fix feet spin.
		  record->anim_state->m_flFeetYawRate = 0.f;


		// apply record.
		  record->Apply(player);

		// run update.
		   Animations::Get().UpdatePlayer(player);

		   return;
	}

	

	// restore old record.

	memcpy(player->GetAnimOverlays(), from->layers, sizeof(AnimationLayer) * 13);
	player->SetAbsOrigin(record->origin);
	player->SetAbsAngles(from->abs_ang);
	


	// setup extrapolation parameters.
	auto old_origin = from->origin;
	auto old_flags = from->flags;

	const auto Shot = record->is_backtrackable = record->last_shot_time > from->sim_time&& record->last_shot_time <= record->sim_time;

	for (auto i = 0; i < record->lag; i++) {

		// move time forward.
		const auto time = from->sim_time + TICKS_TO_TIME(i + 1);
		const auto lerp = 1.f * (((float)i + 1.f) / (float)record->lag);

		if (record->lag > 1)	{
			// lerp eye angles.
			auto eye_angles = Math::Interpolate(from->eye_angles, record->eye_angles, lerp);
			Math::NormalizeAng(eye_angles);
			player->m_angEyeAngles() = eye_angles;

			// lerp duck amount.
			player->m_flDuckAmount() = Math::Interpolate(from->duck, record->duck, lerp);

			// lerp velocity.
			player->m_vecVelocity() = player->m_vecAbsVelocity() = Math::Interpolate(from->velocity, record->velocity, lerp);
			
		}

		if (record->lag - 1 == i)
		{
			player->m_vecVelocity() = Math::Interpolate(from->velocity, record->velocity, 0.5f);
			player->m_fFlags() = record->flags;
		
			
		}
		
		if (Shot)
		{
			player->m_angEyeAngles() = record->last_reliable_angle;

			if (record->last_shot_time <= time)
				player->m_angEyeAngles() = record->eye_angles;
		}

		player->GetPlayerAnimState()->m_flFeetYawRate = 0.f;

		// backup simtime.
		const auto backup_simtime = player->m_flSimulationTime();

		// set new simtime.
		player->m_flSimulationTime() = time;

		// run update.
		Animations::Get().UpdatePlayer(player);

		// restore old simtime.
		player->m_flSimulationTime() = backup_simtime;
	}
	
}

void Animations::UpdatePlayerAnimations() {

	if (!g_EngineClient->IsInGame()) return;
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;

	// why?


   // erase outdated entries
	for (auto it = animation_infos.begin(); it != animation_infos.end();) {
		auto player = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(it->first));

		if (!player || player != it->second.player || !player->IsAlive() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
		{
			if (player)
				player->m_bClientSideAnimation() = true;
			it = animation_infos.erase(it);
		}
		else
			it = next(it);
	}

	if (!g_LocalPlayer) {
		g_EntityList->ForEachPlayer([](C_BasePlayer* player) -> void {
			player->m_bClientSideAnimation() = true;
		});
		return;
	}

	for (auto i = 1; i <= g_EngineClient->GetMaxClients(); ++i) {
		const auto entity = C_BasePlayer::GetPlayerByIndex(i);
		if (!entity || !entity->IsPlayer())            continue;

		if (!entity->IsAlive() || entity->IsDormant()) continue;

		if (entity == g_LocalPlayer)                   continue;

		if (entity != g_LocalPlayer && entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) {
			entity->m_bClientSideAnimation() = true;
			continue;
		}

		if (animation_infos.find(entity->GetRefEHandle().ToInt()) == animation_infos.end())
			animation_infos.insert_or_assign(entity->GetRefEHandle().ToInt(), AnimationInfo(entity, {}));
	}

	// run post update
	for (auto& info : animation_infos) {
		auto& _animation = info.second;
		const auto player = _animation.player;

		// erase frames out-of-range
		
		for (auto i = _animation.frames.rbegin(); i != _animation.frames.rend();) {
			if (g_GlobalVars->curtime - i->sim_time > 1.2f) 
				i = decltype(i) { _animation.frames.erase(next(i).base()) };
			else
				i = next(i); 
		
		}
	
		
    	
		Resolver::Get().ResolvePitch(_animation.player);
		//Resolver::Get().Resolve(_animation.player);

		// have we already seen this update?
		if (player->m_flSimulationTime() == player->m_flOldSimulationTime())  continue;
		const auto state                 = player->GetPlayerAnimState();
		// reset animstate
		if (_animation.last_spawn_time != player->m_flSpawnTime()) {
			if (state) 	{
				player->ResetAnimationState(state);
				state->m_flLastClientSideAnimationUpdateTime = g_GlobalVars->curtime - g_GlobalVars->interval_per_tick;
				
			} 
			_animation.last_spawn_time = player->m_flSpawnTime();
			continue;
		}

		// grab weapon
		const auto weapon = player->m_hActiveWeapon();

		// make a full backup of the player
		auto backup = Animation(player);
		backup.Apply(player);
	
		// grab previous
		Animation* previous = nullptr;

		if (!_animation.frames.empty() && ! _animation.frames.front().dormant && TIME_TO_TICKS(player->m_flSimulationTime() - _animation.frames.front().sim_time) <= 17)
			previous = &_animation.frames.front();
		    
		
		const auto shot = weapon && previous && weapon->m_fLastShotTime () != previous->last_shot_time;

		if (!shot)
			info.second.last_reliable_angle = player->m_angEyeAngles();

		// store server record
		
		auto& record = _animation.frames.emplace_front(player, info.second.last_reliable_angle);
		
		// run full update
		_animation.UpdateAnimations(&record, previous);


		//restore server layers
		memcpy(player->GetAnimOverlays(), backup.layers, sizeof(AnimationLayer) * 13);


		//UpdatePlayerIndex(player, &record);

		// use uninterpolated data to generate our bone matrix
		record.BulidServerBones(player);

		// restore correctly synced values
		backup.Restore(player);

	}

}

void Animations::UpdatePlayerIndex(C_BasePlayer * pPlayer,Animation * pRecord) {

	const auto BackupPoses    = pPlayer->m_flPoseParameter();
	const auto BackupAngles   = pPlayer->m_angEyeAngles();
	const auto BackupVelocity = pPlayer->m_vecVelocity();
	const auto BackupOrigin   = pPlayer->m_vecOrigin();
	const auto BackupDuck     = pPlayer->m_flDuckAmount();
	const auto BackupSimTime  = pPlayer->m_flSimulationTime();
	const auto BackupFlags    = pPlayer->m_fFlags();


	Resolver::Get().SetResolveIndex(pPlayer, - 1);
	

	pPlayer->m_flPoseParameter()  = BackupPoses;
	pPlayer->m_vecVelocity()      = BackupVelocity;
	pPlayer->m_vecOrigin()        = BackupOrigin;
	pPlayer->m_flDuckAmount()     = BackupDuck;
	pPlayer->m_flSimulationTime() = BackupSimTime;
	pPlayer->m_angEyeAngles()     = BackupAngles;
	pPlayer->m_fFlags()           = BackupFlags;
	pPlayer->m_vecAbsVelocity()   = BackupVelocity;
	pPlayer->SetupBones            (nullptr, -1, 0x7FF00, g_GlobalVars->curtime);

	Resolver::Get().SetResolveIndex(pPlayer, 1);

	pPlayer->m_flPoseParameter() = BackupPoses;
	pPlayer->m_vecVelocity() = BackupVelocity;
	pPlayer->m_vecOrigin() = BackupOrigin;
	pPlayer->m_flDuckAmount() = BackupDuck;
	pPlayer->m_flSimulationTime() = BackupSimTime;
	pPlayer->m_angEyeAngles() = BackupAngles;
	pPlayer->m_fFlags() = BackupFlags;
	pPlayer->m_vecAbsVelocity() = BackupVelocity;

	pPlayer->SetupBones(nullptr, -1, 0x7FF00, g_GlobalVars->curtime);

	pPlayer->m_flPoseParameter() = BackupPoses;
	pPlayer->m_vecVelocity() = BackupVelocity;
	pPlayer->m_vecOrigin() = BackupOrigin;
	pPlayer->m_flDuckAmount() = BackupDuck;
	pPlayer->m_flSimulationTime() = BackupSimTime;
	pPlayer->m_angEyeAngles() = BackupAngles;
	pPlayer->m_fFlags() = BackupFlags;
	pPlayer->m_vecAbsVelocity() = BackupVelocity;

}


void Animations::UpdatePlayer(C_BasePlayer* player) {
	static auto& enable_bone_cache_invalidation = **reinterpret_cast<bool**>( reinterpret_cast<uint32_t>((void*)Utils::PatternScan(GetModuleHandleA("client.dll"), "C6 05 ? ? ? ? ? 89 47 70")) + 2);

	// Onetap code be like <3

	//// make a backup of globals
	
	const auto interpolation       =  g_GlobalVars->interpolation_amount;
	const auto backup_absframetime =  g_GlobalVars->absoluteframetime;
	const auto backup_frametime    =  g_GlobalVars->frametime;
	const auto backup_curtime      =  g_GlobalVars->curtime;
	const auto backup_realtime     =  g_GlobalVars->realtime;

	const auto backup_flags        =  player->m_fFlags();
	
	// get player anim state
	auto  aState = player->GetPlayerAnimState();
	

	// fixes for networked players
	g_GlobalVars->interpolation_amount = 0.f;
	g_GlobalVars->frametime            = g_GlobalVars->interval_per_tick;
	g_GlobalVars->absoluteframetime    = g_GlobalVars->interval_per_tick;
	g_GlobalVars->curtime              = player->m_flSimulationTime();
	g_GlobalVars->realtime             = player->m_flSimulationTime();
	

	player->m_iEFlags() &= ~0x1000;
	player->m_vecAbsVelocity()  = player->m_vecVelocity();

	const auto old_invalidation = enable_bone_cache_invalidation;

	// notify the other hooks to instruct animations and pvs fix


	if (aState->m_iLastClientSideAnimationUpdateFramecount == g_GlobalVars->framecount)
		aState->m_iLastClientSideAnimationUpdateFramecount =  g_GlobalVars->framecount - 1;

	player->m_bClientSideAnimation() = true;
	player->UpdateClientSideAnimation();
	if (!player->IsLocalPlayer()) player->m_bClientSideAnimation() = false;


	if (!player->IsLocalPlayer())
		player->InvalidatePhysics(C_BaseEntity::angles_changed| C_BaseEntity::animation_changed | C_BaseEntity::sequence_changed);

	// we don't want to enable cache invalidation by accident
	enable_bone_cache_invalidation = old_invalidation;

	// restore globals
	
	g_GlobalVars->interpolation_amount = interpolation;
	g_GlobalVars->curtime              = backup_curtime;
	g_GlobalVars->realtime             = backup_realtime;
	g_GlobalVars->frametime            = backup_frametime;
	g_GlobalVars->absoluteframetime    = backup_absframetime;

	player->m_fFlags() = backup_flags;
}

Animations::AnimationInfo* Animations::GetAnimInfo(C_BasePlayer* player) {
	auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end())
		return nullptr;

	return &info->second;
}


std::optional<Animation*> Animations::get_latest_animation(C_BasePlayer* player) {


	const auto pInfo = animation_infos.find(player->GetRefEHandle().ToInt());
	if (pInfo == animation_infos.end() || pInfo->second.frames.empty()) {
		return std::nullopt;
	}

	Animation* first_invalid = nullptr;

	for (auto it = pInfo->second.frames.begin(); it != pInfo->second.frames.end(); it = next(it)) {

		if (!first_invalid)
			first_invalid = &*it;

		if (it->is_valid(it->sim_time, it->valid)) {
			return &*it;
		}
	}

	if (first_invalid)
		return first_invalid;
	else
		return std::nullopt;
}


std::optional<Animation*> Animations::get_oldest_animation(C_BasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.rbegin(); it != info->second.frames.rend(); it = next(it)) {
		if (it->is_valid(it->sim_time, it->valid)) {
			return &*it;
		}
	}

	return std::nullopt;


}




std::optional<Animation*> Animations::get_latest_firing_animation(C_BasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it))
		if (it->is_valid() && it->didshot)
			return &*it;

	return std::nullopt;
}
void Animations::FakeAnimation()
{
	//static bool ShouldInitAnimstate = false;

	if ((!g_EngineClient->IsConnected() && !g_EngineClient->IsInGame()) || !g_LocalPlayer) {
		return;
	}
	if (!g_Options.chams_fake) return;

	if (!g_LocalPlayer->IsAlive()) {
		return;
	}

	static CBaseHandle* selfhandle = nullptr;
	static float spawntime = g_LocalPlayer->m_flSpawnTime();

	auto alloc = FakeAnimstate == nullptr;
	auto change = !alloc && selfhandle != &g_LocalPlayer->GetRefEHandle();
	auto reset = !alloc && !change && g_LocalPlayer->m_flSpawnTime() != spawntime;

	if (change) {
		memset(&FakeAnimstate, 0, sizeof(FakeAnimstate));
		selfhandle = (CBaseHandle*)&g_LocalPlayer->GetRefEHandle();
	}
	if (reset) {
		g_LocalPlayer->ResetAnimationState(FakeAnimstate);
		spawntime = g_LocalPlayer->m_flSpawnTime();
	}

	if (alloc || change) {
		FakeAnimstate = reinterpret_cast<CCSGOPlayerAnimState*>(g_MemAlloc->Alloc(sizeof(CCSGOPlayerAnimState)));
		if (FakeAnimstate)
			g_LocalPlayer->CreateAnimationState(FakeAnimstate);
	}

	if (FakeAnimstate->m_iLastClientSideAnimationUpdateFramecount == g_GlobalVars->framecount)
		FakeAnimstate->m_iLastClientSideAnimationUpdateFramecount -= 1.f;

	g_LocalPlayer->GetEffect() |= 0x8;

	g_LocalPlayer->InvalidateBoneCache();

	AnimationLayer backup_layers[13];
	if (g_LocalPlayer->m_flSimulationTime() != g_LocalPlayer->m_flOldSimulationTime())
	{
		bool pidoras = true;
		std::memcpy(backup_layers, g_LocalPlayer->GetAnimOverlays(),
			(sizeof(AnimationLayer) * g_LocalPlayer->GetNumAnimOverlays()));

		g_LocalPlayer->UpdateAnimationState(FakeAnimstate, Snakeware::FakeAngle); // update animstate
		g_LocalPlayer->SetAbsAngles(QAngle(0, FakeAnimstate->m_flGoalFeetYaw, 0));
		g_LocalPlayer->GetAnimOverlay(12)->m_flWeight = FLT_EPSILON;
		g_LocalPlayer->SetupBones(Snakeware::FakeMatrix, 128, 0x7FF00, g_GlobalVars->curtime);// setup matrix

		for (auto& i : Snakeware::FakeMatrix)
		{
			i[0][3] -= g_LocalPlayer->GetRenderOrigin().x;
			i[1][3] -= g_LocalPlayer->GetRenderOrigin().y;
			i[2][3] -= g_LocalPlayer->GetRenderOrigin().z;
		}


		std::memcpy(g_LocalPlayer->GetAnimOverlays(), backup_layers,
			(sizeof(AnimationLayer) * g_LocalPlayer->GetNumAnimOverlays()));
	}

	//csgo->animstate = FakeAnimstate; usseles

	g_LocalPlayer->GetEffect() &= ~0x8;
}


bool CanFix() {

	if (g_Options.antihit_enabled)        return true;
	if (g_Options.misc_legit_antihit)     return true;
	if (g_Options.misc_fakelag_ticks > 1) return true;

	return false;
}
void Animations::FixLocalPlayer() {

	auto animstate = g_LocalPlayer->GetPlayerAnimState();
	if (!animstate)
		return;
	if (!g_LocalPlayer->IsAlive()) return;
	if (!g_EngineClient->IsInGame()) return;
	if (!CanFix) return;

	const auto backup_frametime = g_GlobalVars->frametime;
	const auto backup_curtime = g_GlobalVars->curtime;

	animstate->m_flGoalFeetYaw = Snakeware::RealAngle.yaw;

	if (animstate->m_iLastClientSideAnimationUpdateFramecount == g_GlobalVars->framecount)
		animstate->m_iLastClientSideAnimationUpdateFramecount -= 1.f;

	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
	g_GlobalVars->curtime = g_LocalPlayer->m_flSimulationTime();

	g_LocalPlayer->m_iEFlags() &= ~0x1000;
	g_LocalPlayer->m_vecAbsVelocity() = g_LocalPlayer->m_vecVelocity();

	static float angle = animstate->m_flGoalFeetYaw;

	animstate->m_flFeetYawRate = 0.f;

	AnimationLayer backup_layers[13];
	if (g_LocalPlayer->m_flSimulationTime() != g_LocalPlayer->m_flOldSimulationTime())
	{
		std::memcpy(backup_layers, g_LocalPlayer->GetAnimOverlays(),
			(sizeof(AnimationLayer) * g_LocalPlayer->GetNumAnimOverlays()));

		g_LocalPlayer->m_bClientSideAnimation() = true;
		g_LocalPlayer->UpdateAnimationState(animstate, Snakeware::FakeAngle);
		g_LocalPlayer->UpdateClientSideAnimation();


		angle = animstate->m_flGoalFeetYaw;

		std::memcpy(g_LocalPlayer->GetAnimOverlays(), backup_layers,
			(sizeof(AnimationLayer) * g_LocalPlayer->GetNumAnimOverlays()));
	}

	animstate->m_flGoalFeetYaw = angle;
	g_GlobalVars->curtime = backup_curtime;
	g_GlobalVars->frametime = backup_frametime;

}
void Animations::SetLocalPlayerAnimations()
{
	auto animstate = g_LocalPlayer->GetPlayerAnimState();
	if (!animstate) return;
	// weawe.su reverse
	if (g_EngineClient->IsInGame())
	{
		if (g_Input->m_fCameraInThirdPerson && g_Options.misc_thirdperson)
			g_LocalPlayer->SetSnakewareAngles(Snakeware::FakeAngle);

		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
			animstate->m_bOnGround = true;
			animstate->m_bInHitGroundAnimation = false;
		}

		g_LocalPlayer->SetAbsAngles(QAngle(0, g_LocalPlayer->GetPlayerAnimState()->m_flGoalFeetYaw, 0));
	}

}


