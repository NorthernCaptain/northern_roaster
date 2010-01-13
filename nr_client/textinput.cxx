#include "game.hxx"
#include "textinput.hxx"
#include "exceptions.hxx"
#include "debug.hxx"
#include "boost/lexical_cast.hpp"


const char* const from_en="qwertyuiop[]asdfghjkl;'zxcvbnm,./QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?";
const char* const from_ru="ÊÃÕËÅÎÇÛİÚÈßÆÙ×ÁĞÒÏÌÄÖÜÑŞÓÍÉÔØÂÀ.êãõëåîçûıúèÿæù÷áğòïìäöüñşóíéôøâà,";

unsigned char DecodeKOI8::to_koi[256];

void DecodeKOI8::init()
{
    unsigned int i;
    for(i=0;i<256;i++)
	to_koi[i]=(unsigned char)(i);
    for(i=0;i<ACE_OS::strlen(from_en);i++)
	to_koi[(unsigned)from_en[i]]=(unsigned char)from_ru[i];
}

Uint16 DecodeKOI8::decode(Uint16 from)
{
    return (int)(to_koi[from]);
}


Uint16 DecodeAlnum::decode(Uint16 from)
{
    return isalnum(from) ? from : from=='_' ? from : 0;
}

Uint16 DecodeFloat::decode(Uint16 from)
{
    return isdigit(from) ? from : from=='.' ? from : 0;
}

bool DecodeFloat::validate(const std::string& buf)
{
    try
    {
	 boost::lexical_cast<float>(buf);
	 return true;
    } 
    catch(...)
    {
    }

    return false;
}

Uint16 DecodeUInt::decode(Uint16 from)
{
    return isdigit(from) ? from : 0;
}

bool DecodeUInt::validate(const std::string& buf)
{
    try
    {
	 boost::lexical_cast<Uint32>(buf);
	 return true;
    } 
    catch(...)
    {
    }

    return false;
}

//////////////////////////////TextInput class methods//////////////////////////

Textinput::Textinput(const std::string& text, Game* _game, const char* _name, Images fnt) 
    : Textlabel(text, _game, _name, fnt)
{
    obj_type=GO_Textinput;
    decoder=new DecodePlain();
    alpha_step=geo[1].toy; //Step for decreasing alpha value of text in toy field

    alpha_max=geo[1].l;
    alpha_min=geo[1].h;
    alpha_value=alpha_max;

    cursor_image=game->new_surface(game->font_width(font_id, "P"), 
				   game->font_height(font_id));
    cursor_color=game->get_RGBA()+geo[1].fromy*4;

    DBG(5, "Textinput::constructor - cursor color id=" << geo[1].fromy);
    DBG(5, "Textinput::constructor - cursor color [0]=" <<
	Uint32(cursor_color[0]));
    DBG(5, "Textinput::constructor - cursor color [1]=" <<
	Uint32(cursor_color[1]));
    DBG(5, "Textinput::constructor - cursor color [2]=" <<
	Uint32(cursor_color[2]));

    sge_FilledRect(cursor_image, 
		   0, 0, 
		   cursor_image->w, 
		   cursor_image->h,
		   cursor_color[0],
		   cursor_color[1],
		   cursor_color[2]
		   );
    sge_UpdateRect(cursor_image, 0, 0, 
		   cursor_image->w, 
		   cursor_image->h);
    rcursor.w=cursor_image->w;
    rcursor.h=cursor_image->h;
    rcursor.y=rgeo.y;
    rcursor.x=rgeo.x;
}

bool Textinput::show()
{
    if(shown)
	return false;
    alpha_value=alpha_max;
    Textlabel::show();
    game->add_timer(this, geo[1].tox); // number of ticks in tox field
    DBG(5, "Textinput::show - show textinput object: " << name);
    return true;
}

bool Textinput::hide()
{
    if(!shown)
	return false;
    Textlabel::hide();
    game->del_timer(this);
    return true;
}

void Textinput::draw_view()
{
    Textlabel::draw_view();
    SDL_SetAlpha(cursor_image, SDL_SRCALPHA, alpha_value);
    if(focused)
	draw_cursor();
}

void Textinput::set_text(const std::string& str)
{
    alpha_value=alpha_max;
    Textlabel::set_text(str);
}

bool Textinput::time_out_event()
{
    if(!focused)
	return false;

    alpha_value += alpha_step;
    if(alpha_value > alpha_max ||
       alpha_value < alpha_min)
    {
	alpha_step = -alpha_step;
	alpha_value += alpha_step;	
    }

    SDL_SetAlpha(cursor_image, SDL_SRCALPHA, alpha_value);
    draw_cursor();
    game->redraw_area(rcursor);

    return true;
}

Textinput::~Textinput()
{
    game->delete_surface(cursor_image);
    delete decoder;
}

void Textinput::set_focus()
{
    Textlabel::set_focus();
    draw_cursor();
    game->redraw_area(rcursor);
}

void Textinput::unset_focus()
{
    Textlabel::unset_focus();
    draw_cursor();
    game->redraw_area(rcursor);
}


void Textinput::set_decoder(DecodeSym* dec)
{
    delete decoder;
    decoder=dec;
}

void Textinput::draw_cursor()
{
    int      w=game->font_width(font_id, data.c_str());
    rcursor.x = rgeo.x + w;
    
    DBG(9, "Textinput::font_width " << w);

    sge_Blit(game->image(geo->im), 
	     image,
	     rcursor.x, 
	     rcursor.y,
	     w,
	     rcursor.y - rgeo.y,
	     rcursor.w, rcursor.h);
    if(focused)
	sge_Blit(cursor_image, 
		 image,
		 0,
		 0,
		 w,
		 rcursor.y - rgeo.y,
		 rcursor.w,
		 rcursor.h);
    sge_UpdateRect(image, 
		   w,
		   rcursor.y - rgeo.y,
		   rcursor.w,
		   rcursor.h);
}

bool Textinput::key_press_event(SDL_Event* ev)
{
    switch(ev->key.keysym.sym)
    {
    case SDLK_RETURN:
	if(decoder->validate(data))
	{
	    DBG(5, "TextInput::activate - entered text: " << data);
	    activate_signal.emit(data);
	}
	break;
    case SDLK_ESCAPE:
	escape_signal.emit();
	break;
    case SDLK_TAB:
	next_focus_signal.emit();
	break;
    case SDLK_BACKSPACE:
	insert_symbol(8);
	break;
    default:
	DBG(9, "Textinput::unicode: " << ev->key.keysym.unicode);
	if(ev->key.keysym.unicode > 256 || 
	   ev->key.keysym.unicode < 32)
	    unprocessed_signal.emit(ev);
	else
	    insert_symbol(decoder->decode(ev->key.keysym.unicode));
	break;
    }
    return true;
}

bool Textinput::insert_symbol(Uint16 sym)
{
    if(sym==0)
	return false;
    if(sym==8)
    {
	if(data.empty())
	    return true;

	data.erase(data.size()-1, 1);
    }
    else
	data += char(sym);
    draw_view();
    game->redraw_area(rgeo);
    return true;
}
