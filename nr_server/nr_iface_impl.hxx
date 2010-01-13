#ifndef NR_IFACE_IMPL_HXX
#define NR_IFACE_IMPL_HXX

#include "ncorba_header.hxx"
#include "nr_ifaceS.h"
#include "coffeesorts.hxx"

namespace POA_NR_iface
{
    class RoastCom_Impl: public virtual RoastCom,
			 public virtual PortableServer::RefCountServantBase
    {
    public:
	RoastCom_Impl();
	~RoastCom_Impl();

	CORBA::Boolean        get_data_block(const NR_iface::DataBlockRequest& req,
					     NR_iface::DataBlock_out data) throw (CORBA::SystemException);
	CORBA::Long           roaster_login(const char* dev_name,
					    const NR_iface::DataHeader& hdr) throw (CORBA::SystemException);
	CORBA::Boolean        roaster_logout(const char* dev_name,
					     CORBA::Long user_id) throw (CORBA::SystemException);
	CORBA::Boolean        roaster_finish_roast(const char* dev_name,
						   CORBA::Long user_id,
						   const char* wei_descr,
						   CORBA::Long weight,
						   const char* state_descr,
						   const char* state_short_name) throw (CORBA::SystemException);
	CORBA::Boolean        roaster_start_loading(const char* dev_name) throw (CORBA::SystemException);
	CORBA::Boolean        roaster_start_unloading(const char* dev_name) throw (CORBA::SystemException);
	CORBA::Boolean        roaster_check_rcn(const char* inp_sort_name, const char* inp_short_lvl, CORBA::Long rcn_id)  throw (CORBA::SystemException);
    };


    class RoastCoffeeSort_Impl: public virtual RoastCoffeeSort,
				public virtual PortableServer::RefCountServantBase
    {
	CoSorts               sorts;
    public:
	RoastCoffeeSort_Impl();
	~RoastCoffeeSort_Impl();
	
	CORBA::Long           get_coffee_sort_list(NR_iface::Sorts_out lst)
	    throw (CORBA::SystemException);
	CORBA::Long           get_input_coffee_sort_list(const char* sort_short_name, NR_iface::Sorts_out lst)
	    throw (CORBA::SystemException);
	CORBA::Boolean        set_coffee_sort_list(const NR_iface::Sorts& lst)
	    throw (CORBA::SystemException);
    };

    class RoastCoffeeSort_ImplDB: public virtual RoastCoffeeSort,
				public virtual PortableServer::RefCountServantBase
    {
    public:
	RoastCoffeeSort_ImplDB();
	~RoastCoffeeSort_ImplDB();
	
	CORBA::Long           get_coffee_sort_list(NR_iface::Sorts_out lst)
	    throw (CORBA::SystemException);
	CORBA::Long           get_input_coffee_sort_list(const char* sort_short_name, NR_iface::Sorts_out lst)
	    throw (CORBA::SystemException);
	CORBA::Boolean        set_coffee_sort_list(const NR_iface::Sorts& lst)
	    throw (CORBA::SystemException);
    };

    class RoastUsers_Impl: public virtual RoastUsers,
			   public virtual PortableServer::RefCountServantBase
    {
        CoSorts               users;
    public:
	RoastUsers_Impl();
	~RoastUsers_Impl();
	
	CORBA::Long           get_users_list(NR_iface::Users_out lst)
	    throw (CORBA::SystemException);
	CORBA::Boolean        set_users_list(const NR_iface::Users& lst)
	    throw (CORBA::SystemException);
    };

    class RoastUsers_ImplDB: public virtual RoastUsers,
			     public virtual PortableServer::RefCountServantBase
    {
    public:
	RoastUsers_ImplDB();
	~RoastUsers_ImplDB();
	
	CORBA::Long           get_users_list(NR_iface::Users_out lst)
	    throw (CORBA::SystemException);
	CORBA::Boolean        set_users_list(const NR_iface::Users& lst)
	    throw (CORBA::SystemException);
    };

    class RoastLevels_Impl: public virtual RoastLevels,
			    public virtual PortableServer::RefCountServantBase
    {
        CoSorts               levels;
    public:
	RoastLevels_Impl();
	~RoastLevels_Impl();
	
	CORBA::Long           get_lvl_list(NR_iface::Levels_out lst)
	    throw (CORBA::SystemException);
	CORBA::Boolean        set_lvl_list(const NR_iface::Levels& lst)
	    throw (CORBA::SystemException);
    };

    class RoastLevels_ImplDB: public virtual RoastLevels,
			    public virtual PortableServer::RefCountServantBase
    {
    public:
	RoastLevels_ImplDB();
	~RoastLevels_ImplDB();
	
	CORBA::Long           get_lvl_list(NR_iface::Levels_out lst)
	    throw (CORBA::SystemException);
	CORBA::Boolean        set_lvl_list(const NR_iface::Levels& lst)
	    throw (CORBA::SystemException);
    };

    class RoastWeights_Impl: public virtual RoastWeights,
			     public virtual PortableServer::RefCountServantBase
    {
        CoSorts               weights;
    public:
	RoastWeights_Impl();
	~RoastWeights_Impl();
	
	CORBA::Long           get_weight_list(NR_iface::Weights_out lst)
	    throw (CORBA::SystemException);
	CORBA::Boolean        set_weight_list(const NR_iface::Weights& lst)
	    throw (CORBA::SystemException);
    };

    class RoastStates_ImplDB: public virtual RoastStates,
			    public virtual PortableServer::RefCountServantBase
    {
    public:
	RoastStates_ImplDB();
	~RoastStates_ImplDB();
	
	CORBA::Long           get_state_list(NR_iface::States_out lst)
	    throw (CORBA::SystemException);
	CORBA::Boolean        set_state_list(const NR_iface::States& lst)
	    throw (CORBA::SystemException);
    };

};
#endif
// ------------------------------[EOF]-------------------------------------
