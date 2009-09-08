#ifndef TEXTINPUT_HXX
#define TEXTINPUT_HXX
#include "textlabel.hxx"

#include <vector>
#include <string>
#include "boost/shared_ptr.hpp"

class Game;

class DecodeSym
{
    int          type;
public:
    virtual Uint16  decode(Uint16 from)=0;
    virtual bool    validate(const std::string& buf) { return true;};
    virtual ~DecodeSym() {};
};

class DecodePlain: public DecodeSym
{
public:
    Uint16          decode(Uint16 from) { return from;};
};


class DecodeKOI8: public DecodeSym
{
    static unsigned char  to_koi[256];

public:
    Uint16       decode(Uint16 from);
    static void  init();
};

class DecodeAlnum: public DecodeSym
{
public:
    Uint16       decode(Uint16 from);
};

class DecodeFloat: public DecodeSym
{
public:
    Uint16       decode(Uint16 from);
    bool         validate(const std::string& buf);
};

class Textinput: public Textlabel
{
    SDL_Surface*     cursor_image;
    GRect            rcursor;
    int              alpha_value;
    int              alpha_step;
    int              alpha_max;
    int              alpha_min;
    Uint8           *cursor_color;
    DecodeSym       *decoder;

    bool             time_out_event();
    
    void             draw_view();

    void             draw_cursor();
    bool             key_press_event(SDL_Event* ev);
    bool             insert_symbol(Uint16 sym);

public:
    Textinput(const std::string& text="", Game* _game=0, const char* _name="", Images fnt=imFont1);
    ~Textinput();

    void             set_text(const std::string& new_data);
    bool             show();
    bool             hide();

    void             set_focus();
    void             unset_focus();
    void             my_focus() { GameEditable::my_focus();};
    void             set_decoder(DecodeSym*);

    const std::string& get_text() const { return data;};

    SigC::Signal0<void>                escape_signal;
    SigC::Signal1<void, std::string&>  activate_signal;
    SigC::Signal1<void, SDL_Event*>    unprocessed_signal;
};

typedef boost::shared_ptr<Textinput>             TextinputPtr;

#endif
