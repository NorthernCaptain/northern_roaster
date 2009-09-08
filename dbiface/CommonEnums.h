// -*- C++ -*-

#ifndef __COMMONENUMS_H_
#define __COMMONENUMS_H_

/** Перечисление для типов данных со стороны базы*/
enum DataType
{
    NUMBER_VAL,
    DOUBLE_VAL,
    CHAR_VAL,
    STRING_VAL,
    NULL_VAL,
    DATETIME_VAL,
    TF_BOOL_VAL
};

/** Перечисление типов БД*/
enum DB_TYPE
{
    ORACLE,
    MSSQL,
    POSTGRES
};

#endif
