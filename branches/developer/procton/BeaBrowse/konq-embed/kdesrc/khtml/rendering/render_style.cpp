/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
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
 *
 * $Id: render_style.cpp,v 1.1.1.1 2002/04/27 11:01:55 procton Exp $
 */

#include "render_style.h"

#include "kdebug.h"

using namespace khtml;

StyleSurroundData::StyleSurroundData()
{
    margin.left = Length(0,Fixed);
    margin.right = Length(0,Fixed);
    margin.top = Length(0,Fixed);
    margin.bottom = Length(0,Fixed);
    padding.left = Length(0,Fixed);
    padding.right = Length(0,Fixed);
    padding.top = Length(0,Fixed);
    padding.bottom = Length(0,Fixed);
}

StyleSurroundData::StyleSurroundData(const StyleSurroundData& o )
        : SharedData()
{
    offset = o.offset;
    margin = o.margin;
    padding = o.padding;
    border = o.border;
}

bool StyleSurroundData::operator==(const StyleSurroundData& o) const
{
    return offset==o.offset && margin==o.margin &&
	padding==o.padding && border==o.border;
}

StyleBoxData::StyleBoxData()
{
}

StyleBoxData::StyleBoxData(const StyleBoxData& o )
        : SharedData()
{
    width = o.width;
    height = o.height;
    min_width = o.min_width;
    max_width = o.max_width;
    min_height = o.min_height;
    max_height = o.max_height;
    z_index = o.z_index;
}

bool StyleBoxData::operator==(const StyleBoxData& o) const
{
    return
	    width == o.width &&
	    height == o.height &&
	    min_width == o.min_width &&
	    max_width == o.max_width &&
	    min_height == o.min_height &&
	    max_height == o.max_height &&
	    z_index == o.z_index;
}



void
RenderStyle::setBitDefaults()
{
    _border_collapse = true;
    _empty_cells = SHOW;
    _caption_side = CAPTOP;
    _list_style_type = DISC;
    _list_style_position = OUTSIDE;
    _visiblity = VISIBLE;
    _text_align = JUSTIFY;
    _direction = LTR;
    _text_decoration = TDNONE;
    _white_space = NORMAL;

    _vertical_align = BASELINE;
    _clear = CNONE;
    _overflow = OVISIBLE;
    _table_layout = TAUTO;
    _position = STATIC;
    _floating = FNONE;
    _bg_repeat = REPEAT;
    _bg_attachment = SCROLL;

    _visuallyOrdered = false;
    _direction = LTR;
    _cursor = CURSOR_AUTO;

    _htmlHacks=false;
    _flowAroundFloats=false;

    _hasHover = false;
    _hasFocus = false;
    _hasActive = false;
}



RenderStyle::RenderStyle()
{
//    counter++;
    if (!_default)
	_default = new RenderStyle(true);

    box = _default->box;
    visual = _default->visual;
    background = _default->background;
    surround = _default->surround;

    inherited = _default->inherited;

    setBitDefaults();

    _styleType=NOPSEUDO;
    pseudoStyle = 0;

    _display = INLINE;

}

RenderStyle::RenderStyle(bool)
{
    setBitDefaults();

    box.init();
    box.access()->setDefaultValues();
    visual.init();
    background.init();
    surround.init();

    inherited.init();
    inherited.access()->setDefaultValues();

    _styleType=NOPSEUDO;
    pseudoStyle = 0;

}

RenderStyle::RenderStyle(const RenderStyle& other)
    : DOM::DomShared() // shut up, compiler
{

    _display = other._display;

    _border_collapse = other._border_collapse;
    _empty_cells = other._empty_cells;
    _caption_side = other._caption_side;
    _list_style_type = other._list_style_type;
    _list_style_position = other._list_style_position;
    _visiblity = other._visiblity;
    _text_align = other._text_align;
    _direction = other._direction;
    _white_space = other._white_space;
    _text_decoration = other._text_decoration;
    _visuallyOrdered = other._visuallyOrdered;
    _cursor = other._cursor;

    _htmlHacks = other._htmlHacks;

    _overflow = other._overflow;
    _vertical_align = other._vertical_align;
    _clear = other._clear;
    _table_layout = other._table_layout;
    _bg_repeat = other._bg_repeat;
    _bg_attachment = other._bg_attachment;
    _position = other._position;
    _floating = other._floating;

    _flowAroundFloats = other._flowAroundFloats;

    _styleType=NOPSEUDO;

//    counter++;
    box = other.box;
    visual = other.visual;
    background = other.background;
    surround = other.surround;

    inherited = other.inherited;

    _hasHover = other._hasHover;
    _hasFocus = other._hasFocus;
    _hasActive = other._hasActive;

    pseudoStyle=0;

}

RenderStyle::RenderStyle(const RenderStyle* inheritParent)
{
    _styleType=NOPSEUDO;
    pseudoStyle = 0;

    if (!_default)
	_default = new RenderStyle(true);
//    counter++;
    box = _default->box;
    visual = _default->visual;
    surround = _default->surround;
    background = _default->background;

    inherited = inheritParent->inherited;

    setBitDefaults();

    _border_collapse = inheritParent->_border_collapse;
    _empty_cells = inheritParent->_empty_cells;
    _caption_side = inheritParent->_caption_side;
    _list_style_type = inheritParent->_list_style_type;
    _list_style_position = inheritParent->_list_style_position;
    _visiblity = inheritParent->_visiblity;
    _text_align = inheritParent->_text_align;
    _direction = inheritParent->_direction;
    _text_decoration = inheritParent->_text_decoration;
    _white_space = inheritParent->_white_space;
    _visuallyOrdered = inheritParent->_visuallyOrdered;
    _cursor = inheritParent->_cursor;

    _htmlHacks = inheritParent->_htmlHacks;

    _display = INLINE;

//    kdDebug( 6040 ) << "style=" << counter << " data="
//        << SharedData::counter << endl;
}

RenderStyle::~RenderStyle()
{
    RenderStyle *ps = pseudoStyle;
    RenderStyle *prev = 0;

    while (ps) {
        prev = ps;
        ps = ps->pseudoStyle;
	// to prevent a double deletion. 
	// this works only because the styles below aren't really shared
	// Dirk said we need another construct as soon as these are shared
	prev->pseudoStyle = 0;
        prev->deref();
    }
}

bool RenderStyle::operator==(const RenderStyle& other) const
{
    return
        *box.get() == *other.box.get() &&
        *visual.get() == *other.visual.get() &&
    	*background.get() == *other.background.get() &&
        *surround.get() == *other.surround.get() &&

    	*inherited.get() == *other.inherited.get() &&
        _display == other._display;
}

RenderStyle* RenderStyle::getPseudoStyle(PseudoId pid)
{
    RenderStyle *ps = pseudoStyle;

    while (ps) {
        if (ps->_styleType==pid) return ps;
        ps = ps->pseudoStyle;
    }

    return 0;
}

RenderStyle* RenderStyle::addPseudoStyle(PseudoId pid)
{
    RenderStyle *ps = getPseudoStyle(pid);

    if (!ps)
    {
        ps = new RenderStyle(*this); // use the real copy constructor to get an identical copy
        ps->ref();
        ps->_styleType = pid;
        ps->pseudoStyle = pseudoStyle;

        pseudoStyle = ps;
    }

    return ps;
}

void RenderStyle::removePseudoStyle(PseudoId pid)
{
    RenderStyle *ps = pseudoStyle;
    RenderStyle *prev = this;

    while (ps) {
        if (ps->_styleType==pid) {
            prev->pseudoStyle = ps->pseudoStyle;
            ps->deref();
            return;
        }
        prev = ps;
        ps = ps->pseudoStyle;
    }
}



RenderStyle* RenderStyle::_default = 0;
//int RenderStyle::counter = 0;
//int SharedData::counter = 0;

void RenderStyle::cleanup()
{
    delete _default;
    _default = 0;
//    counter = 0;
//    SharedData::counter = 0;
}
