#include "game.hxx"
#include "screen_confirm.hxx"
#include "exceptions.hxx"
#include "debug.hxx"
#include "textlabelflash.hxx"
#include "commonfuncs.h"

#include "boost/lexical_cast.hpp"

Screen_Confirm::Screen_Confirm(ScanTaskPtr& scan, Game* _game, const char* _name) : Screen_Base(_game, _name), scan_task(scan)
{
    bigmsg_lbl=TextlabelPtr(new Textlabel("", game, "cfm_bigmsg_lbl", imFont1));

    lbox=ListboxPtr(new Listbox(game, "cfm_sort_listbox", imFont1));

    sync_listbox();

    lbox->unprocessed_signal.connect(SigC::slot(*this, &Screen_Confirm::unprocessed_event));
    lbox->activate_signal.connect(SigC::slot(*this, &Screen_Confirm::sort_item_activated));
    lbox->escape_signal.connect(SigC::slot(*this, &Screen_Confirm::goto_prev_screen));

    temp1_lbl=DigitlabelPtr(new Digitlabel(0, game, "main_temp1"));
    temp2_lbl=DigitlabelPtr(new Digitlabel(10, game, "main_temp2"));

    //    sts_full_name->set_decoder(new DecodeKOI8());


    lbox->my_focus();
}


//Go to new screen, login user and start new session
void Screen_Confirm::sort_item_activated(ListboxItem item)
{
    if(item.id=="noop")
	return;

    bool nodiff = true;

    SortMap& smap = sorts->get_map();
    for(SortMap::iterator it=smap.begin(); it!=smap.end(); it++)
    {
	DBG(5, "CMP: " << it->second->name << " == " << item.descr);
	if(it->second->name==item.descr)
	{
	    nodiff = strcmp(scan_task->get_storage()->coffee_sort_short, it->first.c_str()) == 0;
	    break;
	}
    }

    if(nodiff)
    {
	hide();
	finished_signal.emit();
    } else
    {
	set_warn("Введен другой сорт кофе, проверьте правильность выбора!");
	set_bigmsg(scan_task->get_storage()->coffee_sort, false);
    }
}

void Screen_Confirm::set_bigmsg(const std::string& nfo, bool flash)
{
    TextlabelPtr ptr=TextlabelPtr(flash ? new Textlabelflash(nfo, game, "cfm_bigmsg_lbl", imFont6) 
				 : new Textlabel(nfo, game, "cfm_bigmsg_lbl", imFont1));

    if(bigmsg_lbl->is_shown())
	bigmsg_lbl->hide();
    ptr->show();
    bigmsg_lbl.swap(ptr);
}


void Screen_Confirm::unprocessed_event(SDL_Event* ev)
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

void Screen_Confirm::sync_listbox()
{
    sorts = &scan_task->get_coffee_sorts();
    SortMap& smap = sorts->get_map();
    ListboxItem lbox_item = lbox->get_current();
    lbox->clear_data();
    for(SortMap::iterator it=smap.begin(); it!=smap.end(); it++)
	lbox->add_data(ListboxItem(it->first, it->second->name));
    lbox->add_data(ListboxItem("noop", "-  Выбор  -"));
    lbox->sort();
    lbox->redraw();
    lbox->set_cursor_by_item_idx(lbox_item.id);

}

void Screen_Confirm::goto_prev_screen()
{
    if(prev_screen)
    {
	hide();
	prev_screen->show();
    }
}

bool Screen_Confirm::idle_time_out_event()
{
    goto_prev_screen();
    return true;
}

bool Screen_Confirm::show()
{
    if(shown)
	return true;
    shown=true;

    RoastStorage *data=scan_task->get_storage();
    set_bigmsg(data->coffee_sort, false);

    inputsort_lbl = TextlabelPtr(new Textlabel(data->input_coffee_sort, game, "cfm_inputsort_lbl", imFont2));
    weight_lbl = TextlabelPtr(new Textlabel(data->wei_descr, game, "cfm_weight_lbl", imFont2));

    lbox->set_cursor_by_item_idx("noop");

    game->scene_add(this);
    game->add_object(this);
    Game::bg=game->image(imConfirmBG);
    GRect frect;
    frect.x=frect.y=0;
    frect.h=SCREEN_H; 
    frect.w=SCREEN_L;

    //    set_warn(std::string(scan_task->get_storage()->roaster) + ", введите параметры обжарки");
    set_info(std::string(scan_task->get_storage()->roaster) + ", подтвердите Ваш выбор!", false);

    game->redraw_area(frect);
    lbox->show();
    temp1_lbl->show();
    temp2_lbl->show();
    info_lbl->show();
    bigmsg_lbl->show();
    inputsort_lbl->show();
    weight_lbl->show();
    game->set_focus(lbox.get());

    game->set_idle_time_call(this, wEvent(aIdleTimeout), 300);

    DBG(5, "Screen_Confirm::show - show screen_confirm object: " << name);
    return true;
}

bool Screen_Confirm::hide()
{
    if(!shown)
	return true;
    game->scene_del(this);
    game->del_object(this);
    shown=false;
    lbox->hide();
    temp1_lbl->hide();
    temp2_lbl->hide();
    info_lbl->hide();
    bigmsg_lbl->hide();
    inputsort_lbl->hide();
    weight_lbl->hide();

    game->set_idle_time_call(0, wEvent(aIdleTimeout), 0);

    DBG(5, "Screen_Confirm::hide - hide screen_confirm object: " << name);
    return true;
}


Screen_Confirm::~Screen_Confirm()
{
}

bool Screen_Confirm::user_event(SDL_Event* ev)
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
	    DBG(4, "CONFIRMTemp: offset=" << last_offset << " temp1=" << row.get_int(t_input) << " raw=" << row.row[t_input])
	}
	break;
    default:
	break;
    }
    delete sev;
    return true;
}
