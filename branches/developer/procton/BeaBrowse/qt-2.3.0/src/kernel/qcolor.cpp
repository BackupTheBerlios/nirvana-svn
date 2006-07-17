/****************************************************************************
** $Id: qcolor.cpp,v 1.1.1.1 2002/04/27 11:02:18 procton Exp $
**
** Implementation of QColor class
**
** Created : 940112
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifdef __BEOS__
#include <stdio.h>
#endif

#include "qcolor.h"
#include "qnamespace.h"
#include "qdatastream.h"
#include <stdlib.h>
#include <ctype.h>


// NOT REVISED
/*!
  \class QColor qcolor.h
  \brief The QColor class provides colors based on RGB.

  \ingroup color
  \ingroup drawing
  \ingroup appearance

  A color is normally specified in terms of RGB (red,green and blue)
  components, but it is also possible to specify HSV (hue,saturation
  and value) or set a color name (the names are copied from from the
  X11 color database).

  In addition to the RGB value, a QColor also has a pixel value.  This
  value is used by the underlying window system to refer to a color.  It
  can be thought of as an index into the display hardware's color table.

  There are 19 predefined QColor objects: \c black, \c white, \c
  darkGray, \c gray, \c lightGray, \c red, \c green, \c blue, \c cyan,
  \c magenta, \c yellow, \c darkRed, \c darkGreen, \c darkBlue, \c
  darkCyan, \c darkMagenta, \c darkYellow, \c color0 and \c color1.

  The colors \c color0 (zero pixel value) and \c color1 (non-zero
  pixel value) are special colors for drawing in \link QBitmap
  bitmaps\endlink.

  The QColor class has an efficient, dynamic color allocation
  strategy.  A color is normally allocated the first time it is used
  (lazy allocation), that is, whenever the pixel() function is called:

  <ol>
  <li>Is the pixel value valid? If it is, just return it, otherwise,
  allocate a pixel value.
  <li>Check an internal hash table to see if we allocated an equal RGB
  value earlier. If we did, set the pixel value and return.
  <li>Try to allocate the RGB value. If we succeed, we get a pixel value
  which we save in the internal table with the RGB value.
  Return the pixel value.
  <li>The color could not be allocated. Find the closest matching
  color and save it in the internal table.
  </ol>

  Since many people don't know the HSV color model very well, we'll
  cover it briefly here.

  The RGB model is hardware-oriented.  Its representation is close to
  what most monitors show.  In contrast, HSV represents color in a way
  more suited to traditional human perception of color.  For example,
  the relationships "stronger than", "darker than", "the opposite of"
  are easily expressed in HSV, but are much harder to express in RGB.

  HSV, like RGB, has three components.  They are: <ul> <li> H, for
  hue, is either 0-360 if the color is chromatic (not gray), or
  meaningless if it is gray.  It represents degrees on the color wheel
  familiar to most people.  Red is 0 (degrees), green is 120 and blue
  is 240. <li> S, for saturation, is 0-255 and the bigger it is, the
  stronger the color is.  Grayish colors have saturation near 0, very
  strong colors have saturation near 255. <li> V, for value, is 0-255
  and represents lightness or brightness of the color.  0 is black,
  255 is far from black as possible. </ul>

  Here are some examples: Pure red is H=0, S=255, V=255.  A dark red,
  moving slightly towards the magenta, could be H=350 (equivalent to
  -10), S=255, V=180.  A grayish light red could have H about 0 (say
  350-359 or 0-10), S about 50-100, and S=255.

  \sa QPalette, QColorGroup, QApplication::setColorSpec(),
  <a href="http://www.inforamp.net/~poynton/Poynton-color.html">Color FAQ.</a>
*/

/*****************************************************************************
  Global colors
 *****************************************************************************/

#if defined(_WS_WIN_)
#define COLOR0_PIX 0x00ffffff
#define COLOR1_PIX 0
#else
#define COLOR0_PIX 0
#define COLOR1_PIX 1
#endif

static QColor stdcol[19];

QT_STATIC_CONST_IMPL QColor & Qt::color0 = stdcol[0];
QT_STATIC_CONST_IMPL QColor & Qt::color1  = stdcol[1];
QT_STATIC_CONST_IMPL QColor & Qt::black  = stdcol[2];
QT_STATIC_CONST_IMPL QColor & Qt::white = stdcol[3];
QT_STATIC_CONST_IMPL QColor & Qt::darkGray = stdcol[4];
QT_STATIC_CONST_IMPL QColor & Qt::gray = stdcol[5];
QT_STATIC_CONST_IMPL QColor & Qt::lightGray = stdcol[6];
QT_STATIC_CONST_IMPL QColor & Qt::red = stdcol[7];
QT_STATIC_CONST_IMPL QColor & Qt::green = stdcol[8];
QT_STATIC_CONST_IMPL QColor & Qt::blue = stdcol[9];
QT_STATIC_CONST_IMPL QColor & Qt::cyan = stdcol[10];
QT_STATIC_CONST_IMPL QColor & Qt::magenta = stdcol[11];
QT_STATIC_CONST_IMPL QColor & Qt::yellow = stdcol[12];
QT_STATIC_CONST_IMPL QColor & Qt::darkRed = stdcol[13];
QT_STATIC_CONST_IMPL QColor & Qt::darkGreen = stdcol[14];
QT_STATIC_CONST_IMPL QColor & Qt::darkBlue = stdcol[15];
QT_STATIC_CONST_IMPL QColor & Qt::darkCyan = stdcol[16];
QT_STATIC_CONST_IMPL QColor & Qt::darkMagenta = stdcol[17];
QT_STATIC_CONST_IMPL QColor & Qt::darkYellow = stdcol[18];


#ifdef __BEOS__
#include <map>
//Not good to undef DEBUG
#undef DEBUG
#include <String.h>

static void get_system_color( const char* pzName, int* r, int* g, int* b )
{
    static struct { unsigned char r, g, b; const char* name; } asColTable[] = {
	255, 250, 250,		"snow",
	248, 248, 255,		"ghost white",
	248, 248, 255,		"GhostWhite",
	245, 245, 245,		"white smoke",
	245, 245, 245,		"WhiteSmoke",
	220, 220, 220,		"gainsboro",
	255, 250, 240,		"floral white",
	255, 250, 240,		"FloralWhite",
	253, 245, 230,		"old lace",
	253, 245, 230,		"OldLace",
	250, 240, 230,		"linen",
	250, 235, 215,		"antique white",
	250, 235, 215,		"AntiqueWhite",
	255, 239, 213,		"papaya whip",
	255, 239, 213,		"PapayaWhip",
	255, 235, 205,		"blanched almond",
	255, 235, 205,		"BlanchedAlmond",
	255, 228, 196,		"bisque",
	255, 218, 185,		"peach puff",
	255, 218, 185,		"PeachPuff",
	255, 222, 173,		"navajo white",
	255, 222, 173,		"NavajoWhite",
	255, 228, 181,		"moccasin",
	255, 248, 220,		"cornsilk",
	255, 255, 240,		"ivory",
	255, 250, 205,		"lemon chiffon",
	255, 250, 205,		"LemonChiffon",
	255, 245, 238,		"seashell",
	240, 255, 240,		"honeydew",
	245, 255, 250,		"mint cream",
	245, 255, 250,		"MintCream",
	240, 255, 255,		"azure",
	240, 248, 255,		"alice blue",
	240, 248, 255,		"AliceBlue",
	230, 230, 250,		"lavender",
	255, 240, 245,		"lavender blush",
	255, 240, 245,		"LavenderBlush",
	255, 228, 225,		"misty rose",
	255, 228, 225,		"MistyRose",
	255, 255, 255,		"white",
	0,   0,   0,		"black",
	47,  79,  79,		"dark slate gray",
	47,  79,  79,		"DarkSlateGray",
	47,  79,  79,		"dark slate grey",
	47,  79,  79,		"DarkSlateGrey",
	105, 105, 105,		"dim gray",
	105, 105, 105,		"DimGray",
	105, 105, 105,		"dim grey",
	105, 105, 105,		"DimGrey",
	112, 128, 144,		"slate gray",
	112, 128, 144,		"SlateGray",
	112, 128, 144,		"slate grey",
	112, 128, 144,		"SlateGrey",
	119, 136, 153,		"light slate gray",
	119, 136, 153,		"LightSlateGray",
	119, 136, 153,		"light slate grey",
	119, 136, 153,		"LightSlateGrey",
	190, 190, 190,		"gray",
	190, 190, 190,		"grey",
	211, 211, 211,		"light grey",
	211, 211, 211,		"LightGrey",
	211, 211, 211,		"light gray",
	211, 211, 211,		"LightGray",
	25,  25, 112,		"midnight blue",
	25,  25, 112,		"MidnightBlue",
	0,   0, 128,		"navy",
	0,   0, 128,		"navy blue",
	0,   0, 128,		"NavyBlue",
	100, 149, 237,		"cornflower blue",
	100, 149, 237,		"CornflowerBlue",
	72,  61, 139,		"dark slate blue",
	72,  61, 139,		"DarkSlateBlue",
	106,  90, 205,		"slate blue",
	106,  90, 205,		"SlateBlue",
	123, 104, 238,		"medium slate blue",
	123, 104, 238,		"MediumSlateBlue",
	132, 112, 255,		"light slate blue",
	132, 112, 255,		"LightSlateBlue",
	0,   0, 205,		"medium blue",
	0,   0, 205,		"MediumBlue",
	65, 105, 225,		"royal blue",
	65, 105, 225,		"RoyalBlue",
	0,   0, 255,		"blue",
	30, 144, 255,		"dodger blue",
	30, 144, 255,		"DodgerBlue",
	0, 191, 255,		"deep sky blue",
	0, 191, 255,		"DeepSkyBlue",
	135, 206, 235,		"sky blue",
	135, 206, 235,		"SkyBlue",
	135, 206, 250,		"light sky blue",
	135, 206, 250,		"LightSkyBlue",
	70, 130, 180,		"steel blue",
	70, 130, 180,		"SteelBlue",
	176, 196, 222,		"light steel blue",
	176, 196, 222,		"LightSteelBlue",
	173, 216, 230,		"light blue",
	173, 216, 230,		"LightBlue",
	176, 224, 230,		"powder blue",
	176, 224, 230,		"PowderBlue",
	175, 238, 238,		"pale turquoise",
	175, 238, 238,		"PaleTurquoise",
	0, 206, 209,		"dark turquoise",
	0, 206, 209,		"DarkTurquoise",
	72, 209, 204,		"medium turquoise",
	72, 209, 204,		"MediumTurquoise",
	64, 224, 208,		"turquoise",
	0, 255, 255,		"cyan",
	224, 255, 255,		"light cyan",
	224, 255, 255,		"LightCyan",
	95, 158, 160,		"cadet blue",
	95, 158, 160,		"CadetBlue",
	102, 205, 170,		"medium aquamarine",
	102, 205, 170,		"MediumAquamarine",
	127, 255, 212,		"aquamarine",
	0, 100,   0,		"dark green",
	0, 100,   0,		"DarkGreen",
	85, 107,  47,		"dark olive green",
	85, 107,  47,		"DarkOliveGreen",
	143, 188, 143,		"dark sea green",
	143, 188, 143,		"DarkSeaGreen",
	46, 139,  87,		"sea green",
	46, 139,  87,		"SeaGreen",
	60, 179, 113,		"medium sea green",
	60, 179, 113,		"MediumSeaGreen",
	32, 178, 170,		"light sea green",
	32, 178, 170,		"LightSeaGreen",
	152, 251, 152,		"pale green",
	152, 251, 152,		"PaleGreen",
	0, 255, 127,		"spring green",
	0, 255, 127,		"SpringGreen",
	124, 252,   0,		"lawn green",
	124, 252,   0,		"LawnGreen",
	0, 255,   0,		"green",
	127, 255,   0,		"chartreuse",
	0, 250, 154,		"medium spring green",
	0, 250, 154,		"MediumSpringGreen",
	173, 255,  47,		"green yellow",
	173, 255,  47,		"GreenYellow",
	50, 205,  50,		"lime green",
	50, 205,  50,		"LimeGreen",
	154, 205,  50,		"yellow green",
	154, 205,  50,		"YellowGreen",
	34, 139,  34,		"forest green",
	34, 139,  34,		"ForestGreen",
	107, 142,  35,		"olive drab",
	107, 142,  35,		"OliveDrab",
	189, 183, 107,		"dark khaki",
	189, 183, 107,		"DarkKhaki",
	240, 230, 140,		"khaki",
	238, 232, 170,		"pale goldenrod",
	238, 232, 170,		"PaleGoldenrod",
	250, 250, 210,		"light goldenrod yellow",
	250, 250, 210,		"LightGoldenrodYellow",
	255, 255, 224,		"light yellow",
	255, 255, 224,		"LightYellow",
	255, 255,   0,		"yellow",
	255, 215,   0, 		"gold",
	238, 221, 130,		"light goldenrod",
	238, 221, 130,		"LightGoldenrod",
	218, 165,  32,		"goldenrod",
	184, 134,  11,		"dark goldenrod",
	184, 134,  11,		"DarkGoldenrod",
	188, 143, 143,		"rosy brown",
	188, 143, 143,		"RosyBrown",
	205,  92,  92,		"indian red",
	205,  92,  92,		"IndianRed",
	139,  69,  19,		"saddle brown",
	139,  69,  19,		"SaddleBrown",
	160,  82,  45,		"sienna",
	205, 133,  63,		"peru",
	222, 184, 135,		"burlywood",
	245, 245, 220,		"beige",
	245, 222, 179,		"wheat",
	244, 164,  96,		"sandy brown",
	244, 164,  96,		"SandyBrown",
	210, 180, 140,		"tan",
	210, 105,  30,		"chocolate",
	178,  34,  34,		"firebrick",
	165,  42,  42,		"brown",
	233, 150, 122,		"dark salmon",
	233, 150, 122,		"DarkSalmon",
	250, 128, 114,		"salmon",
	255, 160, 122,		"light salmon",
	255, 160, 122,		"LightSalmon",
	255, 165,   0,		"orange",
	255, 140,   0,		"dark orange",
	255, 140,   0,		"DarkOrange",
	255, 127,  80,		"coral",
	240, 128, 128,		"light coral",
	240, 128, 128,		"LightCoral",
	255,  99,  71,		"tomato",
	255,  69,   0,		"orange red",
	255,  69,   0,		"OrangeRed",
	255,   0,   0,		"red",
	255, 105, 180,		"hot pink",
	255, 105, 180,		"HotPink",
	255,  20, 147,		"deep pink",
	255,  20, 147,		"DeepPink",
	255, 192, 203,		"pink",
	255, 182, 193,		"light pink",
	255, 182, 193,		"LightPink",
	219, 112, 147,		"pale violet red",
	219, 112, 147,		"PaleVioletRed",
	176,  48,  96,		"maroon",
	199,  21, 133,		"medium violet red",
	199,  21, 133,		"MediumVioletRed",
	208,  32, 144,		"violet red",
	208,  32, 144,		"VioletRed",
	255,   0, 255,		"magenta",
	238, 130, 238,		"violet",
	221, 160, 221,		"plum",
	218, 112, 214,		"orchid",
	186,  85, 211,		"medium orchid",
	186,  85, 211,		"MediumOrchid",
	153,  50, 204,		"dark orchid",
	153,  50, 204,		"DarkOrchid",
	148,   0, 211,		"dark violet",
	148,   0, 211,		"DarkViolet",
	138,  43, 226,		"blue violet",
	138,  43, 226,		"BlueViolet",
	160,  32, 240,		"purple",
	147, 112, 219,		"medium purple",
	147, 112, 219,		"MediumPurple",
	216, 191, 216,		"thistle",
	255, 250, 250,		"snow1",
	238, 233, 233,		"snow2",
	205, 201, 201,		"snow3",
	139, 137, 137,		"snow4",
	255, 245, 238,		"seashell1",
	238, 229, 222,		"seashell2",
	205, 197, 191,		"seashell3",
	139, 134, 130,		"seashell4",
	255, 239, 219,		"AntiqueWhite1",
	238, 223, 204,		"AntiqueWhite2",
	205, 192, 176,		"AntiqueWhite3",
	139, 131, 120,		"AntiqueWhite4",
	255, 228, 196,		"bisque1",
	238, 213, 183,		"bisque2",
	205, 183, 158,		"bisque3",
	139, 125, 107,		"bisque4",
	255, 218, 185,		"PeachPuff1",
	238, 203, 173,		"PeachPuff2",
	205, 175, 149,		"PeachPuff3",
	139, 119, 101,		"PeachPuff4",
	255, 222, 173,		"NavajoWhite1",
	238, 207, 161,		"NavajoWhite2",
	205, 179, 139,		"NavajoWhite3",
	139, 121,  94,		"NavajoWhite4",
	255, 250, 205,		"LemonChiffon1",
	238, 233, 191,		"LemonChiffon2",
	205, 201, 165,		"LemonChiffon3",
	139, 137, 112,		"LemonChiffon4",
	255, 248, 220,		"cornsilk1",
	238, 232, 205,		"cornsilk2",
	205, 200, 177,		"cornsilk3",
	139, 136, 120,		"cornsilk4",
	255, 255, 240,		"ivory1",
	238, 238, 224,		"ivory2",
	205, 205, 193,		"ivory3",
	139, 139, 131,		"ivory4",
	240, 255, 240,		"honeydew1",
	224, 238, 224,		"honeydew2",
	193, 205, 193,		"honeydew3",
	131, 139, 131,		"honeydew4",
	255, 240, 245,		"LavenderBlush1",
	238, 224, 229,		"LavenderBlush2",
	205, 193, 197,		"LavenderBlush3",
	139, 131, 134,		"LavenderBlush4",
	255, 228, 225,		"MistyRose1",
	238, 213, 210,		"MistyRose2",
	205, 183, 181,		"MistyRose3",
	139, 125, 123,		"MistyRose4",
	240, 255, 255,		"azure1",
	224, 238, 238,		"azure2",
	193, 205, 205,		"azure3",
	131, 139, 139,		"azure4",
	131, 111, 255,		"SlateBlue1",
	122, 103, 238,		"SlateBlue2",
	105,  89, 205,		"SlateBlue3",
	71,  60, 139,		"SlateBlue4",
	72, 118, 255,		"RoyalBlue1",
	67, 110, 238,		"RoyalBlue2",
	58,  95, 205,		"RoyalBlue3",
	39,  64, 139,		"RoyalBlue4",
	0,   0, 255,		"blue1",
	0,   0, 238,		"blue2",
	0,   0, 205,		"blue3",
	0,   0, 139,		"blue4",
	30, 144, 255,		"DodgerBlue1",
	28, 134, 238,		"DodgerBlue2",
	24, 116, 205,		"DodgerBlue3",
	16,  78, 139,		"DodgerBlue4",
	99, 184, 255,		"SteelBlue1",
	92, 172, 238,		"SteelBlue2",
	79, 148, 205,		"SteelBlue3",
	54, 100, 139,		"SteelBlue4",
	0, 191, 255,		"DeepSkyBlue1",
	0, 178, 238,		"DeepSkyBlue2",
	0, 154, 205,		"DeepSkyBlue3",
	0, 104, 139,		"DeepSkyBlue4",
	135, 206, 255,		"SkyBlue1",
	126, 192, 238,		"SkyBlue2",
	108, 166, 205,		"SkyBlue3",
	74, 112, 139,		"SkyBlue4",
	176, 226, 255,		"LightSkyBlue1",
	164, 211, 238,		"LightSkyBlue2",
	141, 182, 205,		"LightSkyBlue3",
	96, 123, 139,		"LightSkyBlue4",
	198, 226, 255,		"SlateGray1",
	185, 211, 238,		"SlateGray2",
	159, 182, 205,		"SlateGray3",
	108, 123, 139,		"SlateGray4",
	202, 225, 255,		"LightSteelBlue1",
	188, 210, 238,		"LightSteelBlue2",
	162, 181, 205,		"LightSteelBlue3",
	110, 123, 139,		"LightSteelBlue4",
	191, 239, 255,		"LightBlue1",
	178, 223, 238,		"LightBlue2",
	154, 192, 205,		"LightBlue3",
	104, 131, 139,		"LightBlue4",
	224, 255, 255,		"LightCyan1",
	209, 238, 238,		"LightCyan2",
	180, 205, 205,		"LightCyan3",
	122, 139, 139,		"LightCyan4",
	187, 255, 255,		"PaleTurquoise1",
	174, 238, 238,		"PaleTurquoise2",
	150, 205, 205,		"PaleTurquoise3",
	102, 139, 139,		"PaleTurquoise4",
	152, 245, 255,		"CadetBlue1",
	142, 229, 238,		"CadetBlue2",
	122, 197, 205,		"CadetBlue3",
	83, 134, 139,		"CadetBlue4",
	0, 245, 255,		"turquoise1",
	0, 229, 238,		"turquoise2",
	0, 197, 205,		"turquoise3",
	0, 134, 139,		"turquoise4",
	0, 255, 255,		"cyan1",
	0, 238, 238,		"cyan2",
	0, 205, 205,		"cyan3",
	0, 139, 139,		"cyan4",
	151, 255, 255,		"DarkSlateGray1",
	141, 238, 238,		"DarkSlateGray2",
	121, 205, 205,		"DarkSlateGray3",
	82, 139, 139,		"DarkSlateGray4",
	127, 255, 212,		"aquamarine1",
	118, 238, 198,		"aquamarine2",
	102, 205, 170,		"aquamarine3",
	69, 139, 116,		"aquamarine4",
	193, 255, 193,		"DarkSeaGreen1",
	180, 238, 180,		"DarkSeaGreen2",
	155, 205, 155,		"DarkSeaGreen3",
	105, 139, 105,		"DarkSeaGreen4",
	84, 255, 159,		"SeaGreen1",
	78, 238, 148,		"SeaGreen2",
	67, 205, 128,		"SeaGreen3",
	46, 139,  87,		"SeaGreen4",
	154, 255, 154,		"PaleGreen1",
	144, 238, 144,		"PaleGreen2",
	124, 205, 124,		"PaleGreen3",
	84, 139,  84,		"PaleGreen4",
	0, 255, 127,		"SpringGreen1",
	0, 238, 118,		"SpringGreen2",
	0, 205, 102,		"SpringGreen3",
	0, 139,  69,		"SpringGreen4",
	0, 255,   0,		"green1",
	0, 238,   0,		"green2",
	0, 205,   0,		"green3",
	0, 139,   0,		"green4",
	127, 255,   0,		"chartreuse1",
	118, 238,   0,		"chartreuse2",
	102, 205,   0,		"chartreuse3",
	69, 139,   0,		"chartreuse4",
	192, 255,  62,		"OliveDrab1",
	179, 238,  58,		"OliveDrab2",
	154, 205,  50,		"OliveDrab3",
	105, 139,  34,		"OliveDrab4",
	202, 255, 112,		"DarkOliveGreen1",
	188, 238, 104,		"DarkOliveGreen2",
	162, 205,  90,		"DarkOliveGreen3",
	110, 139,  61,		"DarkOliveGreen4",
	255, 246, 143,		"khaki1",
	238, 230, 133,		"khaki2",
	205, 198, 115,		"khaki3",
	139, 134,  78,		"khaki4",
	255, 236, 139,		"LightGoldenrod1",
	238, 220, 130,		"LightGoldenrod2",
	205, 190, 112,		"LightGoldenrod3",
	139, 129,  76,		"LightGoldenrod4",
	255, 255, 224,		"LightYellow1",
	238, 238, 209,		"LightYellow2",
	205, 205, 180,		"LightYellow3",
	139, 139, 122,		"LightYellow4",
	255, 255,   0,		"yellow1",
	238, 238,   0,		"yellow2",
	205, 205,   0,		"yellow3",
	139, 139,   0,		"yellow4",
	255, 215,   0,		"gold1",
	238, 201,   0,		"gold2",
	205, 173,   0,		"gold3",
	139, 117,   0,		"gold4",
	255, 193,  37,		"goldenrod1",
	238, 180,  34,		"goldenrod2",
	205, 155,  29,		"goldenrod3",
	139, 105,  20,		"goldenrod4",
	255, 185,  15,		"DarkGoldenrod1",
	238, 173,  14,		"DarkGoldenrod2",
	205, 149,  12,		"DarkGoldenrod3",
	139, 101,   8,		"DarkGoldenrod4",
	255, 193, 193,		"RosyBrown1",
	238, 180, 180,		"RosyBrown2",
	205, 155, 155,		"RosyBrown3",
	139, 105, 105,		"RosyBrown4",
	255, 106, 106,		"IndianRed1",
	238,  99,  99,		"IndianRed2",
	205,  85,  85,		"IndianRed3",
	139,  58,  58,		"IndianRed4",
	255, 130,  71,		"sienna1",
	238, 121,  66,		"sienna2",
	205, 104,  57,		"sienna3",
	139,  71,  38,		"sienna4",
	255, 211, 155,		"burlywood1",
	238, 197, 145,		"burlywood2",
	205, 170, 125,		"burlywood3",
	139, 115,  85,		"burlywood4",
	255, 231, 186,		"wheat1",
	238, 216, 174,		"wheat2",
	205, 186, 150,		"wheat3",
	139, 126, 102,		"wheat4",
	255, 165,  79,		"tan1",
	238, 154,  73,		"tan2",
	205, 133,  63,		"tan3",
	139,  90,  43,		"tan4",
	255, 127,  36,		"chocolate1",
	238, 118,  33,		"chocolate2",
	205, 102,  29,		"chocolate3",
	139,  69,  19,		"chocolate4",
	255,  48,  48,		"firebrick1",
	238,  44,  44,		"firebrick2",
	205,  38,  38,		"firebrick3",
	139,  26,  26,		"firebrick4",
	255,  64,  64,		"brown1",
	238,  59,  59,		"brown2",
	205,  51,  51,		"brown3",
	139,  35,  35,		"brown4",
	255, 140, 105,		"salmon1",
	238, 130,  98,		"salmon2",
	205, 112,  84,		"salmon3",
	139,  76,  57,		"salmon4",
	255, 160, 122,		"LightSalmon1",
	238, 149, 114,		"LightSalmon2",
	205, 129,  98,		"LightSalmon3",
	139,  87,  66,		"LightSalmon4",
	255, 165,   0,		"orange1",
	238, 154,   0,		"orange2",
	205, 133,   0,		"orange3",
	139,  90,   0,		"orange4",
	255, 127,   0,		"DarkOrange1",
	238, 118,   0,		"DarkOrange2",
	205, 102,   0,		"DarkOrange3",
	139,  69,   0,		"DarkOrange4",
	255, 114,  86,		"coral1",
	238, 106,  80,		"coral2",
	205,  91,  69,		"coral3",
	139,  62,  47,		"coral4",
	255,  99,  71,		"tomato1",
	238,  92,  66,		"tomato2",
	205,  79,  57,		"tomato3",
	139,  54,  38,		"tomato4",
	255,  69,   0,		"OrangeRed1",
	238,  64,   0,		"OrangeRed2",
	205,  55,   0,		"OrangeRed3",
	139,  37,   0,		"OrangeRed4",
	255,   0,   0,		"red1",
	238,   0,   0,		"red2",
	205,   0,   0,		"red3",
	139,   0,   0,		"red4",
	255,  20, 147,		"DeepPink1",
	238,  18, 137,		"DeepPink2",
	205,  16, 118,		"DeepPink3",
	139,  10, 80 ,		"DeepPink4",
	255, 110, 180,		"HotPink1",
	238, 106, 167,		"HotPink2",
	205,  96, 144,		"HotPink3",
	139,  58,  98,		"HotPink4",
	255, 181, 197,		"pink1",
	238, 169, 184,		"pink2",
	205, 145, 158,		"pink3",
	139,  99, 108,		"pink4",
	255, 174, 185,		"LightPink1",
	238, 162, 173,		"LightPink2",
	205, 140, 149,		"LightPink3",
	139,  95, 101,		"LightPink4",
	255, 130, 171,		"PaleVioletRed1",
	238, 121, 159,		"PaleVioletRed2",
	205, 104, 137,		"PaleVioletRed3",
	139,  71, 93 ,		"PaleVioletRed4",
	255,  52, 179,		"maroon1",
	238,  48, 167,		"maroon2",
	205,  41, 144,		"maroon3",
	139,  28, 98 ,		"maroon4",
	255,  62, 150,		"VioletRed1",
	238,  58, 140,		"VioletRed2",
	205,  50, 120,		"VioletRed3",
	139,  34, 82 ,		"VioletRed4",
	255,   0, 255,		"magenta1",
	238,   0, 238,		"magenta2",
	205,   0, 205,		"magenta3",
	139,   0, 139,		"magenta4",
	255, 131, 250,		"orchid1",
	238, 122, 233,		"orchid2",
	205, 105, 201,		"orchid3",
	139,  71, 137,		"orchid4",
	255, 187, 255,		"plum1",
	238, 174, 238,		"plum2",
	205, 150, 205,		"plum3",
	139, 102, 139,		"plum4",
	224, 102, 255,		"MediumOrchid1",
	209,  95, 238,		"MediumOrchid2",
	180,  82, 205,		"MediumOrchid3",
	122,  55, 139,		"MediumOrchid4",
	191,  62, 255,		"DarkOrchid1",
	178,  58, 238,		"DarkOrchid2",
	154,  50, 205,		"DarkOrchid3",
	104,  34, 139,		"DarkOrchid4",
	155,  48, 255,		"purple1",
	145,  44, 238,		"purple2",
	125,  38, 205,		"purple3",
	85,  26, 139,		"purple4",
	171, 130, 255,		"MediumPurple1",
	159, 121, 238,		"MediumPurple2",
	137, 104, 205,		"MediumPurple3",
	93,  71, 139,		"MediumPurple4",
	255, 225, 255,		"thistle1",
	238, 210, 238,		"thistle2",
	205, 181, 205,		"thistle3",
	139, 123, 139,		"thistle4",
	0,   0,   0,		"gray0",
	0,   0,   0,		"grey0",
	3,   3,   3,		"gray1",
	3,   3,   3,		"grey1",
	5,   5,   5,		"gray2",
	5,   5,   5,		"grey2",
	8,   8,   8,		"gray3",
	8,   8,   8,		"grey3",
	10,  10,  10, 		"gray4",
	10,  10,  10, 		"grey4",
	13,  13,  13, 		"gray5",
	13,  13,  13, 		"grey5",
	15,  15,  15, 		"gray6",
	15,  15,  15, 		"grey6",
	18,  18,  18, 		"gray7",
	18,  18,  18, 		"grey7",
	20,  20,  20, 		"gray8",
	20,  20,  20, 		"grey8",
	23,  23,  23, 		"gray9",
	23,  23,  23, 		"grey9",
	26,  26,  26, 		"gray10",
	26,  26,  26, 		"grey10",
	28,  28,  28, 		"gray11",
	28,  28,  28, 		"grey11",
	31,  31,  31, 		"gray12",
	31,  31,  31, 		"grey12",
	33,  33,  33, 		"gray13",
	33,  33,  33, 		"grey13",
	36,  36,  36, 		"gray14",
	36,  36,  36, 		"grey14",
	38,  38,  38, 		"gray15",
	38,  38,  38, 		"grey15",
	41,  41,  41, 		"gray16",
	41,  41,  41, 		"grey16",
	43,  43,  43, 		"gray17",
	43,  43,  43, 		"grey17",
	46,  46,  46, 		"gray18",
	46,  46,  46, 		"grey18",
	48,  48,  48, 		"gray19",
	48,  48,  48, 		"grey19",
	51,  51,  51, 		"gray20",
	51,  51,  51, 		"grey20",
	54,  54,  54, 		"gray21",
	54,  54,  54, 		"grey21",
	56,  56,  56, 		"gray22",
	56,  56,  56, 		"grey22",
	59,  59,  59, 		"gray23",
	59,  59,  59, 		"grey23",
	61,  61,  61, 		"gray24",
	61,  61,  61, 		"grey24",
	64,  64,  64, 		"gray25",
	64,  64,  64, 		"grey25",
	66,  66,  66, 		"gray26",
	66,  66,  66, 		"grey26",
	69,  69,  69, 		"gray27",
	69,  69,  69, 		"grey27",
	71,  71,  71, 		"gray28",
	71,  71,  71, 		"grey28",
	74,  74,  74, 		"gray29",
	74,  74,  74, 		"grey29",
	77,  77,  77, 		"gray30",
	77,  77,  77, 		"grey30",
	79,  79,  79, 		"gray31",
	79,  79,  79, 		"grey31",
	82,  82,  82, 		"gray32",
	82,  82,  82, 		"grey32",
	84,  84,  84, 		"gray33",
	84,  84,  84, 		"grey33",
	87,  87,  87, 		"gray34",
	87,  87,  87, 		"grey34",
	89,  89,  89, 		"gray35",
	89,  89,  89, 		"grey35",
	92,  92,  92, 		"gray36",
	92,  92,  92, 		"grey36",
	94,  94,  94, 		"gray37",
	94,  94,  94, 		"grey37",
	97,  97,  97, 		"gray38",
	97,  97,  97, 		"grey38",
	99,  99,  99, 		"gray39",
	99,  99,  99, 		"grey39",
	102, 102, 102, 		"gray40",
	102, 102, 102, 		"grey40",
	105, 105, 105, 		"gray41",
	105, 105, 105, 		"grey41",
	107, 107, 107, 		"gray42",
	107, 107, 107, 		"grey42",
	110, 110, 110, 		"gray43",
	110, 110, 110, 		"grey43",
	112, 112, 112, 		"gray44",
	112, 112, 112, 		"grey44",
	115, 115, 115, 		"gray45",
	115, 115, 115, 		"grey45",
	117, 117, 117, 		"gray46",
	117, 117, 117, 		"grey46",
	120, 120, 120, 		"gray47",
	120, 120, 120, 		"grey47",
	122, 122, 122, 		"gray48",
	122, 122, 122, 		"grey48",
	125, 125, 125, 		"gray49",
	125, 125, 125, 		"grey49",
	127, 127, 127, 		"gray50",
	127, 127, 127, 		"grey50",
	130, 130, 130, 		"gray51",
	130, 130, 130, 		"grey51",
	133, 133, 133, 		"gray52",
	133, 133, 133, 		"grey52",
	135, 135, 135, 		"gray53",
	135, 135, 135, 		"grey53",
	138, 138, 138, 		"gray54",
	138, 138, 138, 		"grey54",
	140, 140, 140, 		"gray55",
	140, 140, 140, 		"grey55",
	143, 143, 143, 		"gray56",
	143, 143, 143, 		"grey56",
	145, 145, 145, 		"gray57",
	145, 145, 145, 		"grey57",
	148, 148, 148, 		"gray58",
	148, 148, 148, 		"grey58",
	150, 150, 150, 		"gray59",
	150, 150, 150, 		"grey59",
	153, 153, 153, 		"gray60",
	153, 153, 153, 		"grey60",
	156, 156, 156, 		"gray61",
	156, 156, 156, 		"grey61",
	158, 158, 158, 		"gray62",
	158, 158, 158, 		"grey62",
	161, 161, 161, 		"gray63",
	161, 161, 161, 		"grey63",
	163, 163, 163, 		"gray64",
	163, 163, 163, 		"grey64",
	166, 166, 166, 		"gray65",
	166, 166, 166, 		"grey65",
	168, 168, 168, 		"gray66",
	168, 168, 168, 		"grey66",
	171, 171, 171, 		"gray67",
	171, 171, 171, 		"grey67",
	173, 173, 173, 		"gray68",
	173, 173, 173, 		"grey68",
	176, 176, 176, 		"gray69",
	176, 176, 176, 		"grey69",
	179, 179, 179, 		"gray70",
	179, 179, 179, 		"grey70",
	181, 181, 181, 		"gray71",
	181, 181, 181, 		"grey71",
	184, 184, 184, 		"gray72",
	184, 184, 184, 		"grey72",
	186, 186, 186, 		"gray73",
	186, 186, 186, 		"grey73",
	189, 189, 189, 		"gray74",
	189, 189, 189, 		"grey74",
	191, 191, 191, 		"gray75",
	191, 191, 191, 		"grey75",
	194, 194, 194, 		"gray76",
	194, 194, 194, 		"grey76",
	196, 196, 196, 		"gray77",
	196, 196, 196, 		"grey77",
	199, 199, 199, 		"gray78",
	199, 199, 199, 		"grey78",
	201, 201, 201, 		"gray79",
	201, 201, 201, 		"grey79",
	204, 204, 204, 		"gray80",
	204, 204, 204, 		"grey80",
	207, 207, 207, 		"gray81",
	207, 207, 207, 		"grey81",
	209, 209, 209, 		"gray82",
	209, 209, 209, 		"grey82",
	212, 212, 212, 		"gray83",
	212, 212, 212, 		"grey83",
	214, 214, 214, 		"gray84",
	214, 214, 214, 		"grey84",
	217, 217, 217, 		"gray85",
	217, 217, 217, 		"grey85",
	219, 219, 219, 		"gray86",
	219, 219, 219, 		"grey86",
	222, 222, 222, 		"gray87",
	222, 222, 222, 		"grey87",
	224, 224, 224, 		"gray88",
	224, 224, 224, 		"grey88",
	227, 227, 227, 		"gray89",
	227, 227, 227, 		"grey89",
	229, 229, 229, 		"gray90",
	229, 229, 229, 		"grey90",
	232, 232, 232, 		"gray91",
	232, 232, 232, 		"grey91",
	235, 235, 235, 		"gray92",
	235, 235, 235, 		"grey92",
	237, 237, 237, 		"gray93",
	237, 237, 237, 		"grey93",
	240, 240, 240, 		"gray94",
	240, 240, 240, 		"grey94",
	242, 242, 242, 		"gray95",
	242, 242, 242, 		"grey95",
	245, 245, 245, 		"gray96",
	245, 245, 245, 		"grey96",
	247, 247, 247, 		"gray97",
	247, 247, 247, 		"grey97",
	250, 250, 250, 		"gray98",
	250, 250, 250, 		"grey98",
	252, 252, 252, 		"gray99",
	252, 252, 252, 		"grey99",
	255, 255, 255, 		"gray100",
	255, 255, 255, 		"grey100",
	169, 169, 169,		"dark grey",
	169, 169, 169,		"DarkGrey",
	169, 169, 169,		"dark gray",
	169, 169, 169,		"DarkGray",
	0  ,   0, 139,		"dark blue",
	0  ,   0, 139,		"DarkBlue",
	0  , 139, 139,		"dark cyan",
	0  , 139, 139,		"DarkCyan",
	139,   0, 139,		"dark magenta",
	139,   0, 139,		"DarkMagenta",
	139,   0,   0,		"dark red",
	139,   0,   0,		"DarkRed",
	144, 238, 144,		"light green",
	144, 238, 144,		"LightGreen",
	0,   0,   0,		NULL
    };
    

    static std::map<BString,long> cColorMap;
    if ( cColorMap.empty() ) {
		for ( int i = 0 ; asColTable[i].name != NULL ; ++i ) {
		    cColorMap[BString(asColTable[i].name).ToLower()] = (asColTable[i].r << 16) | (asColTable[i].r << 8) | (asColTable[i].b);
		}
    }
    std::map<BString,long>::iterator i = cColorMap.find( BString(pzName).ToLower() );
    if ( i != cColorMap.end() ) {
		*r = ((*i).second >> 16) & 0xff;
		*g = ((*i).second >> 8) & 0xff;
		*b = (*i).second & 0xff;
    } else {
		*r = *g = *b = 0;
    }
}
#endif __BEOS__

/*****************************************************************************
  QColor member functions
 *****************************************************************************/

bool QColor::color_init   = FALSE;		// color system not initialized
bool QColor::globals_init = FALSE;		// global color not initialized
bool QColor::lazy_alloc = TRUE;			// lazy color allocation


QColor* QColor::globalColors()
{
    return stdcol;
}


/*!
  Initializes the global colors.  This function is called if a global
  color variable is initialized before the constructors for our global
  color objects are executed.  Without this mechanism, assigning a
  color might assign an uninitialized value.

  Example:
  \code
     QColor myColor = red;			// will initialize red etc.

     int main( int argc, char **argc )
     {
     }
  \endcode
*/

void QColor::initGlobalColors()
{
    globals_init = TRUE;
    stdcol[ 0].pix = COLOR0_PIX;
    stdcol[ 1].pix = COLOR1_PIX;
#ifdef _WS_QWS_
    stdcol[ 0].rgbVal = 0;
    stdcol[ 1].rgbVal = 0x00ffffff; //######### QWS color allocation is a mess
#else
    stdcol[ 0].rgbVal = 0x00ffffff;
    stdcol[ 1].rgbVal = 0;
#endif
    stdcol[ 2].setRgb(   0,   0,   0 );
    stdcol[ 3].setRgb( 255, 255, 255 );
    stdcol[ 4].setRgb( 128, 128, 128 );
    stdcol[ 5].setRgb( 160, 160, 164 );
    stdcol[ 6].setRgb( 192, 192, 192 );
    stdcol[ 7].setRgb( 255,   0,   0 );
    stdcol[ 8].setRgb(   0, 255,   0 );
    stdcol[ 9].setRgb(   0,   0, 255 );
    stdcol[10].setRgb(   0, 255, 255 );
    stdcol[11].setRgb( 255,   0, 255 );
    stdcol[12].setRgb( 255, 255,   0 );
    stdcol[13].setRgb( 128,   0,   0 );
    stdcol[14].setRgb(   0, 128,   0 );
    stdcol[15].setRgb(   0,   0, 128 );
    stdcol[16].setRgb(   0, 128, 128 );
    stdcol[17].setRgb( 128,   0, 128 );
    stdcol[18].setRgb( 128, 128,   0 );
}


/*!
  \fn QColor::QColor()

  Constructs an invalid color with the RGB value (0,0,0). An invalid color
  is a color that is not properly set up for the underlying window system.

  \sa isValid()
*/


/*!
  \fn QColor::QColor( int r, int g, int b )

  Constructs a color with the RGB value \a (r,g,b).
  \a r, \a g and \a b must be in the range 0..255.

  \sa setRgb()
*/


/*!
  Constructs a color with a RGB value and a custom pixel value.

  If the \a pixel = 0xffffffff, then the color uses the RGB value in a
  standard way.	 If \a pixel is something else, then the pixel value will
  be set directly to \a pixel (skips the normal allocation procedure).
*/

QColor::QColor( QRgb rgb, uint pixel )
{
    if ( pixel == 0xffffffff ) {
	setRgb( rgb );
    } else {
	rgbVal = (rgb & RGB_MASK) | RGB_DIRECT;
	pix    = pixel;
    }
}


/*!
  Constructs a color with the RGB \e or HSV value \a (x,y,z).

  The \e (x,y,z) triplet defines an RGB value if \a colorSpec == \c
  QColor::Rgb.	\a x (red), \a y (green) and \a z (blue) must be in the
  range 0..255.

  The \a (x,y,z) triplet defines a HSV value if \a colorSpec == \c
  QColor::Hsv.	\a x (hue) must be in the range -1..360 (-1 means
  achromatic), and \a y (saturation) and \a z (value) must be in the range
  0..255.

  \sa setRgb(), setHsv()
*/

QColor::QColor( int x, int y, int z, Spec colorSpec )
{
    if ( colorSpec == Hsv )
	setHsv( x, y, z );
    else
	setRgb( x, y, z );
}


/*!
  Constructs a named color in the same way as setNamedColor().
  \sa setNamedColor()
*/

QColor::QColor( const QString& name )
{
    setNamedColor( name );
}


/*!
  Constructs a named color in the same way as setNamedColor().
  \sa setNamedColor()
*/

QColor::QColor( const char *name )
{
    setNamedColor( QString(name) );
}



/*!
  Constructs a color that is a copy of \a c.
*/

QColor::QColor( const QColor &c )
{
    if ( !globals_init )
	initGlobalColors();
    rgbVal = c.rgbVal;
    pix	   = c.pix;
}


/*!
  Assigns a copy of the color \c and returns a reference to this color.
*/

QColor &QColor::operator=( const QColor &c )
{
    if ( !globals_init )
	initGlobalColors();
    rgbVal = c.rgbVal;
    pix	   = c.pix;
    return *this;
}


/*!
  \fn bool QColor::isValid() const
  Returns FALSE if the color is invalid, i.e. it was constructed using the
  default constructor.
*/

/*!
  \fn bool QColor::isDirty() const
  Returns TRUE if the color is dirty, i.e. lazy allocation is enabled and
  an RGB/HSV value has been set but not allocated.
  \sa setLazyAlloc(), alloc(), pixel()
*/


/*!
  Returns the name of the color in the format #RRGGBB.

  \sa setNamedColor()
*/

QString QColor::name() const
{
    QString s;
    s.sprintf( "#%02x%02x%02x", red(), green(), blue() );
    return s;
}


static int hex2int( QChar hexchar )
{
    int v;
    if ( hexchar.isDigit() )
	v = hexchar.digitValue();
    else if ( hexchar >= 'A' && hexchar <= 'F' )
	v = hexchar.cell() - 'A' + 10;
    else if ( hexchar >= 'a' && hexchar <= 'f' )
	v = hexchar.cell() - 'a' + 10;
    else
	v = 0;
    return v;
}


/*!
  Sets the RGB value to that of the named color.

  The color name may be in one of five formats: <ul>
  <li> #RGB (each of R, G and B is a single hex digit)
  <li> #RRGGBB
  <li> #RRRGGGBBB
  <li> #RRRRGGGGBBBB
  <li> A named from the X color database (rgb.txt),  e.g.
  "steelblue" or "gainsboro".  These color names also work
  under Qt for Windows.
  </ul>
*/

void QColor::setNamedColor( const QString& name )
{
    if ( name.isEmpty() ) {
	setRgb( 0 );
    } else if ( name[0] == '#' ) {
	const QChar *p = name.unicode()+1;
	int len = name.length()-1;
	int r, g, b;
	if ( len == 12 ) {
	    r = (hex2int(p[0]) << 4) + hex2int(p[1]);
	    g = (hex2int(p[4]) << 4) + hex2int(p[5]);
	    b = (hex2int(p[8]) << 4) + hex2int(p[9]);
	} else if ( len == 9 ) {
	    r = (hex2int(p[0]) << 4) + hex2int(p[1]);
	    g = (hex2int(p[3]) << 4) + hex2int(p[4]);
	    b = (hex2int(p[6]) << 4) + hex2int(p[7]);
	} else if ( len == 6 ) {
	    r = (hex2int(p[0]) << 4) + hex2int(p[1]);
	    g = (hex2int(p[2]) << 4) + hex2int(p[3]);
	    b = (hex2int(p[4]) << 4) + hex2int(p[5]);
	} else if ( len == 3 ) {
	    r = (hex2int(p[0]) << 4) + hex2int(p[0]);
	    g = (hex2int(p[1]) << 4) + hex2int(p[1]);
	    b = (hex2int(p[2]) << 4) + hex2int(p[2]);
	} else {
	    r = g = b = 0;
	}
	setRgb( r, g, b );
    } else {
#ifdef __BEOS__
	int r, g, b;
	get_system_color( name.ascii(), &r, &g, &b );
	setRgb( r, g, b );
//	printf( "Warning: QColor::setNamedColor() does not call setSystemNamedColor()\n" );
#else
	setSystemNamedColor( name );
#endif
    }
}


#undef max
#undef min

/*! \obsolete */

void QColor::getHsv( int &h, int &s, int &v ) const
{
    hsv( &h, &s, &v );
}


/*!
  Returns the current RGB value as HSV.

  \arg \e *h, hue.
  \arg \e *s, saturation.
  \arg \e *v, value.

  The hue defines the color. Its range is 0..359 if the color is chromatic
  and -1 if the color is achromatic.  The saturation and value both vary
  between 0 and 255 inclusive.

  \sa setHsv(), rgb()
*/

void QColor::hsv( int *h, int *s, int *v ) const
{
    int r = qRed(rgbVal);
    int g = qGreen(rgbVal);
    int b = qBlue(rgbVal);
    uint max = r;				// maximum RGB component
    int whatmax = 0;				// r=>0, g=>1, b=>2
    if ( (uint)g > max ) {
	max = g;
	whatmax = 1;
    }
    if ( (uint)b > max ) {
	max = b;
	whatmax = 2;
    }
    uint min = r;				// find minimum value
    if ( (uint)g < min ) min = g;
    if ( (uint)b < min ) min = b;
    int delta = max-min;
    *v = max;					// calc value
    *s = max ? (510*delta+max)/(2*max) : 0;
    if ( *s == 0 ) {
	*h = -1;				// undefined hue
    } else {
	switch ( whatmax ) {
	    case 0:				// red is max component
		if ( g >= b )
		    *h = (120*(g-b)+delta)/(2*delta);
		else
		    *h = (120*(g-b+delta)+delta)/(2*delta) + 300;
		break;
	    case 1:				// green is max component
		if ( b > r )
		    *h = 120 + (120*(b-r)+delta)/(2*delta);
		else
		    *h = 60 + (120*(b-r+delta)+delta)/(2*delta);
		break;
	    case 2:				// blue is max component
		if ( r > g )
		    *h = 240 + (120*(r-g)+delta)/(2*delta);
		else
		    *h = 180 + (120*(r-g+delta)+delta)/(2*delta);
		break;
	}
    }
}


/*!
  Sets a HSV color value.

  \arg \e h, hue (-1,0..360).  -1 means achromatic.
  \arg \e s, saturation (0..255).
  \arg \e v, value (0..255).

  \sa hsv(), setRgb()
*/

void QColor::setHsv( int h, int s, int v )
{
#if defined(CHECK_RANGE)
    if ( h < -1 || (uint)s > 255 || (uint)v > 255 ) {
	qWarning( "QColor::setHsv: HSV parameters out of range" );
	return;
    }
#endif
    int r=v, g=v, b=v;
    if ( s == 0 || h == -1 ) {			// achromatic case
	// Ignore
    } else {					// chromatic case
	if ( (uint)h >= 360 )
	    h %= 360;
	uint f = h%60;
	h /= 60;
	uint p = (uint)(2*v*(255-s)+255)/510;
	uint q, t;
	if ( h&1 ) {
	    q = (uint)(2*v*(15300-s*f)+15300)/30600;
	    switch( h ) {
		case 1: r=(int)q; g=(int)v, b=(int)p; break;
		case 3: r=(int)p; g=(int)q, b=(int)v; break;
		case 5: r=(int)v; g=(int)p, b=(int)q; break;
	    }
	} else {
	    t = (uint)(2*v*(15300-(s*(60-f)))+15300)/30600;
	    switch( h ) {
		case 0: r=(int)v; g=(int)t, b=(int)p; break;
		case 2: r=(int)p; g=(int)v, b=(int)t; break;
		case 4: r=(int)t; g=(int)p, b=(int)v; break;
	    }
	}
    }
    setRgb( r, g, b );
}


/*!
  \fn QRgb QColor::rgb() const
  Returns the RGB value.

  The return type \e QRgb is equivalent to \c unsigned \c int.

  \sa setRgb(), hsv(), qRed(), qBlue(), qGreen()
*/

/*!
  Returns the red, green and blue components of the RGB value in
  \e *r, \e *g and \e *b.  The value range for a component is 0..255.
  \sa setRgb(), hsv()
*/

void QColor::rgb( int *r, int *g, int *b ) const
{
    *r = qRed(rgbVal);
    *g = qGreen(rgbVal);
    *b = qBlue(rgbVal);
}


/*!
  Sets the RGB value to \a (r,g,b).
  \a r, \a g and \a b must be in the range 0..255.
  \sa rgb(), setHsv()
*/

void QColor::setRgb( int r, int g, int b )
{
#if defined(CHECK_RANGE)
    if ( (uint)r > 255 || (uint)g > 255 || (uint)b > 255 )
	qWarning( "QColor::setRgb: RGB parameter(s) out of range" );
#endif
    rgbVal = ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
    if ( lazy_alloc || !color_init ) {
	rgbVal |= RGB_DIRTY;			// alloc later
	pix = 0;
    } else {
	alloc();				// alloc now
    }
}


/*!
  Sets the RGB value to \a rgb.

  The type \e QRgb is equivalent to \c unsigned \c int.

  \sa rgb(), setHsv()
*/

void QColor::setRgb( QRgb rgb )
{
    if ( lazy_alloc || !color_init ) {
	rgbVal = (rgb & RGB_MASK) | RGB_DIRTY;	// alloc later
	pix = 0;
    } else {
	rgbVal = (rgb & RGB_MASK);
	alloc();				// alloc now
    }
}

/*!
  \fn int QColor::red() const
  Returns the R (red) component of the RGB value.
*/


/*!
  \fn int QColor::green() const
  Returns the G (green) component of the RGB value.
*/

/*!
  \fn int QColor::blue() const
  Returns the B (blue) component of the RGB value.
*/


/*!
  Returns a lighter (or darker) color.

  Returns a lighter color if \e factor is greater than 100.
  Setting \e factor to 150 returns a color that is 50% brighter.

  Returns a darker color if \e factor is less than 100, equal to
  dark(10000 / \e factor).

  This function converts the current RGB color to HSV, multiplies V with
  \e factor and converts back to RGB.

  \sa dark()
*/

QColor QColor::light( int factor ) const
{
    if ( factor <= 0 )				// invalid lightness factor
	return *this;
    else if ( factor < 100 )			// makes color darker
	return dark( 10000/factor );

    int h, s, v;
    hsv( &h, &s, &v );
    v = (factor*v)/100;
    if ( v > 255 ) {				// overflow
	s -= v-255;				// adjust saturation
	if ( s < 0 )
	    s = 0;
	v = 255;
    }
    QColor c;
    c.setHsv( h, s, v );
    return c;
}


/*!
  Returns a darker (or lighter) color.

  Returns a darker color if \e factor is greater than 100.
  Setting \e factor to 300 returns a color that has
  one third the brightness.

  Returns a lighter color if \e factor is less than 100, equal to
  light(10000 / \e factor).

  This function converts the current RGB color to HSV, divides V by
  \e factor and converts back to RGB.

  \sa light()
*/

QColor QColor::dark( int factor ) const
{
    if ( factor <= 0 )				// invalid darkness factor
	return *this;
    else if ( factor < 100 )			// makes color lighter
	return light( 10000/factor );
    int h, s, v;
    hsv( &h, &s, &v );
    v = (v*100)/factor;
    QColor c;
    c.setHsv( h, s, v );
    return c;
}


/*!
  \fn bool QColor::operator==( const QColor &c ) const
  Returns TRUE if this color has the same RGB value as \e c,
  or FALSE if they have different RGB values.
*/

/*!
  \fn bool QColor::operator!=( const QColor &c ) const
  Returns TRUE if this color has different RGB value from \e c,
  or FALSE if they have equal RGB values.
*/


/*!
  \fn bool QColor::lazyAlloc()
  Returns TRUE if lazy color allocation is enabled (on-demand allocation),
  or FALSE if it is disabled (immediate allocation).
  \sa setLazyAlloc()
*/

/*!
  Enables or disables lazy color allocation.

  If lazy allocation is enabled, colors are allocated the first time they
  are used (upon calling the pixel() function).	 If lazy allocation is
  disabled, colors are allocated when they are constructed or when either
  setRgb() or setHsv() is called.

  Lazy color allocation is enabled by default.

  \sa lazyAlloc(), pixel(), alloc()
*/

void QColor::setLazyAlloc( bool enable )
{
    lazy_alloc = enable;
}


/*!
  \fn uint QColor::pixel() const
  Returns the pixel value.

  This value is used by the underlying window system to refer to a color.
  It can be thought of as an index into the display hardware's color table,
  but the value is an arbitrary 32-bit value.

  \sa setLazyAlloc(), alloc()
*/


/*****************************************************************************
  QColor stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
  \relates QColor
  Writes a color object to the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QColor &c )
{
    Q_UINT32 p = (Q_UINT32)c.rgb();
    if ( s.version() == 1 )			// Swap red and blue
	p = ((p << 16) & 0xff0000) | ((p >> 16) & 0xff) | (p & 0xff00ff00);
    return s << p;
}

/*!
  \relates QColor
  Reads a color object from the stream.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QColor &c )
{
    Q_UINT32 p;
    s >> p;
    if ( s.version() == 1 )			// Swap red and blue
	p = ((p << 16) & 0xff0000) | ((p >> 16) & 0xff) | (p & 0xff00ff00);
    c.setRgb( p );
    return s;
}
#endif


#ifdef __BEOS__
uint QColor::alloc()
{
    return( 0 );
}
#endif

/*****************************************************************************
  QColor global functions (documentation only)
 *****************************************************************************/

/*!
  \fn int qRed( QRgb rgb )
  \relates QColor
  Returns the red component of the RGB triplet \e rgb.
  \sa qRgb(), QColor::red()
*/

/*!
  \fn int qGreen( QRgb rgb )
  \relates QColor
  Returns the green component of the RGB triplet \e rgb.
  \sa qRgb(), QColor::green()
*/

/*!
  \fn int qBlue( QRgb rgb )
  \relates QColor
  Returns the blue component of the RGB triplet \e rgb.
  \sa qRgb(), QColor::blue()
*/

/*!
  \fn QRgb qRgb( int r, int g, int b )
  \relates QColor
  Returns the RGB triplet \a (r,g,b).

  The return type \e QRgb is equivalent to \c unsigned \c int.

  \sa qRgba(), qRed(), qGreen(), qBlue()
*/

/*!
  \fn QRgb qRgba( int r, int g, int b, int a )
  \relates QColor
  Returns the RGBA triplet \a (r,g,b,a).

  The return type \e QRgb is equivalent to \c unsigned \c int.

  \sa qRgb(), qRed(), qGreen(), qBlue()
*/

/*!
  \fn int qGray( int r, int g, int b )
  \relates QColor
  Returns a gray value 0..255 from the \a (r,g,b) triplet.

  The gray value is calculated using the formula:
  <code>(r*11 + g*16 + b*5)/32</code>.
*/

/*!
  \overload int qGray( qRgb rgb )
  \relates QColor
*/
