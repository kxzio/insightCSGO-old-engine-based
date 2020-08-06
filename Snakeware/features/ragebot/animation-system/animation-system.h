#pragma once
#include "../../../valve_sdk/csgostructs.hpp"
#include <unordered_map>
#include <deque>
#include <optional>
#include <functional>
struct Animation
{
	Animation() = default;
	explicit Animation(C_BasePlayer* player);
	explicit Animation(C_BasePlayer* player, QAngle last_reliable_angle);
	void Restore(C_BasePlayer* player) const;
	void Apply(C_BasePlayer* player) const;
	void BulidServerBones(C_BasePlayer* player);

	bool is_valid(float range, float max_unlag);
	bool is_valid_extended();

	C_BasePlayer* player{};
	int32_t index{};

	bool valid{}, has_anim_state{};
	alignas(16) matrix3x4_t bones[128];

	bool dormant{};
	Vector velocity;
	Vector origin;
	matrix3x4_t* bone_cache;
	Vector abs_origin;
	Vector obb_mins;
	Vector obb_maxs;
	AnimationLayer layers[13];
	std::array<float, 24> poses;
	CCSGOPlayerAnimState* anim_state;
	float anim_time{};
	float sim_time{};
	float interp_time{};
	float duck{};
	float lby{};
	float last_shot_time{};
	QAngle last_reliable_angle{};
	QAngle eye_angles;
	QAngle abs_ang;
	int flags{};
	int eflags{};
	int effects{};
	float m_flFeetCycle{};
	float m_flFeetYawRate{};
	int lag{};
	bool didshot;
	std::string resolver;
};


class Animations : public Singleton<Animations>
{
public:
	struct AnimationInfo {
		AnimationInfo(C_BasePlayer* player, std::deque<Animation*> animations)
			: player(player), frames(std::move(animations)), last_spawn_time(0) { }

		void UpdateAnimations(Animation* to, Animation* from);

		C_BasePlayer* player{};
		std::deque<Animation*> frames;

		// last time this player spawned
		float last_spawn_time;
		float goal_feet_yaw;
		QAngle last_reliable_angle;
	};

	std::unordered_map<int, AnimationInfo> animation_infos;
public:




	void UpdatePlayerAnimations();

	void UpdatePlayer(C_BasePlayer* player);

	Animations::AnimationInfo* GetAnimInfo(C_BasePlayer* player);

	std::optional<Animation*> get_latest_animation(C_BasePlayer* player);





	std::optional<Animation*> get_oldest_animation(C_BasePlayer* player);

	std::optional<Animation*> get_latest_firing_animation(C_BasePlayer* player);

	void FakeAnimation();

	float m_ServerAbsRotation = 0.f;


	void FixLocalPlayer();

	void SetLocalPlayerAnimations();




	// fake  matrix & state
	CCSGOPlayerAnimState* RealAnimstate = nullptr;
	CCSGOPlayerAnimState* FakeAnimstate = nullptr;




	QAngle m_current_real_angle = QAngle(0.f, 0.f, 0.f);
	QAngle m_current_fake_angle = QAngle(0.f, 0.f, 0.f);


private:

};