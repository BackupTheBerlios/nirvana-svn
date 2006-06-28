/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2003 Lars Knoll (knoll@kde.org)
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* This file is mostly data generated by flex. Unfortunately flex
   can't handle 16bit strings directly, so we just copy the part of
   the code we need and modify it to our needs.

   Most of the defines below are to make sure we can easily use the
   flex generated code, using as little editing as possible.

   The flex syntax to generate the lexer are more or less directly
   copied from the CSS2.1 specs, with some fixes for comments and
   the important symbol.

   To regenerate, run flex on tokenizer.flex. After this, copy the
   data tables and the YY_DECL method over to this file. Remove the
   init code from YY_DECL and change the YY_END_OF_BUFFER to only call
   yyterminate().

*/

// --------- begin generated code -------------------

#define YY_NUM_RULES 47
#define YY_END_OF_BUFFER 48
static yyconst short int yy_accept[308] =
    {   0,
        0,    0,   48,   46,    2,    2,   46,   46,   46,   46,
       46,   46,   46,   46,   46,   40,   46,   46,   46,   46,
       11,   11,   46,   46,    2,    0,    0,    0,   10,    0,
       12,    0,    8,    0,    0,    9,    0,    0,   11,   40,
        0,   39,    0,    0,   40,    0,   38,   38,   38,   38,
       38,   38,   38,   38,   38,   38,   38,   38,   35,    0,
        0,    0,    0,    0,    0,    0,   11,   11,    7,   43,
       11,    0,    0,   11,    6,    5,    0,    0,    0,   10,
        0,    0,   12,   12,    0,    0,   10,    0,    0,    4,
        0,    0,   38,   38,   38,    0,   38,   26,   38,   22,

       24,   38,   36,   28,   38,   27,   34,   30,   29,   25,
       38,    0,    0,    0,    0,    0,    0,    0,   11,   11,
       11,   44,   44,   11,    0,    0,    0,   12,    0,    0,
        0,    1,   38,   38,   38,   38,   31,   38,   37,   32,
        3,    0,    0,    0,    0,    0,    0,   11,   11,    0,
       44,   44,   44,   43,    0,    0,   12,    0,    0,    0,
       38,   38,   38,   33,    0,    0,    0,    0,    0,   14,
       11,   11,   45,   44,   44,   44,   44,    0,    0,    0,
        0,   42,    0,    0,    0,   12,    0,   38,   38,   23,
        0,    0,    0,    0,   15,   11,   11,   45,   44,   44,

       44,   44,   44,    0,    0,    0,    0,    0,    0,    0,
        0,    0,   42,    0,    0,    0,    0,   12,    0,   38,
       38,    0,    0,    0,   13,   11,   11,   45,   44,   44,
       44,   44,   44,   44,    0,   41,    0,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,   12,
        0,   38,   38,    0,   17,    0,   11,   45,   44,   44,
       44,   44,   44,   44,   44,    0,   41,    0,    0,    0,
       41,    0,    0,    0,    0,   38,    0,    0,    0,    0,
       45,    0,    0,    0,   21,    0,    0,    0,   16,   45,
        0,    0,    0,    0,    0,    0,    0,    0,    0,    0,

       18,    0,    0,    0,   19,   20,    0
    } ;

static yyconst int yy_ec[256] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    2,    3,
        1,    4,    5,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    6,    7,    8,    9,   10,   11,   12,   13,   14,
       15,   16,   17,   12,   18,   19,   20,   21,   21,   21,
       21,   21,   21,   21,   21,   21,   21,   12,   12,   22,
       23,   24,   25,   26,   30,   31,   32,   33,   34,   35,
       36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
       46,   47,   48,   49,   50,   51,   39,   52,   39,   53,
       12,   27,   12,   28,   29,   12,   30,   31,   32,   33,

       34,   35,   36,   37,   38,   39,   40,   41,   42,   43,
       44,   45,   46,   47,   48,   49,   50,   51,   39,   52,
       39,   53,   12,   54,   12,   55,    1,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,

       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56,   56,   56,   56,   56,   56,
       56,   56,   56,   56,   56
    } ;

static yyconst int yy_meta[57] =
    {   0,
        1,    2,    3,    3,    3,    4,    4,    4,    4,    4,
        4,    4,    4,    5,    4,    4,    4,    6,    4,    4,
        7,    4,    4,    4,    8,    4,    9,    4,    9,   10,
       10,   10,   10,   10,   10,    9,    9,    9,    9,    9,
        9,    9,    9,    9,    9,    9,    9,    9,    9,    9,
        9,    9,    9,    4,    4,    9
    } ;

static yyconst short int yy_base[336] =
    {   0,
        0,    0,  744, 1642,   55,   60,   65,   64,  708,  711,
       60,  710,   56,  711,  715,   93,  716,   57,   75,  698,
       63,   71,  697,  696,   76,  148,  676,   86, 1642,  185,
      677,  126, 1642,   73,  219, 1642,  679,  134,   70,   96,
      686,  690,  673,  678,    0,  141,   92,  671,   74,  136,
      150,  151,  152,  153,  154,  155,  177,  180,  670,  665,
      642,  642,  634,  635,  642,  632,  181,  253, 1642, 1642,
      186,  204,  268,  187, 1642, 1642,  616,  239,  203,  233,
      302,  336,  614,  370,  265,  172,  218,  404,  438, 1642,
      624,  228,  610,  472,  609,  321,  216,  608,  234,  607,

      598,  242,  596,  595,  237,  594,  593,  579,  578,  564,
      200,  564,  544,  550,  536,  533,  544,  540,  423,  249,
      506,  257,  549,  267,  529,  269,  498,  513,  252,  521,
      556, 1642,  528,  544,  562,  282,  543,  264,  542,  530,
     1642,  501,  488,  477,  481,  486,  489,  554,  569,  577,
      293,  497,  496,  611,  473,  597,  612,  618,  502,  297,
      633,  639,  285,  490,  474,  467,  495,  454,  468, 1642,
      654,  660,  675,  296,  472,  471,  470,  709,  735,  316,
      318, 1642,  761,  445,  695,  721,  768,  776,  783,  465,
      450,  455,  453,  438, 1642,  791,  798,  806,  297,  461,

      459,  458,  457,  344,  350,  377,  840,  351,  874,  908,
      934,  960,  986, 1012, 1046,  450,  825,  859,  894,  920,
      946,  461,  380,  397, 1642,  972, 1053, 1061,  301,  394,
      393,  391,  370,  369,  383, 1642,  357,  385,  418, 1095,
     1129,  487,  386, 1108, 1163, 1197, 1182,  347, 1204, 1212,
     1219,  458, 1227,  381, 1642,  330,  444, 1234,  343, 1642,
     1642, 1642, 1642, 1642, 1642, 1026,  388,  403, 1242, 1276,
      439,  454, 1262, 1277,  281,  653,  294,  275,  290,  261,
     1283, 1298, 1304, 1319, 1642,  241,  230,  153, 1642, 1642,
     1325, 1340, 1346,  143,  149,   98, 1361, 1367, 1382,   45,

     1642,   42, 1388, 1403, 1642, 1642, 1642, 1437, 1442, 1451,
     1456, 1463, 1469, 1478, 1485, 1494, 1504, 1506, 1513, 1518,
     1525, 1535, 1542, 1547, 1557, 1562, 1571, 1576, 1585, 1594,
     1603, 1608, 1617, 1626, 1631
    } ;

static yyconst short int yy_def[336] =
    {   0,
      307,    1,  307,  307,  307,  307,  307,  308,  309,  307,
      310,  307,  311,  307,  307,  307,  307,  307,  312,  307,
      313,  313,  307,  307,  307,  307,  307,  308,  307,  314,
      309,  315,  307,  310,  316,  307,  307,  312,  313,   16,
      317,  307,  318,  307,   16,  319,  320,  320,  320,  320,
      320,  320,  320,  320,  320,  320,  320,  320,  320,  307,
      307,  307,  307,  307,  307,  307,  313,  313,  307,  307,
      313,  321,  307,  313,  307,  307,  307,  308,  308,  308,
      308,  314,  309,  309,  310,  310,  310,  310,  316,  307,
      317,  322,  320,  320,  320,  323,  320,  320,  320,  320,

      320,  320,  320,  320,  320,  320,  320,  320,  320,  320,
      320,  307,  307,  307,  307,  307,  307,  307,   68,  313,
       68,  324,  307,  313,  307,  308,   81,   84,  310,   88,
      325,  307,   94,  320,   94,  320,  320,  320,  320,  320,
      307,  307,  307,  307,  307,  307,  307,   68,  121,  307,
      326,  307,  307,  327,  307,   81,   84,   88,  325,  322,
       94,  135,  320,  320,  307,  307,  307,  307,  307,  307,
       68,  121,  307,  328,  307,  307,  307,  327,  327,  329,
      330,  307,  331,  307,   81,   84,   88,   94,  135,  320,
      307,  307,  307,  307,  307,   68,  121,  307,  332,  307,

      307,  307,  307,  307,  329,  307,  333,  330,  334,  327,
      327,  327,  327,  327,  331,  307,   81,   84,   88,   94,
      135,  307,  307,  307,  307,   68,  121,  307,  335,  307,
      307,  307,  307,  307,  307,  307,  329,  329,  329,  329,
      333,  330,  330,  330,  330,  334,  214,  307,   81,   84,
       88,  320,  135,  307,  307,  307,  226,  307,  307,  307,
      307,  307,  307,  307,  307,  329,  329,  329,  240,  330,
      330,  330,  245,  214,  307,  320,  307,  307,  307,  307,
      307,  240,  245,  214,  307,  307,  307,  307,  307,  307,
      240,  245,  214,  307,  307,  307,  240,  245,  214,  307,

      307,  307,  240,  245,  307,  307,    0,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307
    } ;

static yyconst short int yy_nxt[1699] =
    {   0,
        4,    5,    6,    5,    5,    5,    7,    8,    9,   10,
        4,    4,   11,    4,    4,   12,    4,   13,   14,   15,
       16,   17,    4,    4,    4,   18,   19,   20,   21,   21,
       21,   21,   21,   21,   21,   21,   21,   21,   21,   21,
       21,   21,   21,   21,   21,   21,   21,   21,   21,   22,
       21,   21,   21,   23,   24,   21,   25,   25,   25,   25,
       25,   25,   25,   25,   25,   25,   26,   26,   26,   26,
       26,   29,   29,   37,   61,  306,   70,   25,   25,   25,
       25,   25,   38,   70,   70,   29,   35,   73,   62,   72,
       30,   63,  305,   29,   64,   68,   72,   72,   65,   35,

       96,   66,   27,   42,   68,   68,   68,   68,   68,   68,
       43,   44,   30,   45,  307,   98,   40,   74,   96,   46,
       97,   47,   48,   48,   49,   50,   51,   48,   52,   53,
       54,   48,   55,   48,   56,   48,   48,   57,   48,   58,
       59,   48,   48,   48,   48,   48,   84,  302,   48,   26,
       26,   26,   26,   26,   68,   84,   84,   84,   84,   84,
       84,   94,   96,   68,   68,   68,   68,   68,   68,   99,
       94,   94,   94,   94,   94,   94,   96,   96,   96,   96,
       96,   96,  301,  300,   29,   27,   28,   28,   28,   78,
      105,  100,   80,  296,   70,  104,  106,  102,   35,   70,

       70,  101,  107,   96,  103,   81,   96,   72,  108,  111,
       29,   82,   72,   72,   81,   81,   81,   81,   81,   81,
       34,   34,   34,   85,  121,  109,   96,  124,  110,   30,
       29,   87,  140,  121,  121,  121,  121,  121,  121,   88,
       29,   28,   96,   92,   35,   89,   29,  132,   88,   88,
       88,   88,   88,   88,   67,   67,   67,   67,   67,   30,
       96,  136,   70,   96,   29,   30,   70,   34,   96,  137,
      295,  138,  294,  119,  150,   72,   29,   29,   35,   72,
      154,  152,  119,  119,  119,  119,  119,  119,  122,  139,
       96,   35,  123,   72,  289,   30,  164,  122,  122,  122,

      122,  122,  122,   79,  126,  126,  126,   79,   96,   29,
      150,   96,  160,  150,  150,  163,  132,  175,  150,  288,
      200,  230,  127,  206,  287,  260,  190,  286,   30,  285,
      206,  127,  127,  127,  127,  127,  127,   28,   28,   28,
       78,  135,  207,   80,  209,  204,  204,  204,  204,  204,
      135,  135,  135,  135,  135,  135,   81,  206,  182,  205,
      150,  280,   82,  206,  206,   81,   81,   81,   81,   81,
       81,   83,   83,   83,   83,   83,  207,  209,  235,  235,
      235,  235,  235,  207,  235,  235,  235,  235,  235,  275,
      128,  236,  206,  265,  264,  206,   32,  236,  206,  128,

      128,  128,  128,  128,  128,   86,  129,  129,  129,   86,
      206,  207,  209,  277,  207,  263,   29,  262,  261,  266,
      235,  235,  235,  266,  130,  206,  256,  278,  255,  207,
       35,  279,  267,  130,  130,  130,  130,  130,  130,   34,
       34,   34,   85,  148,  207,  120,  120,  120,  120,  120,
       87,  206,  148,  148,  148,  148,  148,  148,   88,   93,
       93,   93,   93,   93,   89,  209,  206,   88,   88,   88,
       88,   88,   88,   93,   93,   93,   93,   93,  254,  248,
      209,  234,  233,  232,   96,  231,  225,  224,  223,  208,
      222,   96,  133,  216,  203,  202,  201,  195,   96,  206,

      194,  133,  133,  133,  133,  133,  133,  120,  120,  120,
      120,  120,  193,  209,  192,  191,   96,  160,  156,  184,
      177,  176,  170,  169,  168,  167,  149,  156,  156,  156,
      156,  156,  156,  157,  166,  149,  149,  149,  149,  149,
      149,  158,  157,  157,  157,  157,  157,  157,  161,  165,
      158,  158,  158,  158,  158,  158,   96,  161,  161,  161,
      161,  161,  161,  134,  134,  134,  134,  134,   96,   96,
       96,  160,  155,  153,  171,  147,  146,  145,  144,  143,
      142,  141,  162,  171,  171,  171,  171,  171,  171,  172,
       96,  162,  162,  162,  162,  162,  162,  173,  172,  172,

      172,  172,  172,  172,   96,   96,  173,  173,  173,  173,
      173,  173,  178,  178,  178,  178,  178,  185,  180,   96,
       96,   96,   96,  181,   96,  182,  185,  185,  185,  185,
      185,  185,  186,   96,   96,   96,   96,  183,  187,   92,
       32,  186,  186,  186,  186,  186,  186,  187,  187,  187,
      187,  187,  187,  188,  134,  134,  134,  134,  134,  189,
      125,  118,  188,  188,  188,  188,  188,  188,  189,  189,
      189,  189,  189,  189,  196,  117,  116,  115,  114,   96,
      197,  113,  112,  196,  196,  196,  196,  196,  196,  197,
      197,  197,  197,  197,  197,  198,   96,   96,   40,   46,

       42,   92,   90,   32,  198,  198,  198,  198,  198,  198,
      178,  178,  178,  178,  178,  217,  180,   77,   76,   75,
       69,  181,   60,  182,  217,  217,  217,  217,  217,  217,
       41,   40,   36,   33,   32,  183,  204,  204,  204,  204,
      204,  218,  307,  307,  307,  307,  307,  307,  307,  182,
      218,  218,  218,  218,  218,  218,  307,  307,  307,  307,
      307,  183,  204,  204,  204,  204,  210,  307,  212,  307,
      307,  307,  307,  212,  212,  213,  307,  307,  307,  307,
      307,  214,  307,  307,  307,  307,  307,  215,  219,  307,
      214,  214,  214,  214,  214,  214,  220,  219,  219,  219,

      219,  219,  219,  221,  307,  220,  220,  220,  220,  220,
      220,  226,  221,  221,  221,  221,  221,  221,  227,  307,
      226,  226,  226,  226,  226,  226,  228,  227,  227,  227,
      227,  227,  227,  307,  307,  228,  228,  228,  228,  228,
      228,  205,  205,  205,  237,  249,  307,  239,  307,  307,
      307,  307,  307,  307,  249,  249,  249,  249,  249,  249,
      240,  307,  307,  307,  307,  307,  241,  307,  307,  240,
      240,  240,  240,  240,  240,  208,  208,  208,  242,  250,
      307,  307,  307,  307,  307,  307,  244,  307,  250,  250,
      250,  250,  250,  250,  245,  307,  307,  307,  307,  307,

      246,  307,  307,  245,  245,  245,  245,  245,  245,  204,
      204,  204,  204,  204,  251,  307,  307,  307,  307,  307,
      307,  307,  182,  251,  251,  251,  251,  251,  251,  307,
      307,  307,  307,  307,  183,  204,  204,  204,  204,  204,
      252,  307,  307,  307,  307,  307,  307,  307,  182,  252,
      252,  252,  252,  252,  252,  307,  307,  307,  307,  307,
      183,  204,  204,  204,  204,  204,  253,  307,  307,  307,
      307,  307,  307,  307,  182,  253,  253,  253,  253,  253,
      253,  307,  307,  307,  307,  307,  183,  204,  204,  204,
      204,  204,   71,  307,  307,  307,  307,  307,  307,  307,

      182,   71,   71,   71,   71,   71,   71,  307,  307,  307,
      307,  307,  183,  210,  210,  210,  210,  210,  307,  307,
      307,  307,  307,  307,  307,  307,  182,  266,  235,  235,
      235,  266,  247,  206,  307,  307,  307,  307,  183,  307,
      267,  247,  247,  247,  247,  247,  247,  204,  204,  204,
      204,  210,  207,  212,  307,  307,  307,  307,  212,  212,
      213,  307,  307,  307,  307,  307,  214,  307,  307,  307,
      307,  307,  215,  257,  307,  214,  214,  214,  214,  214,
      214,  258,  257,  257,  257,  257,  257,  257,  307,  307,
      258,  258,  258,  258,  258,  258,  238,  268,  268,  268,

      238,  307,  206,  307,  307,  307,  307,  307,  307,  270,
      235,  235,  235,  270,  307,  269,  307,  307,  307,  307,
      206,  207,  271,  307,  269,  269,  269,  269,  269,  269,
      205,  205,  205,  237,  209,  307,  239,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  240,
      307,  307,  307,  307,  307,  241,  307,  307,  240,  240,
      240,  240,  240,  240,  243,  272,  272,  272,  243,  307,
      307,  307,  307,  307,  307,  206,  307,  307,  307,  307,
      307,  307,  307,  273,  307,  307,  307,  307,  307,  209,
      307,  307,  273,  273,  273,  273,  273,  273,  208,  208,

      208,  242,  274,  307,  307,  307,  307,  307,  307,  244,
      307,  274,  274,  274,  274,  274,  274,  245,  307,  307,
      307,  307,  307,  246,   28,  307,  245,  245,  245,  245,
      245,  245,   31,   28,   28,   28,   28,   28,   28,   34,
      307,   31,   31,   31,   31,   31,   31,  276,   34,   34,
       34,   34,   34,   34,  281,  307,  276,  276,  276,  276,
      276,  276,  282,  281,  281,  281,  281,  281,  281,  307,
      307,  282,  282,  282,  282,  282,  282,  270,  235,  235,
      235,  270,  283,  307,  307,  307,  307,  307,  206,  307,
      271,  283,  283,  283,  283,  283,  283,  284,  307,  307,

      307,  307,  209,  290,  307,  307,  284,  284,  284,  284,
      284,  284,  290,  290,  290,  290,  290,  290,  291,  307,
      307,  307,  307,  307,  292,  307,  307,  291,  291,  291,
      291,  291,  291,  292,  292,  292,  292,  292,  292,  293,
      307,  307,  307,  307,  307,  297,  307,  307,  293,  293,
      293,  293,  293,  293,  297,  297,  297,  297,  297,  297,
      298,  307,  307,  307,  307,  307,  299,  307,  307,  298,
      298,  298,  298,  298,  298,  299,  299,  299,  299,  299,
      299,  303,  307,  307,  307,  307,  307,  304,  307,  307,
      303,  303,  303,  303,  303,  303,  304,  304,  304,  304,

      304,  304,  179,  307,  307,  307,  307,  307,  205,  307,
      307,  179,  179,  179,  179,  179,  179,  205,  205,  205,
      205,  205,  205,  208,  307,  307,  307,  307,  307,  307,
      307,  307,  208,  208,  208,  208,  208,  208,   28,  307,
       28,   28,   28,   28,   28,   28,   28,   31,   31,  307,
       31,   31,   34,  307,   34,   34,   34,   34,   34,   34,
       34,   39,  307,  307,   39,   39,   67,   67,   67,   67,
       67,   67,   67,   71,   71,   71,  307,   71,   71,   79,
       79,   79,   79,   79,   79,   79,   79,   79,   83,   83,
       83,   83,   83,   83,   83,   86,   86,   86,   86,   86,

       86,   86,   86,   86,   91,   91,   91,   91,   91,   91,
       91,   91,   91,   91,   48,   48,   93,   93,   93,   93,
       93,   93,   93,   95,   95,  307,   95,   95,  120,  120,
      120,  120,  120,  120,  120,  131,  131,  131,  131,  131,
      131,  131,  131,  131,  131,  134,  134,  134,  134,  134,
      134,  134,  151,  151,  151,  307,  151,  159,  159,  159,
      159,  159,  159,  159,  159,  159,  159,  174,  174,  174,
      307,  174,  179,  179,  179,  307,  179,  179,  179,  179,
      179,  199,  199,  199,  307,  199,  205,  307,  205,  205,
      205,  205,  205,  205,  205,  208,  307,  208,  208,  208,

      208,  208,  208,  208,  211,  211,  211,  211,  211,  211,
      211,  211,  211,  229,  229,  229,  307,  229,  238,  238,
      238,  238,  238,  238,  238,  238,  238,  243,  243,  243,
      243,  243,  243,  243,  243,  243,  259,  259,  259,  307,
      259,    3,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307

    } ;

static yyconst short int yy_chk[1699] =
    {   0,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
        1,    1,    1,    1,    1,    1,    5,    5,    5,    5,
        5,    6,    6,    6,    6,    6,    7,    7,    7,    7,
        7,    8,   11,   13,   18,  302,   21,   25,   25,   25,
       25,   25,   13,   39,   22,   34,   11,   22,   18,   21,
        8,   18,  300,   28,   18,   19,   39,   22,   18,   34,

       49,   18,    7,   16,   19,   19,   19,   19,   19,   19,
       16,   16,   28,   16,   40,   49,   40,   22,   47,   16,
       47,   16,   16,   16,   16,   16,   16,   16,   16,   16,
       16,   16,   16,   16,   16,   16,   16,   16,   16,   16,
       16,   16,   16,   16,   16,   16,   32,  296,   16,   26,
       26,   26,   26,   26,   38,   32,   32,   32,   32,   32,
       32,   46,   50,   38,   38,   38,   38,   38,   38,   50,
       46,   46,   46,   46,   46,   46,   51,   52,   53,   54,
       55,   56,  295,  294,   86,   26,   30,   30,   30,   30,
       55,   51,   30,  288,   67,   54,   56,   52,   86,   71,

       74,   51,   56,   57,   53,   30,   58,   67,   57,   58,
       79,   30,   71,   74,   30,   30,   30,   30,   30,   30,
       35,   35,   35,   35,   72,   57,  111,   74,   57,   79,
       87,   35,  111,   72,   72,   72,   72,   72,   72,   35,
       80,   78,   97,   92,   87,   35,   78,   92,   35,   35,
       35,   35,   35,   35,   68,   68,   68,   68,   68,   80,
       99,   97,  120,  105,  129,   78,   68,   85,  102,   99,
      287,  102,  286,   68,  122,  120,  126,   85,  129,   68,
      124,  122,   68,   68,   68,   68,   68,   68,   73,  105,
      138,   85,   73,  124,  280,  126,  138,   73,   73,   73,

       73,   73,   73,   81,   81,   81,   81,   81,  136,   81,
      151,  163,  160,  174,  199,  136,  160,  151,  229,  279,
      174,  199,   81,  180,  278,  229,  163,  277,   81,  275,
      181,   81,   81,   81,   81,   81,   81,   82,   82,   82,
       82,   96,  180,   82,  181,  204,  204,  204,  204,  204,
       96,   96,   96,   96,   96,   96,   82,  205,  204,  237,
      259,  256,   82,  208,  237,   82,   82,   82,   82,   82,
       82,   84,   84,   84,   84,   84,  205,  208,  206,  206,
      206,  206,  206,  237,  235,  235,  235,  235,  235,  248,
       84,  206,  238,  234,  233,  267,   84,  235,  243,   84,

       84,   84,   84,   84,   84,   88,   88,   88,   88,   88,
      268,  238,  243,  254,  267,  232,   88,  231,  230,  239,
      239,  239,  239,  239,   88,  239,  224,  254,  223,  268,
       88,  254,  239,   88,   88,   88,   88,   88,   88,   89,
       89,   89,   89,  119,  239,  257,  257,  257,  257,  257,
       89,  271,  119,  119,  119,  119,  119,  119,   89,  252,
      252,  252,  252,  252,   89,  271,  272,   89,   89,   89,
       89,   89,   89,   94,   94,   94,   94,   94,  222,  216,
      272,  203,  202,  201,  252,  200,  194,  193,  192,  242,
      191,  190,   94,  184,  177,  176,  175,  169,   94,  242,

      168,   94,   94,   94,   94,   94,   94,  121,  121,  121,
      121,  121,  167,  242,  166,  165,  164,  159,  127,  155,
      153,  152,  147,  146,  145,  144,  121,  127,  127,  127,
      127,  127,  127,  128,  143,  121,  121,  121,  121,  121,
      121,  130,  128,  128,  128,  128,  128,  128,  133,  142,
      130,  130,  130,  130,  130,  130,  140,  133,  133,  133,
      133,  133,  133,  135,  135,  135,  135,  135,  139,  137,
      134,  131,  125,  123,  148,  118,  117,  116,  115,  114,
      113,  112,  135,  148,  148,  148,  148,  148,  148,  149,
      110,  135,  135,  135,  135,  135,  135,  150,  149,  149,

      149,  149,  149,  149,  109,  108,  150,  150,  150,  150,
      150,  150,  154,  154,  154,  154,  154,  156,  154,  107,
      106,  104,  103,  154,  101,  154,  156,  156,  156,  156,
      156,  156,  157,  100,   98,   95,   93,  154,  158,   91,
       83,  157,  157,  157,  157,  157,  157,  158,  158,  158,
      158,  158,  158,  161,  276,  276,  276,  276,  276,  162,
       77,   66,  161,  161,  161,  161,  161,  161,  162,  162,
      162,  162,  162,  162,  171,   65,   64,   63,   62,  276,
      172,   61,   60,  171,  171,  171,  171,  171,  171,  172,
      172,  172,  172,  172,  172,  173,   59,   48,   44,   43,

       42,   41,   37,   31,  173,  173,  173,  173,  173,  173,
      178,  178,  178,  178,  178,  185,  178,   27,   24,   23,
       20,  178,   17,  178,  185,  185,  185,  185,  185,  185,
       15,   14,   12,   10,    9,  178,  179,  179,  179,  179,
      179,  186,  179,    3,    0,    0,    0,  179,    0,  179,
      186,  186,  186,  186,  186,  186,    0,    0,    0,    0,
        0,  179,  183,  183,  183,  183,  183,    0,  183,    0,
        0,    0,    0,  183,  183,  183,    0,    0,    0,    0,
        0,  183,    0,    0,    0,    0,    0,  183,  187,    0,
      183,  183,  183,  183,  183,  183,  188,  187,  187,  187,

      187,  187,  187,  189,    0,  188,  188,  188,  188,  188,
      188,  196,  189,  189,  189,  189,  189,  189,  197,    0,
      196,  196,  196,  196,  196,  196,  198,  197,  197,  197,
      197,  197,  197,    0,    0,  198,  198,  198,  198,  198,
      198,  207,  207,  207,  207,  217,    0,  207,    0,    0,
        0,    0,    0,    0,  217,  217,  217,  217,  217,  217,
      207,    0,    0,    0,    0,    0,  207,    0,    0,  207,
      207,  207,  207,  207,  207,  209,  209,  209,  209,  218,
        0,    0,    0,    0,    0,    0,  209,    0,  218,  218,
      218,  218,  218,  218,  209,    0,    0,    0,    0,    0,

      209,    0,    0,  209,  209,  209,  209,  209,  209,  210,
      210,  210,  210,  210,  219,  210,    0,    0,    0,    0,
      210,    0,  210,  219,  219,  219,  219,  219,  219,    0,
        0,    0,    0,    0,  210,  211,  211,  211,  211,  211,
      220,  211,    0,    0,    0,    0,  211,    0,  211,  220,
      220,  220,  220,  220,  220,    0,    0,    0,    0,    0,
      211,  212,  212,  212,  212,  212,  221,  212,    0,    0,
        0,    0,  212,    0,  212,  221,  221,  221,  221,  221,
      221,    0,    0,    0,    0,    0,  212,  213,  213,  213,
      213,  213,  226,  213,    0,    0,    0,    0,  213,    0,

      213,  226,  226,  226,  226,  226,  226,    0,    0,    0,
        0,    0,  213,  214,  214,  214,  214,  214,    0,  214,
        0,    0,    0,    0,  214,    0,  214,  266,  266,  266,
      266,  266,  214,  266,    0,    0,    0,    0,  214,    0,
      266,  214,  214,  214,  214,  214,  214,  215,  215,  215,
      215,  215,  266,  215,    0,    0,    0,    0,  215,  215,
      215,    0,    0,    0,    0,    0,  215,    0,    0,    0,
        0,    0,  215,  227,    0,  215,  215,  215,  215,  215,
      215,  228,  227,  227,  227,  227,  227,  227,    0,    0,
      228,  228,  228,  228,  228,  228,  240,  240,  240,  240,

      240,    0,  240,    0,    0,    0,    0,    0,    0,  244,
      244,  244,  244,  244,    0,  240,    0,    0,    0,    0,
      244,  240,  244,    0,  240,  240,  240,  240,  240,  240,
      241,  241,  241,  241,  244,    0,  241,    0,    0,    0,
        0,    0,    0,    0,    0,    0,    0,    0,    0,  241,
        0,    0,    0,    0,    0,  241,    0,    0,  241,  241,
      241,  241,  241,  241,  245,  245,  245,  245,  245,    0,
        0,    0,    0,    0,    0,  245,    0,    0,    0,    0,
        0,    0,    0,  245,    0,    0,    0,    0,    0,  245,
        0,    0,  245,  245,  245,  245,  245,  245,  246,  246,

      246,  246,  247,    0,    0,    0,    0,    0,    0,  246,
        0,  247,  247,  247,  247,  247,  247,  246,    0,    0,
        0,    0,    0,  246,  249,    0,  246,  246,  246,  246,
      246,  246,  250,  249,  249,  249,  249,  249,  249,  251,
        0,  250,  250,  250,  250,  250,  250,  253,  251,  251,
      251,  251,  251,  251,  258,    0,  253,  253,  253,  253,
      253,  253,  269,  258,  258,  258,  258,  258,  258,    0,
        0,  269,  269,  269,  269,  269,  269,  270,  270,  270,
      270,  270,  273,    0,    0,    0,    0,    0,  270,    0,
      270,  273,  273,  273,  273,  273,  273,  274,    0,    0,

        0,    0,  270,  281,    0,    0,  274,  274,  274,  274,
      274,  274,  281,  281,  281,  281,  281,  281,  282,    0,
        0,    0,    0,    0,  283,    0,    0,  282,  282,  282,
      282,  282,  282,  283,  283,  283,  283,  283,  283,  284,
        0,    0,    0,    0,    0,  291,    0,    0,  284,  284,
      284,  284,  284,  284,  291,  291,  291,  291,  291,  291,
      292,    0,    0,    0,    0,    0,  293,    0,    0,  292,
      292,  292,  292,  292,  292,  293,  293,  293,  293,  293,
      293,  297,    0,    0,    0,    0,    0,  298,    0,    0,
      297,  297,  297,  297,  297,  297,  298,  298,  298,  298,

      298,  298,  299,    0,    0,    0,    0,    0,  303,    0,
        0,  299,  299,  299,  299,  299,  299,  303,  303,  303,
      303,  303,  303,  304,    0,    0,    0,    0,    0,    0,
        0,    0,  304,  304,  304,  304,  304,  304,  308,    0,
      308,  308,  308,  308,  308,  308,  308,  309,  309,    0,
      309,  309,  310,    0,  310,  310,  310,  310,  310,  310,
      310,  311,    0,    0,  311,  311,  312,  312,  312,  312,
      312,  312,  312,  313,  313,  313,    0,  313,  313,  314,
      314,  314,  314,  314,  314,  314,  314,  314,  315,  315,
      315,  315,  315,  315,  315,  316,  316,  316,  316,  316,

      316,  316,  316,  316,  317,  317,  317,  317,  317,  317,
      317,  317,  317,  317,  318,  318,  319,  319,  319,  319,
      319,  319,  319,  320,  320,    0,  320,  320,  321,  321,
      321,  321,  321,  321,  321,  322,  322,  322,  322,  322,
      322,  322,  322,  322,  322,  323,  323,  323,  323,  323,
      323,  323,  324,  324,  324,    0,  324,  325,  325,  325,
      325,  325,  325,  325,  325,  325,  325,  326,  326,  326,
        0,  326,  327,  327,  327,    0,  327,  327,  327,  327,
      327,  328,  328,  328,    0,  328,  329,    0,  329,  329,
      329,  329,  329,  329,  329,  330,    0,  330,  330,  330,

      330,  330,  330,  330,  331,  331,  331,  331,  331,  331,
      331,  331,  331,  332,  332,  332,    0,  332,  333,  333,
      333,  333,  333,  333,  333,  333,  333,  334,  334,  334,
      334,  334,  334,  334,  334,  334,  335,  335,  335,    0,
      335,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307,  307,  307,
      307,  307,  307,  307,  307,  307,  307,  307

    } ;

YY_DECL
	{
	register yy_state_type yy_current_state;
	register unsigned short *yy_cp, *yy_bp;
	register int yy_act;
	while ( 1 )		/* loops until end-of-file is reached */
		{
		yy_cp = yy_c_buf_p;

		/* Support of yytext. */
		*yy_cp = yy_hold_char;

		/* yy_bp points to the position in yy_ch_buf of the start of
		 * the current run.
		 */
		yy_bp = yy_cp;

		yy_current_state = yy_start;
		do
			{
			register YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)];
			if ( yy_accept[yy_current_state] )
				{
				yy_last_accepting_state = yy_current_state;
				yy_last_accepting_cpos = yy_cp;
				}
			while ( yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state )
				{
				yy_current_state = (int) yy_def[yy_current_state];
				if ( yy_current_state >= 308 )
					yy_c = yy_meta[(unsigned int) yy_c];
				}
			yy_current_state = yy_nxt[yy_base[yy_current_state] + (unsigned int) yy_c];
			++yy_cp;
			}
		while ( yy_base[yy_current_state] != 1642 );

yy_find_action:
		yy_act = yy_accept[yy_current_state];
		if ( yy_act == 0 )
			{ /* have to back up */
			yy_cp = yy_last_accepting_cpos;
			yy_current_state = yy_last_accepting_state;
			yy_act = yy_accept[yy_current_state];
			}

		YY_DO_BEFORE_ACTION;




		switch ( yy_act )
	{ /* beginning of action switch */
			case 0: /* must back up */
			/* undo the effects of YY_DO_BEFORE_ACTION */
			*yy_cp = yy_hold_char;
			yy_cp = yy_last_accepting_cpos;
			yy_current_state = yy_last_accepting_state;
			goto yy_find_action;

case 1:
YY_RULE_SETUP
#line 25 "tokenizer.flex"
/* ignore comments */
	YY_BREAK
case 2:
YY_RULE_SETUP
#line 27 "tokenizer.flex"
{yyTok = S; return yyTok;}
	YY_BREAK
case 3:
YY_RULE_SETUP
#line 29 "tokenizer.flex"
{yyTok = SGML_CD; return yyTok;}
	YY_BREAK
case 4:
YY_RULE_SETUP
#line 30 "tokenizer.flex"
{yyTok = SGML_CD; return yyTok;}
	YY_BREAK
case 5:
YY_RULE_SETUP
#line 31 "tokenizer.flex"
{yyTok = INCLUDES; return yyTok;}
	YY_BREAK
case 6:
YY_RULE_SETUP
#line 32 "tokenizer.flex"
{yyTok = DASHMATCH; return yyTok;}
	YY_BREAK
case 7:
YY_RULE_SETUP
#line 33 "tokenizer.flex"
{yyTok = BEGINSWITH; return yyTok;}
	YY_BREAK
case 8:
YY_RULE_SETUP
#line 34 "tokenizer.flex"
{yyTok = ENDSWITH; return yyTok;}
	YY_BREAK
case 9:
YY_RULE_SETUP
#line 35 "tokenizer.flex"
{yyTok = CONTAINS; return yyTok;}
	YY_BREAK
case 10:
YY_RULE_SETUP
#line 37 "tokenizer.flex"
{yyTok = STRING; return yyTok;}
	YY_BREAK
case 11:
YY_RULE_SETUP
#line 39 "tokenizer.flex"
{yyTok = IDENT; return yyTok;}
	YY_BREAK
case 12:
YY_RULE_SETUP
#line 41 "tokenizer.flex"
{yyTok = HASH; return yyTok;}
	YY_BREAK
case 13:
YY_RULE_SETUP
#line 43 "tokenizer.flex"
{yyTok = IMPORT_SYM; return yyTok;}
	YY_BREAK
case 14:
YY_RULE_SETUP
#line 44 "tokenizer.flex"
{yyTok = PAGE_SYM; return yyTok;}
	YY_BREAK
case 15:
YY_RULE_SETUP
#line 45 "tokenizer.flex"
{yyTok = MEDIA_SYM; return yyTok;}
	YY_BREAK
case 16:
YY_RULE_SETUP
#line 46 "tokenizer.flex"
{yyTok = FONT_FACE_SYM; return yyTok;}
	YY_BREAK
case 17:
YY_RULE_SETUP
#line 47 "tokenizer.flex"
{yyTok = CHARSET_SYM; return yyTok;}
	YY_BREAK
case 18:
YY_RULE_SETUP
#line 48 "tokenizer.flex"
{yyTok = KHTML_RULE_SYM; return yyTok; }
	YY_BREAK
case 19:
YY_RULE_SETUP
#line 49 "tokenizer.flex"
{yyTok = KHTML_DECLS_SYM; return yyTok; }
	YY_BREAK
case 20:
YY_RULE_SETUP
#line 50 "tokenizer.flex"
{yyTok = KHTML_VALUE_SYM; return yyTok; }
	YY_BREAK
case 21:
YY_RULE_SETUP
#line 52 "tokenizer.flex"
{yyTok = IMPORTANT_SYM; return yyTok;}
	YY_BREAK
case 22:
YY_RULE_SETUP
#line 54 "tokenizer.flex"
{yyTok = EMS; return yyTok;}
	YY_BREAK
case 23:
YY_RULE_SETUP
#line 55 "tokenizer.flex"
{yyTok = QEMS; return yyTok;} /* quirky ems */
	YY_BREAK
case 24:
YY_RULE_SETUP
#line 56 "tokenizer.flex"
{yyTok = EXS; return yyTok;}
	YY_BREAK
case 25:
YY_RULE_SETUP
#line 57 "tokenizer.flex"
{yyTok = PXS; return yyTok;}
	YY_BREAK
case 26:
YY_RULE_SETUP
#line 58 "tokenizer.flex"
{yyTok = CMS; return yyTok;}
	YY_BREAK
case 27:
YY_RULE_SETUP
#line 59 "tokenizer.flex"
{yyTok = MMS; return yyTok;}
	YY_BREAK
case 28:
YY_RULE_SETUP
#line 60 "tokenizer.flex"
{yyTok = INS; return yyTok;}
	YY_BREAK
case 29:
YY_RULE_SETUP
#line 61 "tokenizer.flex"
{yyTok = PTS; return yyTok;}
	YY_BREAK
case 30:
YY_RULE_SETUP
#line 62 "tokenizer.flex"
{yyTok = PCS; return yyTok;}
	YY_BREAK
case 31:
YY_RULE_SETUP
#line 63 "tokenizer.flex"
{yyTok = DEGS; return yyTok;}
	YY_BREAK
case 32:
YY_RULE_SETUP
#line 64 "tokenizer.flex"
{yyTok = RADS; return yyTok;}
	YY_BREAK
case 33:
YY_RULE_SETUP
#line 65 "tokenizer.flex"
{yyTok = GRADS; return yyTok;}
	YY_BREAK
case 34:
YY_RULE_SETUP
#line 66 "tokenizer.flex"
{yyTok = MSECS; return yyTok;}
	YY_BREAK
case 35:
YY_RULE_SETUP
#line 67 "tokenizer.flex"
{yyTok = SECS; return yyTok;}
	YY_BREAK
case 36:
YY_RULE_SETUP
#line 68 "tokenizer.flex"
{yyTok = HERZ; return yyTok;}
	YY_BREAK
case 37:
YY_RULE_SETUP
#line 69 "tokenizer.flex"
{yyTok = KHERZ; return yyTok;}
	YY_BREAK
case 38:
YY_RULE_SETUP
#line 70 "tokenizer.flex"
{yyTok = DIMEN; return yyTok;}
	YY_BREAK
case 39:
YY_RULE_SETUP
#line 71 "tokenizer.flex"
{yyTok = PERCENTAGE; return yyTok;}
	YY_BREAK
case 40:
YY_RULE_SETUP
#line 72 "tokenizer.flex"
{yyTok = NUMBER; return yyTok;}
	YY_BREAK
case 41:
YY_RULE_SETUP
#line 74 "tokenizer.flex"
{yyTok = URI; return yyTok;}
	YY_BREAK
case 42:
YY_RULE_SETUP
#line 75 "tokenizer.flex"
{yyTok = URI; return yyTok;}
	YY_BREAK
case 43:
YY_RULE_SETUP
#line 76 "tokenizer.flex"
{yyTok = FUNCTION; return yyTok;}
	YY_BREAK
case 44:
YY_RULE_SETUP
#line 78 "tokenizer.flex"
{yyTok = UNICODERANGE; return yyTok;}
	YY_BREAK
case 45:
YY_RULE_SETUP
#line 79 "tokenizer.flex"
{yyTok = UNICODERANGE; return yyTok;}
	YY_BREAK
case 46:
YY_RULE_SETUP
#line 81 "tokenizer.flex"
{yyTok = *yytext; return yyTok;}
	YY_BREAK
case 47:
YY_RULE_SETUP
#line 83 "tokenizer.flex"
ECHO;
	YY_BREAK
#line 1320 "lex.yy.c"
case YY_STATE_EOF(INITIAL):
case YY_END_OF_BUFFER:
	yyterminate();

	default:
		YY_FATAL_ERROR(
			"fatal flex scanner internal error--no action found" );
	} /* end of action switch */
		} /* end of scanning one token */
	} /* end of yylex */
