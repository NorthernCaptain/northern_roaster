#include "game.hxx"
#include "screen_login.hxx"
#include "exceptions.hxx"
#include "debug.hxx"
#include "version_cln.hxx"

#include "boost/lexical_cast.hpp"

Screen_Login::Screen_Login(ScanTaskPtr& scan, Game* _game, const char* _name) 
    : Screen_Base(_game, _name), scan_task(scan)
{
    lbox=ListboxPtr(new Listbox(game, "login_listbox", imFont1));
    full_name=TextinputPtr(new Textinput("", game, "full_login", imFont1));
    short_name=TextinputPtr(new Textinput("", game, "short_login", imFont1));

    set_info(ClnVersion);

    sync_listbox();

    lbox->escape_signal.connect(SigC::slot(*this, &Screen_Login::process_exit));
    full_name->escape_signal.connect(SigC::slot(*lbox, &Listbox::my_focus));
    short_name->escape_signal.connect(SigC::slot(*lbox, &Listbox::my_focus));

    full_name->next_focus_signal.connect(SigC::slot(*short_name, &GameObject::my_focus));
    short_name->next_focus_signal.connect(SigC::slot(*full_name, &GameObject::my_focus));

    full_name->activate_signal.connect(SigC::slot(*this, &Screen_Login::edit_activated));
    short_name->activate_signal.connect(SigC::slot(*this, &Screen_Login::edit_activated));
    short_name->unprocessed_signal.connect(SigC::slot(*this, &Screen_Login::unprocessed_event));
    full_name->unprocessed_signal.connect(SigC::slot(*this, &Screen_Login::unprocessed_event));
    lbox->unprocessed_signal.connect(SigC::slot(*this, &Screen_Login::unprocessed_event));

    lbox->activate_signal.connect(SigC::slot(*this, &Screen_Login::item_activated));

    temp1_lbl=DigitlabelPtr(new Digitlabel(0, game, "main_temp1"));
    temp2_lbl=DigitlabelPtr(new Digitlabel(10, game, "main_temp2"));

//    full_name->set_decoder(new DecodeKOI8());
    short_name->set_decoder(new DecodeAlnum());

    lbox->my_focus();
}


void Screen_Login::process_exit()
{
    DBG(2, "Screen_Login: User requested exit operation (Logout), processing...");
    game->game_over();
}

//Go to new screen, login user and start new session
void Screen_Login::item_activated(ListboxItem item)
{
    if(item.id=="noop")
    {
	sync_listbox();
	return;
    }

    if(item.id=="new")
    {
	full_name->set_text("");
	short_name->set_text("");
	full_name->my_focus();
	return;
    }

    hide();
    game->scene_del(this);
    SortMap& smap = sorts->get_map();
    for(SortMap::iterator it=smap.begin(); it!=smap.end(); it++)
	if(it->second->name==item.descr)
	    scan_task->set_user_name(it->first, it->second->name);

    finished_signal.emit();
}

void Screen_Login::item_selected(ListboxItem item)
{
}

void Screen_Login::edit_activated(std::string&)
{
    std::string      short_str=short_name->get_text();
    std::string      full_str=full_name->get_text();

    if(short_str.empty())
    {
	short_name->my_focus();
	return;
    }

    if(full_str.empty())
    {
	full_name->my_focus();
	return;
    }

    if(!sorts->add(short_str, full_str))
    {
	short_name->set_text("");
	short_name->my_focus();
	return;
    }
    
    full_name->set_text("");
    short_name->set_text("");

    try
    {
	scan_task->send_users();
	lbox->add_data(ListboxItem(short_str, full_str));
	lbox->sort();
	lbox->set_cursor_by_item_idx(short_str);
    }
    catch(...)
    {
    }
    sync_listbox();
    lbox->my_focus();
}

void Screen_Login::unprocessed_event(SDL_Event* ev)
{
    if(ev->type!=SDL_KEYDOWN)
	return;
    switch(ev->key.keysym.sym)
    {
    case SDLK_F4:
	full_name->set_text("");
	short_name->set_text("");
	full_name->my_focus();
	break;
    default:
	break;
    }
}

void Screen_Login::sync_listbox()
{
    sorts = &scan_task->get_users();
    SortMap& smap = sorts->get_map();
    ListboxItem lbox_item = lbox->get_current();
    lbox->clear_data();

    for(SortMap::iterator it=smap.begin(); it!=smap.end(); it++)
	lbox->add_data(ListboxItem(it->first, it->second->name));

    lbox->add_data(ListboxItem("noop", "-  Выбор  -"));
    lbox->add_data(ListboxItem("new", "-- Новый --"));
    lbox->sort();
    lbox->redraw();
    lbox->set_cursor_by_item_idx(lbox_item.id);
}

bool Screen_Login::show()
{
    if(shown)
	return true;
    shown=true;

    lbox->set_cursor_by_item_idx("noop");

    game->scene_add(this);
    game->add_object(this);
    Game::bg=game->image(imLoginBG);
    GRect frect;
    frect.x=frect.y=0;
    frect.h=SCREEN_H; 
    frect.w=SCREEN_L;

    set_info(std::string(ClnVersion)+ " / " + scan_task->server_version());

    game->redraw_area(frect);
    lbox->show();
    full_name->show();
    short_name->show();
    temp1_lbl->show();
    temp2_lbl->show();
    info_lbl->show();
    game->set_focus(lbox.get());

    DBG(5, "Screen_Login::show - show screen_login object: " << name);
    return true;
}

bool Screen_Login::hide()
{
    if(!shown)
	return true;
    game->scene_del(this);
    game->del_object(this);
    shown=false;
    lbox->hide();
    full_name->hide();
    short_name->hide();
    temp1_lbl->hide();
    temp2_lbl->hide();
    info_lbl->hide();
    DBG(5, "Screen_Login::hide - hide screen_login object: " << name);
    return true;
}


Screen_Login::~Screen_Login()
{
}

bool Screen_Login::user_event(SDL_Event* ev)
{
    if(ev->user.code!=1)
	return false;
    ScreenEvent*  sev = (ScreenEvent*)ev->user.data1;

    switch(sev->type)
    {
    case se_update_grid:
	{
	    RoastStorage*  store=scan_task->get_storage();
	    ROAST_LOCK(store);

	    int last_offset=store->total_rows-1;
	    TempRow& row=(*store)[last_offset];
	    temp1_lbl->set_value(row.get_int(t_input));
	    temp2_lbl->set_value(row.get_int(t_output));
	}
	break;
    default:
	break;
    }
    delete sev;
    return true;
}

