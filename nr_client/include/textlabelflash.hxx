#ifndef TEXTLABELFLASH_HXX
#define TEXTLABELFLASH_HXX
#include "textlabel.hxx"

#include <vector>
#include <string>
#include "boost/shared_ptr.hpp"

class Game;


class Textlabelflash: public Textlabel
{
    int              alpha_value;
    int              alpha_step;
    int              alpha_max;
    int              alpha_min;
    bool             time_out_event();
    
    void             draw_view();
public:
    Textlabelflash(const std::string& text="", Game* _game=0, const char* _name="", Images fnt=imFont1);
    ~Textlabelflash();

    void             set_text(const std::string& new_data);
    bool             show();
    bool             hide();
};

typedef boost::shared_ptr<Textlabelflash>             TextlabelflashPtr;

#endif
