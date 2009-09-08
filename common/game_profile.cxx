/***************************************************************************************************
 **
 ** profile.cpp
 **
 ** Real-Time Hierarchical Profiling for Game Programming Gems 3
 **
 ** by Greg Hjelstrom & Byon Garrabrant
 **
 ***************************************************************************************************/

#include "game_profile.h"
#include "commonfuncs.h"

#if 0 //Commented out because of platform dependent

inline void Profile_Get_Ticks(_int64 * ticks)
{
    __asm
        {
	    push edx;
	    push ecx;
	    mov ecx,ticks;
	    _emit 0Fh
                _emit 31h
                mov [ecx],eax;
	    mov [ecx+4],edx;
	    pop ecx;
	    pop edx;
        }
}

inline double Profile_Get_Tick_Rate(void)
{
    static double _CPUFrequency = -1.0f;
        
    if (_CPUFrequency == -1.0f) {
	__int64 curr_rate = 0;
	::QueryPerformanceFrequency ((LARGE_INTEGER *)&curr_rate);
	_CPUFrequency = (double)curr_rate;
    } 
        
    return _CPUFrequency;
}

#endif

/***************************************************************************************************
 **
 ** CProfileNode
 **
 ***************************************************************************************************/

/***********************************************************************************************
 * INPUT:                                                                                      *
 * name - pointer to a static string which is the name of this profile node                    *
 * parent - parent pointer                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The name is assumed to be a static pointer, only the pointer is stored and compared for     *
 * efficiency reasons.                                                                         *
 *=============================================================================================*/
CProfileNode::CProfileNode( const char * name, CProfileNode * parent ) :
    Name( name ),
    TotalCalls( 0 ),
    TotalTime( 0 ),
    StartTime(),
    RecursionCounter( 0 ),
    Parent( parent ),
    Child( 0 ),
    Sibling( 0 )
{
    Reset();
}


CProfileNode::~CProfileNode( void )
{
    delete Child;
    delete Sibling;
}


/***********************************************************************************************
 * INPUT:                                                                                      *
 * name - static string pointer to the name of the node we are searching for                   *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * All profile names are assumed to be static strings so this function uses pointer compares   *
 * to find the named node.                                                                     *
 *=============================================================================================*/
CProfileNode * CProfileNode::Get_Sub_Node( const char * name )
{
    // Try to find this sub node
    CProfileNode * child = Child;
    while ( child ) {
	if ( child->Name == name ) {
	    return child;
	}
	child = child->Sibling;
    }

    // We didn't find it, so add it
    CProfileNode * node = new CProfileNode( name, this );
    node->Sibling = Child;
    Child = node;
    return node;
}


void        CProfileNode::Reset( void )
{
    TotalCalls = 0;
    TotalTime = 0.0f;

    if ( Child ) {
	Child->Reset();
    }
    if ( Sibling ) {
	Sibling->Reset();
    }
}


void        CProfileNode::Call( void )
{
    TotalCalls++;
    if (RecursionCounter++ == 0) {
	StartTime.restart();
    }
}


bool        CProfileNode::Return( void )
{
    if ( --RecursionCounter == 0 && TotalCalls != 0 ) { 
	TotalTime += StartTime.elapsed();
    }
    return ( RecursionCounter == 0 );
}

void        CProfileNode::show_profile_info(int lvl)
{
    char buf[128]="";
    int  i;

    for(i=0;i<lvl;i++)
	buf[i]=' ';
    buf[i]=0;
    debprintf("%2d %s%-*s :  %2.3f sec in %5d calls (%3d recursion)\n",
	     lvl, buf,30-lvl, Name, TotalTime, TotalCalls, RecursionCounter);

}

/***************************************************************************************************
 **
 ** CProfileIterator
 **
 ***************************************************************************************************/
CProfileIterator::CProfileIterator( CProfileNode * start )
{
    CurrentParent = start;
    CurrentChild = CurrentParent->Get_Child();
}


void        CProfileIterator::First(void)
{
    CurrentChild = CurrentParent->Get_Child();
}


void        CProfileIterator::Next(void)
{
    CurrentChild = CurrentChild->Get_Sibling();
}


bool        CProfileIterator::Is_Done(void)
{
    return CurrentChild == 0;
}


void        CProfileIterator::Enter_Child( int index )
{
    CurrentChild = CurrentParent->Get_Child();
    while ( (CurrentChild != 0) && (index != 0) ) {
	index--;
	CurrentChild = CurrentChild->Get_Sibling();
    }

    if ( CurrentChild != 0 ) {
	CurrentParent = CurrentChild;
	CurrentChild = CurrentParent->Get_Child();
    }
}


void        CProfileIterator::Enter_Parent( void )
{
    if ( CurrentParent->Get_Parent() != 0 ) {
	CurrentParent = CurrentParent->Get_Parent();
    }
    CurrentChild = CurrentParent->Get_Child();
}


/***************************************************************************************************
 **
 ** CProfileManager
 **
 ***************************************************************************************************/

CProfileNode        CProfileManager::Root( "Root", 0 );
CProfileNode*       CProfileManager::CurrentNode = &CProfileManager::Root;
int                 CProfileManager::FrameCounter = 0;
boost::timer        CProfileManager::ResetTime;


/***********************************************************************************************
 * CProfileManager::Start_Profile -- Begin a named profile                                    *
 *                                                                                             *
 * Steps one level deeper into the tree, if a child already exists with the specified name     *
 * then it accumulates the profiling; otherwise a new child node is added to the profile tree. *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - name of this profiling record                                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The string used is assumed to be a static string; pointer compares are used throughout      *
 * the profiling code for efficiency.                                                          *
 *=============================================================================================*/
void        CProfileManager::Start_Profile( const char * name )
{
    if (name != CurrentNode->Get_Name()) {
	CurrentNode = CurrentNode->Get_Sub_Node( name );
    } 
        
    CurrentNode->Call();
}


/***********************************************************************************************
 * CProfileManager::Stop_Profile -- Stop timing and record the results.                       *
 *=============================================================================================*/
void        CProfileManager::Stop_Profile( void )
{
    // Return will indicate whether we should back up to our parent (we may
    // be profiling a recursive function)
    if (CurrentNode->Return()) {
	CurrentNode = CurrentNode->Get_Parent();
    }
}


/***********************************************************************************************
 * CProfileManager::Reset -- Reset the contents of the profiling system                       *
 *                                                                                             *
 *    This resets everything except for the tree structure.  All of the timing data is reset.  *
 *=============================================================================================*/
void        CProfileManager::Reset( void )
{ 
    Root.Reset(); 
    FrameCounter = 0;
    ResetTime.restart();
}


/***********************************************************************************************
 * CProfileManager::Increment_Frame_Counter -- Increment the frame counter                    *
 *=============================================================================================*/
void CProfileManager::Increment_Frame_Counter( void )
{
    FrameCounter++;
}


/***********************************************************************************************
 * CProfileManager::Get_Time_Since_Reset -- returns the elapsed time since last reset         *
 *=============================================================================================*/
double CProfileManager::Get_Time_Since_Reset( void )
{
    return ResetTime.elapsed();
}


void CProfileManager::Show_All_Profile()
{
#ifndef DEBUG_XNC
    return;
#endif
    debprintf("==========================Profile results=======================\n");
    debprintf("Total time:    %2.3f\n", Get_Time_Since_Reset());
    debprintf("Total frames:  %d\n",    Get_Frame_Count_Since_Reset());
    debprintf("FPS:           %2.3f\n", double(Get_Frame_Count_Since_Reset())/Get_Time_Since_Reset());
    debprintf("Clock prec:    %d\n",    CLOCKS_PER_SEC);
    show_profile_tree(Root.Get_Child(), 0);
    
    debprintf("================================================================\n");
}

void CProfileManager::show_profile_tree(CProfileNode* pnode, int lvl)
{
    while(pnode)
    {
	pnode->show_profile_info(lvl);
	show_profile_tree(pnode->Get_Child(), lvl+1);
	pnode=pnode->Get_Sibling();
    }
}
