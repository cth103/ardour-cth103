// Generated by gtkmmproc -- DO NOT MODIFY!


#include <pangomm/attrlist.h>
#include <pangomm/private/attrlist_p.h>

// -*- c++ -*-
/* $Id$ */

/*
 *
 * Copyright 1998-1999 The Gtk-- Development Team
 * Copyright 2001      Free Software Foundation
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
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

namespace Pango
{

AttrList::AttrList(const Glib::ustring& markup_text, gunichar accel_marker)
{
  gboolean bTest = pango_parse_markup(markup_text.c_str(), -1 /* means null-terminated */, accel_marker,
                   	                  &gobject_, 0, 0, 0);
  if(bTest == FALSE)
    gobject_ = 0;
}

AttrList::operator bool()
{
  return gobj() != 0;
}

AttrList::AttrList(const Glib::ustring& markup_text, gunichar accel_marker, Glib::ustring& text, gunichar& accel_char)
{
  //initialize output parameters:
  text.erase();
  accel_char = 0;

  gchar* pchText = 0;
  gboolean bTest = pango_parse_markup(markup_text.c_str(), -1 /* means null-terminated */, accel_marker,
                   	                  &gobject_, &pchText, &accel_char, 0);
  if(bTest == FALSE)
  {
    gobject_ = 0;
  }
  else
  {
    text = pchText;
    g_free(pchText);
  }
}

void AttrList::insert(Attribute& attr)
{
  pango_attr_list_insert(gobj(), pango_attribute_copy(attr.gobj()));
}
 
void AttrList::insert_before(Attribute& attr)
{
  pango_attr_list_insert_before(gobj(), pango_attribute_copy(attr.gobj()));
}

void AttrList::change(Attribute& attr)
{
  pango_attr_list_change(gobj(), pango_attribute_copy(attr.gobj()));
}
 
} /* namespace Pango */

namespace
{
} // anonymous namespace


namespace Glib
{

Pango::AttrList wrap(PangoAttrList* object, bool take_copy)
{
  return Pango::AttrList(object, take_copy);
}

} // namespace Glib


namespace Pango
{


// static
GType AttrList::get_type()
{
  return pango_attr_list_get_type();
}

AttrList::AttrList()
:
  gobject_ (pango_attr_list_new())
{}

AttrList::AttrList(const AttrList& other)
:
  gobject_ ((other.gobject_) ? pango_attr_list_copy(other.gobject_) : 0)
{}

AttrList::AttrList(PangoAttrList* gobject, bool make_a_copy)
:
  // For BoxedType wrappers, make_a_copy is true by default.  The static
  // BoxedType wrappers must always take a copy, thus make_a_copy = true
  // ensures identical behaviour if the default argument is used.
  gobject_ ((make_a_copy && gobject) ? pango_attr_list_copy(gobject) : gobject)
{}

AttrList& AttrList::operator=(const AttrList& other)
{
  AttrList temp (other);
  swap(temp);
  return *this;
}

AttrList::~AttrList()
{
  if(gobject_)
    pango_attr_list_unref(gobject_);
}

void AttrList::swap(AttrList& other)
{
  PangoAttrList *const temp = gobject_;
  gobject_ = other.gobject_;
  other.gobject_ = temp;
}

PangoAttrList* AttrList::gobj_copy() const
{
  return pango_attr_list_copy(gobject_);
}


void AttrList::splice(AttrList& other, int pos, int len)
{
pango_attr_list_splice(gobj(), (other).gobj(), pos, len); 
}

AttrIter AttrList::get_iter()
{
  return Glib::wrap((pango_attr_list_get_iterator(gobj())));
}


} // namespace Pango


