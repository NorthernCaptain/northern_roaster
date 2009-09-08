#include "game.hxx"
#include "screen_main.hxx"
#include "exceptions.hxx"
#include "debug.hxx"
#include "textlabelflash.hxx"
#include "commonfuncs.h"

#include "boost/lexical_cast.hpp"

Screen_Main::Screen_Main(ScanTaskPtr& scan, Game* _game, const char* _name) : Screen_Base(_game, _name), scan_task(scan)
{
    uname_lbl=TextlabelPtr(new Textlabel("", game, "main_uname", imFont3));
    uname_lbl->justify_mode(JustRight);
    sortname_lbl=TextlabelPtr(new Textlabel("", game, "main_sortname", imFont3));
    sortname_lbl->justify_mode(JustRight);
    time_lbl=TextlabelPtr(new Textlabel("", game, "main_time", imFont2));
    time_lbl->justify_mode(JustCenter);
    level_lbl=TextlabelPtr(new Textlabel("", game, "main_lvl", imFont3));
    level_lbl->justify_mode(JustRight);
    weight_lbl=TextlabelPtr(new Textlabel("", game, "main_wei", imFont3));
    weight_lbl->justify_mode(JustRight);
    nroast_lbl=TextlabelPtr(new Textlabel("", game, "main_nr", imFont3));
    nroast_lbl->justify_mode(JustRight);
    load_lbl=TextlabelPtr(new Textlabel("", game, "main_load", imFont3));
    load_lbl->justify_mode(JustRight);
    unload_lbl=TextlabelPtr(new Textlabel("", game, "main_unload", imFont3));
    unload_lbl->justify_mode(JustRight);
    troast_lbl=TextlabelPtr(new Textlabel("", game, "main_troast", imFont2));
    troast_lbl->justify_mode(JustCenter);
    temp1_lbl=DigitlabelPtr(new Digitlabel(0, game, "main_temp1"));
    temp2_lbl=DigitlabelPtr(new Digitlabel(10, game, "main_temp2"));
    graph=GraphviewPtr(new Graphview(scan, game, "main_graph", imFont4));
    graph->escape_signal.connect(SigC::slot(*this, &Screen_Main::process_escape));
    graph->unprocessed_signal.connect(SigC::slot(*this, &Screen_Main::unprocessed_event));
    data_offset=0;
}

void Screen_Main::unprocessed_event(SDL_Event* ev)
{
    if(ev->type!=SDL_KEYDOWN)
	return;
    switch(ev->key.keysym.sym)
    {
    case SDLK_F4:
	process_escape();
	break;
    case SDLK_F1:
	scan_task->start_loading();
	break;
    case SDLK_F6:
	scan_task->start_unloading();
	break;
    default:
	break;
    }
}

void Screen_Main::process_escape()
{
    RoastStorage*  store=scan_task->get_storage();
    bool roasting = false;
    time_t r_start, s_start;
    {
	ACE_Guard<ACE_Thread_Mutex>  guard(scan_task->get_lock());
	r_start = store->roast_start;
	s_start = store->session_start;
    }
    
    //check conditions for roasting
    if(r_start) //if roasting is started the ok
	roasting = true;
    else
	if(s_start==0)   //if our session is not started then no roast
	    roasting = false;
	else
	{
	    time_t now_t;
	    ACE_OS::time(&now_t);
	    if(now_t - s_start > 60*5)
		roasting = true;
	}



    DBG(5, "ScreenMain::process_escape - exiting to prev screen");

    if(roasting)
    {
	hide_only();
	finished_signal.emit();
    }
    else
    {
	if(prev_screen)
	{
	    hide();
	    prev_screen->show();
	}
	else
	{
	    hide_only();
	    finished_signal.emit();
	}
    }
}

bool Screen_Main::show()
{
    if(shown)
	return true;
    shown=true;

    game->scene_add(this);
    game->add_object(this);
    Game::bg=game->image(imBoardBG);
    GRect frect;
    frect.x=frect.y=0;
    frect.h=SCREEN_H; 
    frect.w=SCREEN_L;

    game->redraw_area(frect);

    {
	RoastStorage*  store=scan_task->get_storage();
	ROAST_LOCK(store);

	uname_lbl->set_text(store->roaster);
	sortname_lbl->set_text(store->coffee_sort);
    }

    uname_lbl->show();
    sortname_lbl->show();
    time_lbl->show();
    level_lbl->show();
    weight_lbl->show();
    nroast_lbl->show();
    load_lbl->show();
    unload_lbl->show();
    troast_lbl->show();
    temp1_lbl->show();
    temp2_lbl->show();
    graph->show();
    info_lbl->show();

    graph->my_focus();

    scan_task->send(new ScanTaskEvent(stask_login));

    DBG(5, "Screen_Main::show - show screen_main object: " << name);
    return true;
}

bool Screen_Main::hide_only()
{
    if(!shown)
        return true;
    shown=false;

    game->scene_del(this);
    game->del_object(this);
    uname_lbl->hide();
    sortname_lbl->hide();
    time_lbl->hide();
    level_lbl->hide();
    weight_lbl->hide();
    nroast_lbl->hide();
    load_lbl->hide();
    unload_lbl->hide();
    troast_lbl->hide();
    temp1_lbl->hide();
    temp2_lbl->hide();
    graph->hide();
    info_lbl->hide();
    DBG(5, "Screen_Main::hide_only - hide screen_main object: " << name);
    return true;
}

bool Screen_Main::hide()
{
    if(!shown)
	return true;
    hide_only();

    {
	RoastStorage*  store=scan_task->get_storage();
	ROAST_LOCK(store);
	bool roasting = false;
	roasting = store->roast_start!=0;

	if(!roasting)
	    scan_task->send(new ScanTaskEvent(stask_logout));
    }
    DBG(5, "Screen_Main::hide - hide screen_main object: " << name);
    return true;
}

Screen_Main::~Screen_Main()
{
}

bool Screen_Main::process_any_event(wEvent ev)
{
    bool ret = Screen_Base::process_any_event(ev);
    switch(ev.type)
    {
    case aAutoExit:
	process_escape();
	return false;
    default:
	break;
    }
    return ret;
}

bool Screen_Main::user_event(SDL_Event* ev)
{
    if(ev->user.code!=1)
	return false;
    ScreenEvent*  sev = (ScreenEvent*)ev->user.data1;

    switch(sev->type)
    {
    case se_update_info:
	{
	    {
		RoastStorage*  store=scan_task->get_storage();
		ROAST_LOCK(store);

		DBG(5, "Screen_Main::user_event - process update_info event for object: " << name);
		uname_lbl->set_text(store->roaster);
		sortname_lbl->set_text(store->coffee_sort);
		level_lbl->set_text(store->roast_lvl);
		weight_lbl->set_text(store->wei_descr);
		nroast_lbl->set_text(boost::lexical_cast<std::string>(store->local_roast_id+1).c_str());
	    }
	    graph->redraw();
	}
	break;
    case se_update_grid:
	update_grid_data();
	break;
    case se_init_grid:
	reinit_grid_data();
	break;
    case se_auto_exit:
	//we need to call this only after all processing is done
	game->set_post_event_call(this, wEvent(aAutoExit));
	break;
    default:
	break;
    }
    delete sev;
    return true;
}

void Screen_Main::update_grid_data()
{
    {
	RoastStorage*  store=scan_task->get_storage();
	ROAST_LOCK(store);
	
	int last_offset=store->total_rows-1;
	if(last_offset==data_offset) //nothing changed
	    return;
	
	TempRow& row=(*store)[last_offset];
	temp1_lbl->set_value(row.get_int(t_input));
	temp2_lbl->set_value(row.get_int(t_output));
	
	{
	    char buf[128];
	    time_t now_t;
	    curdate2str(buf);
	    time_lbl->set_text(buf);
	    
	    time_t rload = store->roast_start;
	    if(rload)
	    {
		time2str(rload, buf);
		load_lbl->set_text(buf);
	    } else
		load_lbl->set_text("Нет");
	    
	    ACE_OS::time(&now_t);
	    
	    time_t runload = store->roast_end;
	    
	    if(rload)
	    {
		time_t total_t = abs((runload == 0 ? now_t : 
				      ((runload < now_t && runload > rload) ?  runload : now_t)) - rload);
		difftime2str(total_t, buf);
		troast_lbl->set_text(buf);
	    }
	    else
		troast_lbl->set_text("Нет");
	    
	    if(runload)
	    {
		time2str(runload, buf);
		unload_lbl->set_text(buf);
	    } else
		unload_lbl->set_text("Нет");	
	}
    }

    graph->redraw();
}

void Screen_Main::reinit_grid_data()
{
    {
	RoastStorage*  store=scan_task->get_storage();
	ROAST_LOCK(store);

	int last_offset=store->total_rows-1;
	TempRow& row=(*store)[last_offset];
	temp1_lbl->set_value(row.get_int(t_input));
	temp2_lbl->set_value(row.get_int(t_output));
	
	{
	    char buf[128];
	    curdate2str(buf);
	    time_lbl->set_text(buf);
	}
    }
    graph->full_redraw();
}
