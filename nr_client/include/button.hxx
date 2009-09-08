#ifndef BUTTON_HXX
#define BUTTON_HXX
#include "sprite.hxx"

enum ButState { stNormal=0, stPressed=1, stHighlight=2};
const int but_max_states=3;

class Button: public GameObject
{
protected:
    Geo             *geo;
    Sprite          *sprite[but_max_states];
    ButState         state;
    
    bool             mouse_press_event(MouseEvent*);
    bool             mouse_unpress_event(MouseEvent*);
    bool             mouse_move_event(MouseEvent*);

    virtual void     process_start();
    virtual void     process_stop();

public:
    Button(Game* _game, const char* _name);
    ~Button();

    virtual bool    show();
    virtual bool    hide();

    SigC::Signal0<void>    pressed_signal;
    SigC::Signal2<void, Sprite*, ButState>    start_state_signal;
    SigC::Signal2<void, Sprite*, ButState>    stop_state_signal;
};

#endif
