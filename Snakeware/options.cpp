#include "options.hpp"

int realAimSpot[] = { 8, 7, 6, 0 };
int realHitboxSpot[] = { 0, 1, 2, 3 };
namespace Snakeware
{
	int MissedShots[65];
	QAngle LocalAngle = QAngle(0, 0, 0);
	QAngle FakeAngle = QAngle(0, 0, 0);
	QAngle RealAngle = QAngle(0, 0, 0);
	matrix3x4_t FakeMatrix[128];
	matrix3x4_t realmatrix[128];
	matrix3x4_t FakeLagMatrix[128];
	Vector Angle;
	QAngle Aimangle = QAngle(0, 0, 0);
	uint32_t ShotCmd;
	int UnpredTick;
	float g_flVelocityModifer;
	bool bSendPacket ;
	bool OnShot;
	bool g_bOverrideVelMod;
	bool LBY_Update = false;
	bool bAimbotting = false;
	int m_nTickbaseShift;
	int m_nBaseTick;
	
	bool bVisualAimbotting = false;
	QAngle vecVisualAimbotAngs = QAngle(0.f, 0.f, 0.f);
	

}


CUserCmd * Cmd::GetCommand()
{

		return cmdshka;

}

Cmd g_Cmd;