#include "Main.h"

GameMaterial::GameMaterial( char *name, char *path ) : Material( name, path )
{
	// Load the script for this material.
	Script *script = new Script( name, path );

	// Store the step sounds.
	m_stepSounds = new LinkedList< Sound >;
	char stepSound[16] = { "step_sound0" };
	while( script->GetStringData( stepSound ) != NULL )
	{
		m_stepSounds->Add( new Sound( script->GetStringData( stepSound ) ) );
		sprintf( stepSound, "step_sound%d", m_stepSounds->GetTotalElements() );
	}

	// Destory the material's script.
	SAFE_DELETE( script );
}


GameMaterial::~GameMaterial()
{
	// Destroy the step sounds list.
	SAFE_DELETE( m_stepSounds );
}


Sound *GameMaterial::GetStepSound()
{
	return m_stepSounds->GetRandom();
}