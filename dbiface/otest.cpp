#include <string>
#include "DBSObjects.h"
#include "otl_connect_wrapper.h"

int main(int argc, char** argv)
{
    if(argc<2)
    {
	DBG(-1, "Usage: otest user/pwd@DSN");
	return 1;
    }

    DL.debug_level(100);
    new_otl_connect_wrapper_f = new_otl_connect_wrapper;
    otl_wrapper* ow = new_otl_wrapper();

    ow->otl_initialize();

    try
    {

	DBConnectionHome*   dbhome = new DBConnectionHome(std::string(argv[1]));
	otl_connect_ptr con = dbhome->getDBConnection();

	otl_stream_wrapper_ptr str(con->new_stream(50, "select name, full_name from worker where disabled=0 and type='roaster' and id >= :id<int>"));

	(*str) << 0;

	while(!str->eof())
	{
	    std::string id;
	    std::string name;
	    (*str) >> id >> name;
	    DBG(0, "NAME, FULL_NAME: " << id << " : " << name);
	}

    }
    catch(DBQueryException& ex)
    {
	DBG(-1, "Got DBQueryException:");
	DBG(-1, ex.getLocalizedMessage(""));
    } 

    ow->otl_terminate();	

    return 0;
}
