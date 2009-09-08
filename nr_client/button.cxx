#include "game.hxx"
#include "button.hxx"

Button::Button(Game* _game, const char* _name) : GameObject(_game, _name)
{
    char subname[128];

    geo=get_geo_by_name(name);

    sprintf(subname, "%s_norm", name);
    sprite[stNormal]=new Sprite(game, subname);

    sprintf(subname, "%s_pres", name);
    sprite[stPressed]=new Sprite(game, subname);

    sprintf(subname, "%s_hili", name);
    sprite[stHighlight]=new Sprite(game, subname);

    state=stNormal;
}

Button::~Button()
{
    for(int i=0;i<but_max_states;i++)
	delete sprite[i];
}

bool Button::show()
{
    if(shown)
	return true;
    shown=true;
    sprite[state]->show();
    game->add_object(this);
    return false;
}

bool Button::hide()
{
    if(!shown)
	return true;
    shown=false;
    game->del_object(this);
    sprite[state]->hide();
}

bool  Button::mouse_press_event(MouseEvent* mev)
{
    if(mev->mx < geo->tox ||
       mev->my < geo->toy ||
       mev->mx > geo->tox+geo->l ||
       mev->my > geo->toy+geo->h)
	return false;
    
    process_stop();
    state=stPressed;
    process_start();
    dbgprintf(("Button pressed()\n"));
    pressed_signal.emit();
}


bool  Button::mouse_unpress_event(MouseEvent* mev)
{
    if(state!=stPressed)
	return false;
    
    process_stop();
    state=stNormal;
    process_start();

    dbgprintf(("Button unpressed()\n"));
}


bool  Button::mouse_move_event(MouseEvent* mev)
{
    if(state==stHighlight)
    {
	if(mev->mx < geo->tox ||
	   mev->my < geo->toy ||
	   mev->mx > geo->tox+geo->l ||
	   mev->my > geo->toy+geo->h)
	{
	    process_stop();
	    state=mev->bstate==But1Press ? stPressed : stNormal;
	    process_start();
	    dbgprintf(("Button dehighlighted()\n"));
	    return true;
	}
    } else
    {
	if(mev->mx < geo->tox ||
	   mev->my < geo->toy ||
	   mev->mx > geo->tox+geo->l ||
	   mev->my > geo->toy+geo->h)
	    return false;
	else
	{
	    process_stop();
	    state=stHighlight;
	    process_start();
	    dbgprintf(("Button highlighted()\n"));
	    return true;
	}
    }
    return false;
}

void  Button::process_stop()
{
    stop_state_signal.emit(sprite[state], state);
    sprite[state]->hide();
}

void  Button::process_start()
{
    sprite[state]->show();
    start_state_signal.emit(sprite[state], state);
}

