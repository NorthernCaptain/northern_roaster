#ifndef LISTBOX_HXX
#define LISTBOX_HXX
#include "game_object.hxx"

#include <vector>
#include <string>
#include "boost/shared_ptr.hpp"

class Game;

struct ListboxItem
{
    std::string       id;
    std::string       descr;

    ListboxItem(const std::string& iid="", const std::string& _descr=""): id(iid), descr(_descr) {};

    bool operator < (const ListboxItem& i) const { return descr < i.descr;};
};

typedef std::vector<ListboxItem>           ListboxVec;

class Listbox: public GameEditable
{
    Geo             *geo;
    SDL_Surface     *image;
    GRect            rgeo;
    Images           font_id;

    ListboxVec       data;

    int              view_step_y;
    int              view_items;
    int              base_idx;
    int              cur_idx;
    Uint8           *cursor_color;

    void             copy_bg();

    void             draw_item(int view_pos, int idx);
    void             draw_view();

    bool             key_press_event(SDL_Event* ev);

    bool             cursor_up();
    bool             cursor_down();

public:
    Listbox(Game* _game=0, const char* _name="", Images fnt=imFont1);
    ~Listbox();

    virtual bool     show();
    virtual bool     draw(RectInfo&);
    virtual bool     hide();
    virtual void     redraw();

    void             add_data(const ListboxItem& new_item) { data.push_back(new_item);};
    void             clear_data() { data.clear(); cur_idx=base_idx=0;};
    int              size() const { return data.size();};

    void             set_cursor_by_item_idx(const std::string& idx);
    void             sort();

    void             set_focus();
    void             unset_focus();
    ListboxItem      get_current();

    SigC::Signal0<void>                escape_signal;
    SigC::Signal1<void, ListboxItem>   activate_signal;
    SigC::Signal1<void, ListboxItem>   item_selected_signal;
    SigC::Signal1<void, SDL_Event*>    unprocessed_signal;
};

typedef boost::shared_ptr<Listbox>             ListboxPtr;

#endif
