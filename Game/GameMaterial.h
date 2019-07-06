#ifndef GAME_MATERIAL_H
#define GAME_MATERIAL_H

//-----------------------------------------------------------------------------
// Game Material Class
//-----------------------------------------------------------------------------
class GameMaterial : public Material
{
public:
	GameMaterial( char *name, char *path = "./" );
	virtual ~GameMaterial();

	Sound *GetStepSound();

private:
	LinkedList< Sound > *m_stepSounds;
};

#endif