// docm_prefix(///)
/****************************************************************************
*  Copyright (C) 2002 by Leo Khramov
*  email:   leo@xnc.dubna.su
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
 ****************************************************************************/
// $Id: geometry.cxx,v 1.1 2009/09/08 19:07:40 leo Exp $

#include "globals.h"
#include "geometry.hxx"

//====================== Geometry settings and functions====================

//imNone means simple clear this area with background image :)

Geo      cursor1[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imCursor,  0,   0,   0,   0,  54,  54 },
  {    imNone,    1,  13,   2,   2,   0,   0 }
};

////////////// Params screen data //////////////////////

Geo    sort_listbox[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntroBG, 38,186,   0,   0, 375, 359 },
  {    imNone,    1,   5,   2,   2,   0,   0 }
};

Geo    full_sort_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntroBG, 38,583,  38, 583, 375,  24 },
  {    imNone,    1,  57,   3,  8,  255,  80 }
};

Geo    short_sort_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntroBG,126, 640,  38, 640, 287,  24 },
  {    imNone,    1, 57,   3,  8,  255,  80 }
};

Geo    isort_listbox[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntroBG,439,186,   0,   0, 390, 164 },
  {    imNone,    1,   5,   2,   2,   0,   0 }
};

Geo    istat_listbox[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntroBG,439,389,   0,   0, 390, 159 },
  {    imNone,    1,   5,   2,   2,   0,   0 }
};

Geo    sts_full_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntroBG,439,583,  38, 583, 390,  24 },
  {    imNone,    1,  57,   3,  8,  255,  80 }
};

Geo    sts_short_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntroBG,439, 640,  38, 640, 287,  24 },
  {    imNone,    1, 57,   3,  8,  255,  80 }
};


////////////// Params screen-2 data //////////////////////
Geo    lvl_listbox[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,265,259,   0,   0, 287, 251 },
  {    imNone,    1,   5,   2,   2,   0,   0 }
};

Geo    full_lvl_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,265,547,  38, 547, 287,  24 },
  {    imNone,    1,  57,   3,  8,  287,  80 }
};

Geo    short_lvl_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,265, 604,  38, 604, 287,  24 },
  {    imNone,    1, 57,   3,  8,  287,  80 }
};

Geo    wei_listbox[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,628,186,   0,   0, 188, 359 },
  {    imNone,    1,   5,   2,   2,   0,   0 }
};

Geo    full_wei_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,265,186,  38, 186, 188,  24 },
  {    imNone,    1,  57,   3,  8,  255,  80 }
};

Geo    short_wei_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,628, 640,  38, 640, 188,  24 },
  {    imNone,    1, 57,   3,  8,  255,  80 }
};


////////////// Finish screen data //////////////////////
Geo    finlvl_listbox[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,265,259,   0,   0, 287, 251 },
  {    imNone,    1,   5,   2,   2,   0,   0 }
};

Geo    finfull_lvl_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,265,547,  38, 547, 287,  24 },
  {    imNone,    1,  57,   3,  8,  287,  80 }
};

Geo    finshort_lvl_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,265, 604,  38, 604, 287,  24 },
  {    imNone,    1, 57,   3,  8,  287,  80 }
};

Geo    finwei_listbox[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,628,186,   0,   0, 188, 359 },
  {    imNone,    1,   5,   2,   2,   0,   0 }
};

Geo    finfull_wei_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,265,186,  38, 186, 188,  24 },
  {    imNone,    1,  57,   3,  8,  255,  80 }
};

Geo    finshort_wei_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntro2BG,628, 640,  38, 640, 188,  24 },
  {    imNone,    1, 57,   3,  8,  255,  80 }
};


//////////////// Login data /////////////////////////

Geo    login_listbox[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imLoginBG,258,186,   0,   0, 338, 360 },
  {    imNone,    1,   5,   2,   2,   0,   0 }
};

Geo    full_login_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imLoginBG,258,583, 256, 583, 338,  24 },
  {    imNone,    1,  57,   3,  8,  255,  80 }
};

Geo    short_login_name[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imLoginBG,258,640, 256, 640, 338,  24 },
  {    imNone,    1,  57,   3,  8,  255,  80 }
};

Geo    info_lbl[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG, 6, 738,   6,338,  650,  21 },
  {    imNone,    1,  60,   1,  8,  255,  60 }
};

Geo    bigmsg_lbl[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imIntroBG, 210, 650,   125, 650,  650,  35 },
  {    imNone,    1,  60,   1,  8,  255,  60 }
};

//////////////// Main roast screen data////////////////

Geo    main_uname[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,912,290, 912,290,  105,  18 },
  {    imNone,    1,  60,   3,  8,  255,  80 }
};

Geo    main_nroast[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,912,309, 912,309,  105,  18 },
  {    imNone,    1,  60,   3,  8,  255,  80 }
};

Geo    main_time[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,851,264, 851,264,  168,  22 },
  {    imNone,    1,  60,   3,  8,  255,  80 }
};

Geo    main_csort[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,882,328, 882,328,  135,  18 },
  {    imNone,    1,  60,   3,  8,  255,  80 }
};

Geo    main_lvl[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,900,347, 900,347,  117,  18 },
  {    imNone,    1,  60,   3,  8,  255,  80 }
};

Geo    main_wei[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,900,366, 900,366,  117,  18 },
  {    imNone,    1,  60,   3,  8,  255,  80 }
};

Geo    main_load[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,900,424, 900,424,  117,  18 },
  {    imNone,    1,  60,   3,  8,  255,  80 }
};

Geo    main_troast[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,851,485, 851,485,  168,  22 },
  {    imNone,    1,  60,   3,  8,  255,  80 }
};

Geo    main_unload[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,900,464, 900,464,  117,  18 },
  {    imNone,    1,  60,   3,  8,  255,  80 }
};

Geo    main_temp1[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,910, 23, 910, 23,  100,  48 },
  {    imDigits,   1,  6,   0,  0,   30,  48 }
};

Geo    main_temp2[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,910, 97, 910, 97,  100,  48 },
  {    imDigits,   1,  6,   0,  0,   30,  48 }
};

Geo    main_graph[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imBoardBG,  5,129,   5,129,  841, 594 },
  {    imNone,     1, 53,  57, 55,   40,  77 }
};

Geo    main_graphic_1[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imFont4,     1,  2,   0,  0,   40,  77 }
};

Geo    main_graphic_2[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imFont4,     1, 55,   0,  0,   40,  77 }
};

Geo    main_graphic_3[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imFont4,     4,  1,   0,  0,   40,  77 }
};


/////////////////// Confirm screen data ///////////////////
Geo    cfm_sort_listbox[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imConfirmBG, 227, 307,   0,   0, 375, 359 },
  {    imNone,    1,   5,   2,   2,   0,   0 }
};

Geo    cfm_bigmsg_lbl[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imConfirmBG, 227, 156,   227, 156,  650,  35 },
  {    imNone,    1,  60,   1,  8,  255,  60 }
};

Geo    cfm_inputsort_lbl[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imConfirmBG, 227, 202,   227, 222,  650,  35 },
  {    imNone,    1,  60,   1,  8,  255,  60 }
};


Geo    cfm_weight_lbl[]= {
  //   image  srcx srcy dstx dsty  len  hei
  {    imConfirmBG, 227, 232,   227, 252,  650,  35 },
  {    imNone,    1,  60,   1,  8,  255,  60 }
};





// Array of all objects geometry
GeoObject all_objects[]= {
  { "sort_listbox"            , sort_listbox},
  { "full_sort"               , full_sort_name},
  { "short_sort"              , short_sort_name},
  { "isort_listbox"           , isort_listbox},
  { "istat_listbox"           , istat_listbox},
  { "sts_full_name"           , sts_full_name},
  { "sts_short_name"          , sts_short_name},
  { "lvl_listbox"             , lvl_listbox},
  { "full_lvl"                , full_lvl_name},
  { "short_lvl"               , short_lvl_name},
  { "wei_listbox"             , wei_listbox},
  { "full_wei"                , full_wei_name},
  { "short_wei"               , short_wei_name},
  { "finish_listbox"          , finlvl_listbox},
  { "finish_full_lvl"         , finfull_lvl_name},
  { "finish_short_lvl"        , finshort_lvl_name},
  { "finish_wei_listbox"      , finwei_listbox},
  { "finish_full_wei"         , finfull_wei_name},
  { "finish_short_wei"        , finshort_wei_name},
  { "login_listbox"           , login_listbox},
  { "full_login"              , full_login_name},
  { "short_login"             , short_login_name},
  { "main_uname"              , main_uname},
  { "main_sortname"           , main_csort},
  { "main_time"               , main_time},
  { "main_temp1"              , main_temp1},
  { "main_temp2"              , main_temp2},
  { "main_lvl"                , main_lvl},
  { "main_wei"                , main_wei},
  { "main_load"               , main_load},
  { "main_troast"             , main_troast},
  { "main_unload"             , main_unload},
  { "main_nr"                 , main_nroast},
  { "main_graph"              , main_graph},
  { "main_graphic_1"          , main_graphic_1},
  { "main_graphic_2"          , main_graphic_2},
  { "main_graphic_3"          , main_graphic_3},
  { "bigmsg_lbl"              , bigmsg_lbl},
  { "info_lbl"                , info_lbl},
  { "cfm_sort_listbox"        , cfm_sort_listbox},
  { "cfm_bigmsg_lbl"          , cfm_bigmsg_lbl},
  { "cfm_inputsort_lbl"       , cfm_inputsort_lbl},
  { "cfm_weight_lbl"          , cfm_weight_lbl},
  { "empty",      0}
};



//===========================================================================
/// global get_geo_by_name(char* fname)
///     find geometry structures for object by given name
/// tags geometry
Geo* get_geo_by_name(const char* name)
{
  int i=0;
  while(all_objects[i].geo)
  {
    if(!strcmp(all_objects[i].name,name))
      return all_objects[i].geo;
    i++;
  }
  return 0;
}

// Adds mini_ before geo name
Geo* get_mini_geo_by_name(const char* name)
{
    char buf[GEO_NAME*2]="mini_";
    strcpy(buf+5, name);
    return get_geo_by_name(buf);
}
