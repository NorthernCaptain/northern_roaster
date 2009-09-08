#----------------------------------------------------------------------------
#       Local macros
#----------------------------------------------------------------------------
# Oracle8 by default
DB_TYPE?=ODBC 

ifeq "$(DB_TYPE)" "ORACLE8"
  OTL_DB_CPPFLAGS = -DOTL_ORA9I -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public
  OTL_LIBS = -L$(ORACLE_HOME)/lib -lclntsh
else
  ifeq "$(DB_TYPE)" "FREETDS"
    OTL_DB_CPPFLAGS = -DOTL_ODBC_FREETDS
  else
    ifeq "$(DB_TYPE)" "ODBC"
      OTL_DB_CPPFLAGS = -DOTL_ODBC -DOTL_ODBC_UNIX -DOPENLINK_UTF_FIX
    else
      $(warning Please, choose data base type by defining DB_TYPE environment variable)
      $(warning DB_TYPE=$(DB_TYPE) not supported)
      $(error Only ORACLE8, FREETDS or ODBC is supported)
    endif
  endif
    OTL_LIBS = -lodbc
endif

OTL_CPPFLAGS = $(OTL_DB_CPPFLAGS) -DOTL_STL -DOTL_UTF8_MAX_BYTES=2 -I./
