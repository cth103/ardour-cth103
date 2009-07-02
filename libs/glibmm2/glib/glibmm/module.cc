// Generated by gtkmmproc -- DO NOT MODIFY!


#include <glibmm/module.h>
#include <glibmm/private/module_p.h>

// -*- c++ -*-
/* $Id: module.ccg,v 1.2 2004/04/09 14:49:44 murrayc Exp $ */

/* Copyright (C) 2002 The gtkmm Development Team
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

#include <glibmm/utility.h>
#include <gmodule.h>

namespace Glib
{

Module::Module(const std::string& file_name, ModuleFlags flags)
:
  gobject_ (g_module_open(file_name.c_str(), (GModuleFlags) flags))
{}

Module::~Module()
{
  if(gobject_)
    g_module_close(gobject_);
}

Module::operator bool() const
{
  return (gobject_ != 0);
}

} // namespace Glib


namespace
{
} // anonymous namespace


namespace Glib
{


bool Module::get_supported()
{
  return g_module_supported();
}


void Module::make_resident()
{
g_module_make_resident(gobj()); 
}

std::string Module::get_last_error()
{
  return Glib::convert_const_gchar_ptr_to_stdstring(g_module_error());
}


bool Module::get_symbol(const std::string& symbol_name, void*& symbol) const
{
  return g_module_symbol(const_cast<GModule*>(gobj()), symbol_name.c_str(), &(symbol));
}

std::string Module::get_name() const
{
  return Glib::convert_const_gchar_ptr_to_stdstring(g_module_name(const_cast<GModule*>(gobj())));
}

std::string Module::build_path(const std::string& directory, const std::string& module_name)
{
  return Glib::convert_return_gchar_ptr_to_stdstring(g_module_build_path(directory.c_str(), module_name.c_str()));
}


} // namespace Glib


