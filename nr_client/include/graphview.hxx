#ifndef GRAPHVIEW_HXX
#define GRAPHVIEW_HXX
#include <vector>
#include <string>
#include "boost/shared_ptr.hpp"

#include "game_object.hxx"
#include "nr_dataflow.hxx"
#include "scan_task.hxx"

class Game;
class Graphview;

class Graphic
{
protected:
    RoastStorage          *store;
    int                    column;
    Graphview             *parent;
    int                    view_offset;
    Uint8                 *color;
    Geo                   *geo;

public:
    Graphic(Graphview* parent_, RoastStorage* store_, int column_, const char* name);

    virtual void           draw_graph(int from_x, int len, int cursor_x=-1);
    virtual void           set_view_x(int x) {view_offset=x;};
    virtual ~Graphic() {};
};

class GraphicAction: public Graphic
{
    Uint8                 *unload_color;
public:
    GraphicAction(Graphview* parent_, RoastStorage* store_, int column_, const char* name);

    virtual void           draw_graph(int from_x, int len, int cursor_x=-1);
};

typedef boost::shared_ptr<Graphic>  GraphicPtr;

typedef std::vector<GraphicPtr>     GraphVec;

class Graphview: public GameEditable
{
protected:
    friend class Graphic;
    friend class GraphicAction;

    Geo             *geo;
    SDL_Surface     *image;
    GRect            rgeo;
    Images           font_id;


    int              view_from_y, view_h;
    int              view_from_x, view_l;
    int              grid_step_y, grid_step_x;

    Uint8           *grid_color;
    Uint8           *bg_color;
    Uint8           *cursor_color;

    GraphVec         graphs;
    ScanTaskPtr      scan_task;

    int              last_offset;
    int              cursor_x;
    bool             frozen_view;

    virtual void     draw_view();
    virtual void     draw_view_partly(int from, int to);
    void             draw_back();
    bool             key_press_event(SDL_Event* ev);

public:
    Graphview(ScanTaskPtr& scan, Game* _game=0, const char* _name="", Images fnt_=imFont2);
    ~Graphview();

    virtual bool     show();
    virtual bool     draw(RectInfo&);
    virtual bool     hide();

    void             redraw();
    void             full_redraw();

    inline  int      scale_y(int y) { return geo->h - (y - view_from_y)*geo->h/view_h;};

    inline  Game*    get_game() { return game;};

    SigC::Signal0<void>                escape_signal;
    SigC::Signal1<void, SDL_Event*>    unprocessed_signal;
};

typedef boost::shared_ptr<Graphview>             GraphviewPtr;

#endif
