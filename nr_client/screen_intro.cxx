#include "game.hxx"
#include "screen_intro.hxx"
#include "exceptions.hxx"
#include "debug.hxx"
#include "textlabelflash.hxx"
#include "commonfuncs.h"

#include "boost/lexical_cast.hpp"

Screen_Intro::Screen_Intro(ScanTaskPtr& scan, Game* _game, const char* _name) : Screen_Base(_game, _name), scan_task(scan)
{
    bigmsg_lbl=TextlabelPtr(new Textlabel("", game, "bigmsg_lbl", imFont2));

    lbox=ListboxPtr(new Listbox(game, "sort_listbox", imFont1));
    full_name=TextinputPtr(new Textinput("", game, "full_sort", imFont1));

    sts_box=ListboxPtr(new Listbox(game, "istat_listbox", imFont1));
    sts_full_name=TextinputPtr(new Textinput("", game, "sts_full_name", imFont1));

    lvl_box=ListboxPtr(new Listbox(game, "isort_listbox", imFont1));

    sync_listbox();

    full_name->escape_signal.connect(SigC::slot(*lbox, &Listbox::my_focus));
    full_name->next_focus_signal.connect(SigC::slot(*lbox, &GameObject::my_focus));
    full_name->activate_signal.connect(SigC::slot(*this, &Screen_Intro::sort_edit_activated));
    full_name->unprocessed_signal.connect(SigC::slot(*this, &Screen_Intro::unprocessed_event));


    sts_full_name->escape_signal.connect(SigC::slot(*sts_box, &Listbox::my_focus));
    sts_full_name->next_focus_signal.connect(SigC::slot(*sts_box, &GameObject::my_focus));
    sts_full_name->activate_signal.connect(SigC::slot(*this, &Screen_Intro::sts_edit_activated));
    sts_full_name->unprocessed_signal.connect(SigC::slot(*this, &Screen_Intro::unprocessed_event));

    lbox->unprocessed_signal.connect(SigC::slot(*this, &Screen_Intro::unprocessed_event));
    lvl_box->unprocessed_signal.connect(SigC::slot(*this, &Screen_Intro::unprocessed_event));
    sts_box->unprocessed_signal.connect(SigC::slot(*this, &Screen_Intro::unprocessed_event));

    lbox->activate_signal.connect(SigC::slot(*this, &Screen_Intro::sort_item_activated));
    lvl_box->activate_signal.connect(SigC::slot(*this, &Screen_Intro::lvl_item_activated));
    sts_box->activate_signal.connect(SigC::slot(*this, &Screen_Intro::sts_item_activated));

    lbox->escape_signal.connect(SigC::slot(*this, &Screen_Intro::goto_prev_screen));
    lvl_box->escape_signal.connect(SigC::slot(*lbox, &Listbox::my_focus));
    sts_box->escape_signal.connect(SigC::slot(*lvl_box, &Listbox::my_focus));

    temp1_lbl=DigitlabelPtr(new Digitlabel(0, game, "main_temp1"));
    temp2_lbl=DigitlabelPtr(new Digitlabel(10, game, "main_temp2"));

    //    sts_full_name->set_decoder(new DecodeKOI8());


    lbox->my_focus();
}

void Screen_Intro::sts_item_activated(ListboxItem item)
{
    if(item.id=="noop")
	return;

    if(item.id=="new")
    {
	sts_full_name->set_text("");
	sts_full_name->my_focus();
	return;
    }

    SortMap& smap = sts->get_map();
    for(SortMap::iterator it=smap.begin(); it!=smap.end(); it++)
    {
	DBG(5, "CMP: " << it->second << " == " << item.descr);
	if(it->second->name==item.descr)
	{
	    scan_task->set_in_state_name(it->first, it->second->name);
	    break;
	}
    }

    hide();
    finished_signal.emit();
}

void Screen_Intro::sts_edit_activated(std::string&)
{
    std::string      short_str;
    std::string      full_str=sts_full_name->get_text();

    if(full_str.empty())
    {
	sts_full_name->my_focus();
	return;
    }

    generate_uname(full_str, short_str);

    if(!sts->add(short_str, full_str))
    {
	sts_full_name->set_text("");
	sts_full_name->my_focus();
	return;
    }
    
    sts_full_name->set_text("");

    try
    {
	scan_task->send_states();
	sts_box->add_data(ListboxItem(short_str, full_str));
	sts_box->sort();
	sts_box->set_cursor_by_item_idx(short_str);
    }
    catch(...)
    {
    }
    sync_listbox();
    sts_box->my_focus();
}

//Go to new screen, login user and start new session
void Screen_Intro::sort_item_activated(ListboxItem item)
{
    if(item.id=="noop")
	return;

    if(item.id=="new")
    {
	full_name->set_text("");
	full_name->my_focus();
	return;
    }

    SortMap& smap = sorts->get_map();
    for(SortMap::iterator it=smap.begin(); it!=smap.end(); it++)
    {
	DBG(5, "CMP: " << it->second->name << " == " << item.descr);
	if(it->second->name==item.descr)
	{
	    scan_task->set_sort_name(it->first, it->second->name);
	    break;
	}
    }

    sync_listbox();

    lvl_box->my_focus();
}

void Screen_Intro::sort_edit_activated(std::string&)
{
    std::string      short_str;
    std::string      full_str=full_name->get_text();

    if(full_str.empty())
    {
	full_name->my_focus();
	return;
    }

    generate_uname(full_str, short_str);

    if(!sorts->add(short_str, full_str))
    {
	full_name->set_text("");
	full_name->my_focus();
	return;
    }
    
    full_name->set_text("");

    try
    {
	scan_task->send_coffee_sorts();
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

void Screen_Intro::lvl_item_activated(ListboxItem item)
{
    if(item.id=="noop")
	return;

    SortMap& smap = levels->get_map();
    for(SortMap::iterator it=smap.begin(); it!=smap.end(); it++)
    {
	DBG(5, "CMP: " << it->second << " == " << item.descr);
	if(it->second->name==item.descr)
	{
	    scan_task->set_input_sort_name(it->first, it->second->name);
	    set_bigmsg(std::string("Сейчас сырья на складе: ") + boost::lexical_cast<std::string>((float)(it->second->qty)/1000.0) + " кг",
		     it->second->qty <= 50000);
	    break;
	}
    }

    sts_box->my_focus();
}


void Screen_Intro::set_bigmsg(const std::string& nfo, bool flash)
{
    TextlabelPtr ptr=TextlabelPtr(flash ? new Textlabelflash(nfo, game, "bigmsg_lbl", imFont6) 
				 : new Textlabel(nfo, game, "bigmsg_lbl", imFont1));

    if(bigmsg_lbl->is_shown())
	bigmsg_lbl->hide();
    ptr->show();
    bigmsg_lbl.swap(ptr);
}


void Screen_Intro::unprocessed_event(SDL_Event* ev)
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

void Screen_Intro::sync_listbox()
{
    int i=0;
    sorts = &scan_task->get_coffee_sorts();
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

    sts = &scan_task->get_states();
    SortMap& smap2 = sts->get_map();
    ListboxItem stsbox_item = sts_box->get_current();
    sts_box->clear_data();
    i=0;
    for(SortMap::iterator it=smap2.begin(); it!=smap2.end(); it++)
	sts_box->add_data(ListboxItem(it->first, it->second->name));
    sts_box->add_data(ListboxItem("noop", "-  Выбор  -"));
    sts_box->add_data(ListboxItem("new", "-- Новый --"));
    sts_box->sort();
    sts_box->redraw();
    sts_box->set_cursor_by_item_idx(stsbox_item.id);

    levels = &scan_task->get_input_sorts();
    SortMap& smap3 = levels->get_map();
    ListboxItem lvlbox_item = lvl_box->get_current();
    lvl_box->clear_data();
    i=0;
    for(SortMap::iterator it=smap3.begin(); it!=smap3.end(); it++)
	lvl_box->add_data(ListboxItem(it->first, it->second->name));
    lvl_box->add_data(ListboxItem("noop", "-  Выбор  -"));
    lvl_box->sort();
    lvl_box->redraw();
    lvl_box->set_cursor_by_item_idx(lvlbox_item.id);
}

void Screen_Intro::goto_prev_screen()
{
    if(prev_screen)
    {
	hide();
	prev_screen->show();
    }
}

bool Screen_Intro::idle_time_out_event()
{
    goto_prev_screen();
    return true;
}

bool Screen_Intro::show()
{
    if(shown)
	return true;
    shown=true;

    lbox->set_cursor_by_item_idx("noop");
    lvl_box->set_cursor_by_item_idx("noop");
    sts_box->set_cursor_by_item_idx("noop");

    game->scene_add(this);
    game->add_object(this);
    Game::bg=game->image(imIntroBG);
    GRect frect;
    frect.x=frect.y=0;
    frect.h=SCREEN_H; 
    frect.w=SCREEN_L;

    //    set_warn(std::string(scan_task->get_storage()->roaster) + ", введите параметры обжарки");
    set_info(std::string(scan_task->get_storage()->roaster) + ", введите параметры обжарки", false);

    game->redraw_area(frect);
    lbox->show();
    full_name->show();
    sts_full_name->show();
    lvl_box->show();
    sts_box->show();
    temp1_lbl->show();
    temp2_lbl->show();
    info_lbl->show();
    game->set_focus(lbox.get());

    game->set_idle_time_call(this, wEvent(aIdleTimeout), 300);

    DBG(5, "Screen_Intro::show - show screen_intro object: " << name);
    return true;
}

bool Screen_Intro::hide()
{
    if(!shown)
	return true;
    game->scene_del(this);
    game->del_object(this);
    shown=false;
    lbox->hide();
    full_name->hide();
    sts_full_name->hide();
    lvl_box->hide();
    sts_box->hide();
    temp1_lbl->hide();
    temp2_lbl->hide();
    info_lbl->hide();
    bigmsg_lbl->hide();

    game->set_idle_time_call(0, wEvent(aIdleTimeout), 0);

    DBG(5, "Screen_Intro::hide - hide screen_intro object: " << name);
    return true;
}


Screen_Intro::~Screen_Intro()
{
}

bool Screen_Intro::user_event(SDL_Event* ev)
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
	    DBG(4, "INTROTemp: offset=" << last_offset << " temp1=" << row.get_int(t_input) << " raw=" << row.row[t_input])
	}
	break;
    default:
	break;
    }
    delete sev;
    return true;
}
