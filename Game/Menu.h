#ifndef MENU_H
#define MENU_H


#define STATE_MENU 0

//-----------------------------------------------------------------------------
// Menu Class
//-----------------------------------------------------------------------------
class Menu : public State
{
public:
	Menu();

	virtual void Update( float elapsed );
};

#endif