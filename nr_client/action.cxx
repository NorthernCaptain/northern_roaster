#include "action.hxx"

SimpleAction::SimpleAction(Game* _game, const char* _name)
{
    geo=get_geo_by_name((char*)_name);
    auto_delete=false;
}

//////////////////////////////ActionQueue////////////////////////////////////

ActionQueue::ActionQueue()
{
    current_act=0;
}

bool ActionQueue::add(SimpleAction* act)
{
    bool empty=que.empty();
    que.push_back(act);
    act->set_action_queue(this);
    activate();
    return !empty;
}

bool ActionQueue::add_only(SimpleAction* act)
{
    ActionList::iterator it=que.begin();
    it++;
    que.erase(it, que.end());
    return add(act);
}

bool ActionQueue::replace_current(SimpleAction* act)
{
    bool empty=que.empty();

    current_act=act;
    if(!empty)
	que.pop_front();
    que.push_front(act);
    return !empty;
}

bool ActionQueue::clear_all()
{
    bool empty=que.empty();
    current_act=0;
    if(!empty)
	que.erase(que.begin(), que.end());
    return !empty;
}

bool ActionQueue::next_action()
{
    stop_current();
    if(que.empty())
	return false;
    activate();
    return true;
}

void ActionQueue::activate()
{
    if(!que.empty() && current_act==0)
    {
	current_act=que.front();
	current_act->start();
    }
}

bool ActionQueue::stop_current()
{
    if(current_act)
    {
	current_act->stop();
	if(current_act->need_auto_delete())
	    delete current_act;
	current_act=0;
	que.pop_front();
	return true;
    }
    return false;
}

