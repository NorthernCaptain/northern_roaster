#ifndef BOUNDED_QUEUE_HXX
#define BOUNDED_QUEUE_HXX

#include "ace/Mutex.h"
#include "ace/Condition_T.h"
#include <list>

template <typename TYPE, typename LOCK_TYPE> class MTBoundedQueue
{
    std::list<TYPE>            storage;
    LOCK_TYPE                  mtx;
    ACE_Condition<LOCK_TYPE>   pushcond;
    //    ACE_Condition<LOCK_TYPE>   popcond;
    size_t                     max_queue_size;
    size_t                     n_elements;
public:
    MTBoundedQueue<TYPE, LOCK_TYPE> (size_t max_siz=100): pushcond(mtx)
	//, popcond(mtx) 
    {
	max_queue_size=max_siz;
	n_elements=0;
    }

    size_t                size() 
    {
	ACE_Guard<LOCK_TYPE>      guard(mtx);
	return n_elements;
    }

    void                  push(TYPE val)
    {
	ACE_Guard<LOCK_TYPE>      guard(mtx);
	//	if(n_elements+1 >= max_queue_size)
	//	    popcond.wait();
	storage.push_back(val);
	n_elements++;
	pushcond.signal();
    }

    void                  pop(TYPE& val)
    {
	ACE_Guard<LOCK_TYPE>      guard(mtx);
	if(n_elements==0)
	    pushcond.wait();
	val=storage.front();
	n_elements--;
	storage.pop_front();
	//	popcond.signal();
    }
};

#endif
