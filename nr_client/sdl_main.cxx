// docm_prefix(///)
/****************************************************************************
*  Copyright (C) 2002 by Leo Khramov
*  email:   leo@xnc.dubna.su
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
 ****************************************************************************/
// $Id: sdl_main.cxx,v 1.1 2009/09/08 19:07:40 leo Exp $

/// module description
/// Main module - init the game, create main objects and entering in event loop
#include <iostream>

#include "exceptions.hxx"
#include "game_fill.hxx"
#include "game_profile.h"

Game  *game=0;

char *fname=0;

Game* init_game(int argc, char** argv)
{
    if(argc>1)
    {
	fname=argv[1];
	return new GameFill;
    }
    return 0;
}

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
//===========================================================================
/// global main(int, char**)
///     main function - start of the game
/// tags main
int main(int argc, char** argv)
{
    std::vector<ids_type> vec;

    try 
    {
	CProfileManager::Reset();  
	game=init_game(argc, argv);
	
	game->show();
	game->event_loop();
	
	delete game;
	
	CProfileManager::Show_All_Profile();
    }
    catch(ExMain& ex)
    {
	std::cerr << "Got global exception: " << ex.get_info() << std::endl << "Exiting." << std::endl;
	if(game)
	    delete game;
	return 1;
    }

    return 0;
}
