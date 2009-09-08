//docm_prefix(///)

#include "debug.hxx"
#include "timing.hxx"

#include "boost/format.hpp"

//-----------------------------------------------------------------------
/// module description
/// Модуль реализует класс для проведения временных замеров

using std::string;

//-----------------------------------------------------------------------
/// global Timing::Timing
/// Начальная инициализация интерфейса
/// tags Timing
Timing::Timing(const char* iname): name(iname)
{
    start_time=get_double_time();
    DBG(4, "==========================================[" << name.c_str() << ']');
}


//-----------------------------------------------------------------------
/// global Timing::~Timing
/// Деструктор  - результаты по замеру времени
/// tags Timing
Timing::~Timing()
{
    end_time=get_double_time();
    run_time=end_time - start_time;
    DBG(4, boost::format("==========================================(%s: %7.6f sec)")
	% name.c_str() % run_time);
}

double Timing::get_double_time()
{
    struct timeval tv;
    struct timezone dummy;
    gettimeofday(&tv, &dummy);
    return double(tv.tv_sec) + (double)(tv.tv_usec)/ 1000000.0;
}

// ------------------------------[EOF]-------------------------------------
