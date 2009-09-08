#include "game.hxx"
#include "screen_intro2.hxx"
#include "exceptions.hxx"
#include "debug.hxx"
#include "textlabelflash.hxx"
#include "commonfuncs.h"

#include "boost/lexical_cast.hpp"

Screen_Intro2::Screen_Intro2(ScanTaskPtr& scan, Game* _game, const char* _name) : Screen_Base(_game, _name), scan_task(scan)
{
    lvl_box=ListboxPtr(new Listbox(game, "lvl_listbox", imFont1));
    lvl_full_name=TextinputPtr(new Textinput("", game, "full_lvl", imFont1));

    sync_listbox();

    lvl_box->unprocessed_signal.connect(SigC::slot(*this, &Screen_Intro2::unprocessed_event));

    lvl_box->activate_signal.connect(SigC::slot(*this, &Screen_Intro2::lvl_item_activated));

    lvl_full_name->escape_signal.connect(SigC::slot(*lvl_box, &Listbox::my_focus));
    lvl_full_name->next_focus_signal.connect(SigC::slot(*lvl_box, &GameObject::my_focus));
    lvl_full_name->activate_signal.connect(SigC::slot(*this, &Screen_Intro2::lvl_edit_activated));
    lvl_full_name->unprocessed_signal.connect(SigC::slot(*this, &Screen_Intro2::unprocessed_event));

    wei_full_name=TextinputPtr(new Textinput("", game, "full_wei", imFont1));
    wei_full_name->escape_signal.connect(SigC::slot(*this, &Screen_Intro2::goto_prev_screen));
    wei_full_name->activate_signal.connect(SigC::slot(*this, &Screen_Intro2::wei_edit_activated));
    wei_full_name->unprocessed_signal.connect(SigC::slot(*this, &Screen_Intro2::unprocessed_event));
    wei_full_name->set_decoder(new DecodeFloat());

    lvl_box->escape_signal.connect(SigC::slot(*wei_full_name, &GameObject::my_focus));

    temp1_lbl=DigitlabelPtr(new Digitlabel(0, game, "main_temp1"));
    temp2_lbl=DigitlabelPtr(new Digitlabel(10, game, "main_temp2"));

//    lvl_full_name->set_decoder(new DecodeKOI8());

    wei_full_name->my_focus();
}

void Screen_Intro2::lvl_item_activated(ListboxItem item)
{
    if(item.id=="noop")
	return;

    if(item.id=="new")
    {
	lvl_full_name->set_text("");
	lvl_full_name->my_focus();
	return;
    }

    SortMap& smap = levels->get_map();
    for(SortMap::iterator it=smap.begin(); it!=smap.end(); it++)
    {
	DBG(5, "CMP: " << it->second << " == " << item.descr);
	if(it->second->name==item.descr)
	{
	    scan_task->set_lvl_name(it->first, it->second->name);
	    break;
	}
    }

    scan_task->set_weight_name(wei_full_name->get_text(), wei_full_name->get_text());

    hide();
    finished_signal.emit();
}

void Screen_Intro2::lvl_edit_activated(std::string&)
{
    std::string      short_str;
    std::string      full_str=lvl_full_name->get_text();

    if(full_str.empty())
    {
	lvl_full_name->my_focus();
	return;
    }

    generate_uname(full_str, short_str);

    if(!levels->add(short_str, full_str))
    {
	lvl_full_name->set_text("");
	lvl_full_name->my_focus();
	return;
    }
    
    lvl_full_name->set_text("");

    try
    {
	scan_task->send_levels();
	lvl_box->add_data(ListboxItem(short_str, full_str));
	lvl_box->sort();
	lvl_box->set_cursor_by_item_idx(short_str);
    }
    catch(...)
    {
    }
    sync_listbox();
    lvl_box->my_focus();
}

void Screen_Intro2::wei_edit_activated(std::string& wei)
{
    if(wei.empty())
	return;

    try
    {
	float w = boost::lexical_cast<float>(wei);
	if(w < 1.0)
	    throw 1;
    }
    catch(...)
    {
	set_warn(std::string("Введен неверный вес загрузки: " +wei));
	wei_full_name->set_text("");
	wei_full_name->my_focus();
	return;
    }
    lvl_box->my_focus();
}

void Screen_Intro2::unprocessed_event(SDL_Event* ev)
{
    if(ev->type!=SDL_KEYDOWN)
	return;
    switch(ev->key.keysym.sym)
    {
    case SDLK_F4:
	break;
    default:
	break;
    }
}

void Screen_Intro2::sync_listbox()
{
    levels = &scan_task->get_levels();
    SortMap& smap2 = levels->get_map();
    ListboxItem itm = lvl_box->get_current();
    lvl_box->clear_data();
    for(SortMap::iterator it=smap2.begin(); it!=smap2.end(); it++)
	lvl_box->add_data(ListboxItem(it->first, it->second->name));
    lvl_box->add_data(ListboxItem("noop", "-  Выбор  -"));
    lvl_box->add_data(ListboxItem("new", "-- Новый --"));
    lvl_box->sort();
    lvl_box->redraw();
    lvl_box->set_cursor_by_item_idx(itm.id);
}

void Screen_Intro2::goto_prev_screen()
{
    if(prev_screen)
    {
	hide();
	prev_screen->show();
    }
}

bool Screen_Intro2::idle_time_out_event()
{
    goto_prev_screen();
    return true;
}

bool Screen_Intro2::show()
{
    if(shown)
	return true;
    shown=true;

    lvl_box->set_cursor_by_item_idx("noop");

    game->scene_add(this);
    game->add_object(this);
    Game::bg=game->image(imIntro2BG);
    GRect frect;
    frect.x=frect.y=0;
    frect.h=SCREEN_H; 
    frect.w=SCREEN_L;

    set_info(std::string(scan_task->get_storage()->roaster) + ", введите вес и степень обжарки", true);

    game->redraw_area(frect);
    lvl_box->show();
    lvl_full_name->show();
    wei_full_name->show();

    temp1_lbl->show();
    temp2_lbl->show();
    info_lbl->show();
    game->set_focus(wei_full_name.get());
    game->set_idle_time_call(this, wEvent(aIdleTimeout), 180);

    DBG(5, "Screen_Intro2::show - show screen_intro object: " << name);
    return true;
}

bool Screen_Intro2::hide()
{
    if(!shown)
	return true;
    game->scene_del(this);
    game->del_object(this);
    shown=false;
    lvl_box->hide();
    lvl_full_name->hide();
    wei_full_name->hide();
    temp1_lbl->hide();
    temp2_lbl->hide();
    info_lbl->hide();
    game->set_idle_time_call(0, wEvent(aIdleTimeout), 0);

    DBG(5, "Screen_Intro2::hide - hide screen_intro object: " << name);
    return true;
}


Screen_Intro2::~Screen_Intro2()
{
}

bool Screen_Intro2::user_event(SDL_Event* ev)
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
	    DBG(4, "INTRO2Temp: offset=" << last_offset << " temp1=" << row.get_int(t_input) << " raw=" << row.row[t_input])
	}
	break;
    default:
	break;
    }
    delete sev;
    return true;
}
