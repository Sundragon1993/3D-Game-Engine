#ifndef MAIN_H
#define MAIN_H

#include <windows.h>

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#include "..\Engine\Engine.h"
//#pragma comment( lib, "./Engine/Day_Chaos.lib" )

//-----------------------------------------------------------------------------
// Game Includes
//-----------------------------------------------------------------------------
#include "GameMaterial.h"
#include "Bullet.h"
#include "Weapon.h"
#include "PlayerObject.h"
#include "PlayerManager.h"
#include "Menu.h"
#include "Game.h"

//-----------------------------------------------------------------------------
// Player Data Structure
//-----------------------------------------------------------------------------
struct PlayerData
{
	char character[MAX_PATH];
	char map[MAX_PATH];
};

#endif