#ifndef	__NAMESERVICE_H
#define	__NAMESERVICE_H

#include <string>
#include <vector>
#include <iostream>

#include "debug.hxx"
#include "ncorba_header.hxx"

class NameServiceException
{
public:
    int ErrCode;
    std::string ServerMessage;
    std::string ToolMessage;

    NameServiceException( int ec = 0,
			  const char* sm = "",
			  const char* tm = "") :
	ErrCode( ec ), ServerMessage( sm ), ToolMessage( tm ) {}
};

class NameService
{
private:
    //CORBA::Object_var	            NSref;
    CORBA::ORB_var	            Orb;
    CosNaming::NamingContext_var    rootContext;
    CosNaming::NamingContext_var    currContext;

public:

   NameService( CORBA::ORB_ptr OrbPtr );
   ~NameService() {}

   void Bootstrap( const char* uri = 0 );

   inline void Bootstrap( const std::string& s ) { Bootstrap( s.c_str() ); }

   void Bind( CORBA::Object_ptr Obj_ptr, const char* Name_ptr );
   void Rebind( CORBA::Object_ptr Obj_ptr, const char* Name_ptr );

   void Unbind( const char* Name_ptr );

   inline void Bind( CORBA::Object_ptr Obj_ptr, const std::string& Name_str )
   { return Bind( Obj_ptr, Name_str.c_str() ); }

   inline void Rebind( CORBA::Object_ptr Obj_ptr, const std::string& Name_str )
   { return Rebind( Obj_ptr, Name_str.c_str() ); }

   inline void Unbind( const std::string& Name_str )
   { return Unbind( Name_str.c_str() ); }

   const CosNaming::NamingContext_ptr GetContext( const char* Name_ptr = 0,
						  bool createNew = false ) const
       throw ( NameServiceException );

   inline const CosNaming::NamingContext_ptr GetContext( const std::string& Name_str, bool createNew = false ) const
       throw ( NameServiceException )
   {
       return GetContext( Name_str.c_str(), createNew );
   }

   inline const CosNaming::NamingContext_ptr GetContextDup( const char* Name_ptr = 0, bool createNew = false ) const
       throw ( NameServiceException )
   {
       return CosNaming::NamingContext::_duplicate( GetContext(Name_ptr, createNew ) );
   }

   inline const CosNaming::NamingContext_ptr GetContextDup( const std::string& Name_str, bool createNew = false ) const
       throw ( NameServiceException )
   {
       return CosNaming::NamingContext::_duplicate( GetContext(Name_str.c_str(), createNew ) );
   }

   inline const CosNaming::NamingContext_ptr SetContext( const char* Name_ptr = 0,
							 bool createNew = false )
       throw ( NameServiceException )
   {
       if ( Name_ptr )
	   currContext = GetContext( Name_ptr, createNew );
       else
	   currContext = CosNaming::NamingContext::_duplicate( rootContext.in() );

       return currContext.in();
   }

   inline const CosNaming::NamingContext_ptr SetContext( const std::string& Name_str, bool createNew = false )
       throw ( NameServiceException )
   {
       currContext = GetContext( Name_str.c_str(), createNew );
       return currContext.in();
   }

   inline const CosNaming::NamingContext_ptr SetContext( const CosNaming::NamingContext_ptr new_ctx )
       throw ( NameServiceException )
   {
       if ( !currContext->_is_equivalent( new_ctx ) )
	   currContext = CosNaming::NamingContext::_duplicate( new_ctx );

       return currContext.in();
   }

   inline const CosNaming::NamingContext_ptr SetContextDup( const char* Name_ptr, bool createNew = false )
       throw ( NameServiceException )
   {
       return CosNaming::NamingContext::_duplicate( SetContext(Name_ptr, createNew ) );
   }

   inline const CosNaming::NamingContext_ptr SetContextDup( const std::string& Name_str, bool createNew = false )
       throw ( NameServiceException )
   {
       return CosNaming::NamingContext::_duplicate( SetContext(Name_str.c_str(), createNew ) );
   }

   static CosNaming::Name* String2Name( const std::string& NameString );
   static std::string Name2String( const CosNaming::Name& NameStruct );

   std::vector< std::string > GetBindingNames( const CosNaming::NamingContext_ptr& NContext );

   std::vector< std::string > RecursiveList( bool show_kind = true, int max_level = 10 );

   inline std::vector< std::string > List( bool show_kind = true )
   {
       return RecursiveList( show_kind, 0 );
   }

   template<class OBJTYPE> OBJTYPE* resolve_name( const char* nm )
   {
      DBG( 20, "Resolving: " << nm );
      CosNaming::Name_var cn = String2Name( nm );
      CORBA::Object_ptr obj = currContext->resolve( cn );
      return OBJTYPE::_narrow( obj );
   }

   template<class OBJTYPE> static OBJTYPE* resolve_name( const char* nm,
							 CosNaming::NamingContext_ptr nc )
   {
      DBG( 20, "Resolving: " << nm );
      CosNaming::Name_var cn = String2Name( nm );
      CORBA::Object_ptr obj = nc->resolve( cn );
      return OBJTYPE::_narrow( obj );
   }

   void Destroy( void );
};

extern std::ostream& operator<< ( std::ostream& s, const NameServiceException& nse );

#endif
