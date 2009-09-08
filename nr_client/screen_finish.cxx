#include "game.hxx"
#include "screen_finish.hxx"
#include "exceptions.hxx"
#include "debug.hxx"
#include "textlabelflash.hxx"
#include "commonfuncs.h"

#include "boost/lexical_cast.hpp"

Screen_Finish::Screen_Finish(ScanTaskPtr& scan, Game* _game, const char* _name) : Screen_Base(_game, _name), scan_task(scan)
{
    lvl_box=ListboxPtr(new Listbox(game, "finish_listbox", imFont1));
    lvl_full_name=TextinputPtr(new Textinput("", game, "finish_full_lvl", imFont1));

    sync_listbox();

    lvl_box->unprocessed_signal.connect(SigC::slot(*this, &Screen_Finish::unprocessed_event));

    lvl_box->activate_signal.connect(SigC::slot(*this, &Screen_Finish::lvl_item_activated));

    lvl_full_name->escape_signal.connect(SigC::slot(*lvl_box, &Listbox::my_focus));
    lvl_full_name->next_focus_signal.connect(SigC::slot(*lvl_box, &GameObject::my_focus));
    lvl_full_name->activate_signal.connect(SigC::slot(*this, &Screen_Finish::lvl_edit_activated));
    lvl_full_name->unprocessed_signal.connect(SigC::slot(*this, &Screen_Finish::unprocessed_event));

    wei_full_name=TextinputPtr(new Textinput("", game, "finish_full_wei", imFont1));
    wei_full_name->escape_signal.connect(SigC::slot(*this, &Screen_Finish::goto_prev_screen));
    wei_full_name->activate_signal.connect(SigC::slot(*this, &Screen_Finish::wei_edit_activated));
    wei_full_name->unprocessed_signal.connect(SigC::slot(*this, &Screen_Finish::unprocessed_event));
    wei_full_name->set_decoder(new DecodeFloat());

    lvl_box->escape_signal.connect(SigC::slot(*wei_full_name, &GameObject::my_focus));


    temp1_lbl=DigitlabelPtr(new Digitlabel(0, game, "main_temp1"));
    temp2_lbl=DigitlabelPtr(new Digitlabel(10, game, "main_temp2"));

 //   lvl_full_name->set_decoder(new DecodeKOI8());

    wei_full_name->my_focus();
}

void Screen_Finish::lvl_item_activated(ListboxItem item)
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
	DBG(5, "CMP: " << it->second->name << " == " << item.descr);
	if(it->second->name==item.descr)
	{
	    scan_task->set_state_name(it->first, it->second->name);
	    break;
	}
    }

    scan_task->set_output_weight_name(wei_full_name->get_text(), wei_full_name->get_text());
    
    scan_task->finish_roast();

    hide();
    finished_signal.emit();
}

void Screen_Finish::lvl_edit_activated(std::string&)
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
	scan_task->send_states();
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

void Screen_Finish::wei_edit_activated(std::string& wei)
{
    if(wei.empty())
	return;

    try
    {
	scan_task->set_output_weight_name(wei_full_name->get_text(), wei_full_name->get_text());
    }
    catch(...)
    {
	set_warn("Введен неверный вес готового кофе: " + wei + " (надо >= 1 и <= " 
		 + scan_task->get_storage()->wei_descr + ")");
	wei_full_name->set_text("");
	wei_full_name->my_focus();
	return;
    }
    lvl_box->my_focus();
}

void Screen_Finish::unprocessed_event(SDL_Event* ev)
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

void Screen_Finish::sync_listbox()
{
    levels = &scan_task->get_states();
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

void Screen_Finish::goto_prev_screen()
{
    if(prev_screen)
    {
	hide();
	prev_screen->show();
    }
}

bool Screen_Finish::show()
{
    if(shown)
	return true;
    shown=true;

    lvl_box->set_cursor_by_item_idx("noop");

    game->scene_add(this);
    game->add_object(this);
    Game::bg=game->image(imFinishBG);
    GRect frect;
    frect.x=frect.y=0;
    frect.h=SCREEN_H; 
    frect.w=SCREEN_L;

    set_info(std::string(scan_task->get_storage()->roaster) + ", введите результаты обжарки", true);

    game->redraw_area(frect);
    lvl_box->show();
    lvl_full_name->show();
    wei_full_name->show();

    temp1_lbl->show();
    temp2_lbl->show();
    info_lbl->show();
    game->set_focus(wei_full_name.get());

    DBG(5, "Screen_Finish::show - show screen_intro object: " << name);
    return true;
}

bool Screen_Finish::hide()
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

    scan_task->send(new ScanTaskEvent(stask_logout));

    DBG(5, "Screen_Finish::hide - hide screen_intro object: " << name);
    return true;
}


Screen_Finish::~Screen_Finish()
{
}

bool Screen_Finish::user_event(SDL_Event* ev)
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
	    DBG(4, "FinishTemp: offset=" << last_offset << " temp1=" << row.get_int(t_input) << " raw=" << row.row[t_input])
	}
	break;
    default:
	break;
    }
    delete sev;
    return true;
}
