/***************************************************************************************************
 **
 ** profile.h
 **
 ** Real-Time Hierarchical Profiling for Game Programming Gems 3
 **
 ** by Greg Hjelstrom & Byon Garrabrant
 **
 ***************************************************************************************************/

#include <boost/timer.hpp>

/*
** A node in the Profile Hierarchy Tree
*/
class        CProfileNode {

public:
    CProfileNode( const char * name, CProfileNode * parent );
    ~CProfileNode( void );

    CProfileNode*       Get_Sub_Node( const char * name );

    inline CProfileNode*       Get_Parent( void )               { return Parent; }
    inline CProfileNode*       Get_Sibling( void )              { return Sibling; }
    inline CProfileNode*       Get_Child( void )                { return Child; }

    void                Reset( void );
    void                Call( void );
    bool                Return( void );

    inline const char *        Get_Name( void )                 { return Name; }
    inline int                 Get_Total_Calls( void )          { return TotalCalls; }
    inline double              Get_Total_Time( void )           { return TotalTime; }

    void                       show_profile_info(int lvl);

protected:

    const char *          Name;
    int                   TotalCalls;
    double                TotalTime;
    boost::timer          StartTime;
    int                   RecursionCounter;

    CProfileNode *        Parent;
    CProfileNode *        Child;
    CProfileNode *        Sibling;
};

/*
** An iterator to navigate through the tree
*/
class CProfileIterator
{
public:
    // Access all the children of the current parent
    void                                First(void);
    void                                Next(void);
    bool                                Is_Done(void);

    void                                Enter_Child( int index );                // Make the given child the new parent
    void                                Enter_Largest_Child( void );        // Make the largest child the new parent
    void                                Enter_Parent( void );                        // Make the current parent's parent the new parent

    // Access the current child
    inline const char *        Get_Current_Name( void )                        { return CurrentChild->Get_Name(); }
    inline int                 Get_Current_Total_Calls( void )        { return CurrentChild->Get_Total_Calls(); }
    inline double              Get_Current_Total_Time( void )        { return CurrentChild->Get_Total_Time(); }

    // Access the current parent
    inline const char *        Get_Current_Parent_Name( void )                        { return CurrentParent->Get_Name(); }
    inline int                 Get_Current_Parent_Total_Calls( void )        { return CurrentParent->Get_Total_Calls(); }
    inline double              Get_Current_Parent_Total_Time( void )        { return CurrentParent->Get_Total_Time(); }

protected:

    CProfileNode *        CurrentParent;
    CProfileNode *        CurrentChild;

    CProfileIterator( CProfileNode * start );
    friend        class                CProfileManager;
};


/*
** The Manager for the Profile system
*/
class        CProfileManager {
public:
    static        void        Start_Profile( const char * name );
    static        void        Stop_Profile( void );

    static        void        Reset( void );
    static        void        Increment_Frame_Counter( void );
    static        int         Get_Frame_Count_Since_Reset( void )                { return FrameCounter; }
    static        double      Get_Time_Since_Reset( void );

    static        CProfileIterator *        Get_Iterator( void )        { return new CProfileIterator( &Root ); }
    static        void        Release_Iterator( CProfileIterator * iterator ) { delete iterator; }
    static        void        Show_All_Profile();

private:
    static        CProfileNode               Root;
    static        CProfileNode *             CurrentNode;
    static        int                        FrameCounter;
    static        boost::timer               ResetTime;
    static        void        show_profile_tree(CProfileNode* pnode, int lvl);
};


/*
** ProfileSampleClass is a simple way to profile a function's scope
** Use the PROFILE macro at the start of scope to time
*/
class        CProfileSample {
public:
    CProfileSample( const char * name )
    { 
	CProfileManager::Start_Profile( name ); 
    }
        
    ~CProfileSample( void )                                        
    { 
	CProfileManager::Stop_Profile(); 
    }
};

#ifdef DEBUG_XNC
#define        PROFILE( name )                        CProfileSample __profile( name )
#else
#define        PROFILE( name )
#endif


