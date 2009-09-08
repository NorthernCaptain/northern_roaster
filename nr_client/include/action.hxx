#ifndef ACTION_HXX
#define ACTION_HXX

#include <list>
#include "game_object.hxx"

class ActionQueue;

class SimpleAction
{
    Game*               game;
    Geo*                geo;
    ActionQueue*        aqueue;      
    bool                auto_delete;

public:
    SimpleAction(Game* _game=0, const char* _name="");
    virtual ~SimpleAction() {};
    
    void    set_action_queue(ActionQueue* aq) { aqueue=aq;};

    bool    need_auto_delete() { return auto_delete;};

    virtual void        start()=0;
    virtual void        stop()=0;
};


typedef     std::list<SimpleAction*>     ActionList;

class ActionQueue
{
    ActionList          que;
    SimpleAction*       current_act;
    
    void                activate();

public:
    ActionQueue();
    ~ActionQueue() {};

    bool                add(SimpleAction*);
    bool                add_only(SimpleAction*);
    bool                replace_current(SimpleAction*);
    bool                clear_all();
    bool                next_action();
    bool                stop_current();
    SimpleAction*       current() { return current_act;};
};

#endif
