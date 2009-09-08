#include "game.hxx"
#include "textlabelflash.hxx"
#include "exceptions.hxx"
#include "debug.hxx"

Textlabelflash::Textlabelflash(const std::string& text, Game* _game, const char* _name, Images fnt) 
    : Textlabel(text, _game, _name, fnt)
{
    obj_type=GO_Textlabelflash;
    alpha_step=geo[1].toy; //Step for decreasing alpha value of text in toy field

    alpha_max=geo[1].l;
    alpha_min=geo[1].h;
    alpha_value=alpha_max;
}

bool Textlabelflash::show()
{
    if(shown)
	return false;
    alpha_value=alpha_max;
    Textlabel::show();
    game->add_timer(this, geo[1].tox); // number of ticks in tox field
    DBG(5, "Textlabelflash::show - show textlabelflash object: " 
	<< name << ", flash tick is " << geo[1].tox);
    return true;
}

bool Textlabelflash::hide()
{
    if(!shown)
	return false;
    Textlabel::hide();
    game->del_timer(this);
    return true;
}

void Textlabelflash::draw_view()
{
    Textlabel::draw_view();
    SDL_SetAlpha(image, SDL_SRCALPHA, alpha_value);
}

void Textlabelflash::set_text(const std::string& str)
{
    alpha_value=alpha_max;
    Textlabel::set_text(str);
}

bool Textlabelflash::time_out_event()
{
    alpha_value += alpha_step;
    if(alpha_value > alpha_max ||
       alpha_value < alpha_min)
    {
	alpha_step = -alpha_step;
	alpha_value += alpha_step;	
    }

    SDL_SetAlpha(image, SDL_SRCALPHA, alpha_value);
    game->redraw_area(rgeo);
    return true;
}

Textlabelflash::~Textlabelflash()
{
}
