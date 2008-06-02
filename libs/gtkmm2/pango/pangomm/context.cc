// Generated by gtkmmproc -- DO NOT MODIFY!


#include <pangomm/context.h>
#include <pangomm/private/context_p.h>

#include <pango/pango-enum-types.h>
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

#include <pango/pangocairo.h>
#include <pango/pango-types.h> //For PANGO_MATRIX_INIT

namespace Pango
{

Glib::ArrayHandle< Glib::RefPtr<FontFamily> > Context::list_families() const
{
  //Get array:
  PangoFontFamily** pFamilies = 0;
  int n_families = 0;
  pango_context_list_families(const_cast<PangoContext*>(gobj()), &pFamilies, &n_families);
  
  return Glib::ArrayHandle< Glib::RefPtr<FontFamily> >
      (pFamilies, n_families, Glib::OWNERSHIP_SHALLOW);
}

Pango::FontMetrics Context::get_metrics(const FontDescription& desc) const
{
  return FontMetrics(pango_context_get_metrics(const_cast<PangoContext*>(gobj()), desc.gobj(), 0));
}

ListHandle_Item Context::itemize(const Glib::ustring& text, const AttrList& attrs) const
{
  return ListHandle_Item(
      pango_itemize(const_cast<PangoContext*>(gobj()),
                    text.c_str(), 0, text.bytes(),
                    const_cast<PangoAttrList*>(attrs.gobj()), 0),
      Glib::OWNERSHIP_DEEP);
}

ListHandle_Item Context::itemize(const Glib::ustring& text, int start_index, int length,
                                 const AttrList& attrs, AttrIter& cached_iter) const
{
  return ListHandle_Item(
      pango_itemize(const_cast<PangoContext*>(gobj()),
                    text.c_str(), start_index, length,
                    const_cast<PangoAttrList*>(attrs.gobj()), cached_iter.gobj()),
      Glib::OWNERSHIP_DEEP);
}

void Context::update_from_cairo_context(const Cairo::RefPtr<Cairo::Context>& context)
{
  pango_cairo_update_context(context->cobj(), gobj());
}

Matrix Context::get_matrix() const
{
  const PangoMatrix* matrix = pango_context_get_matrix(const_cast<PangoContext*>(gobj()));
  if(matrix)
    return *matrix;
  else
  {
    PangoMatrix identity_transform = PANGO_MATRIX_INIT;
    return identity_transform;
  }
}

} /* namespace Pango */


namespace
{
} // anonymous namespace

// static
GType Glib::Value<Pango::Direction>::value_type()
{
  return pango_direction_get_type();
}

// static
GType Glib::Value<Pango::GravityHint>::value_type()
{
  return pango_gravity_hint_get_type();
}


namespace Glib
{

Glib::RefPtr<Pango::Context> wrap(PangoContext* object, bool take_copy)
{
  return Glib::RefPtr<Pango::Context>( dynamic_cast<Pango::Context*> (Glib::wrap_auto ((GObject*)(object), take_copy)) );
  //We use dynamic_cast<> in case of multiple inheritance.
}

} /* namespace Glib */


namespace Pango
{


/* The *_Class implementation: */

const Glib::Class& Context_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &Context_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(pango_context_get_type());

    // Add derived versions of interfaces, if the C type implements any interfaces:
  }

  return *this;
}

void Context_Class::class_init_function(void* g_class, void* class_data)
{
  BaseClassType *const klass = static_cast<BaseClassType*>(g_class);
  CppClassParent::class_init_function(klass, class_data);

#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED

#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
}

#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED

#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED


Glib::ObjectBase* Context_Class::wrap_new(GObject* object)
{
  return new Context((PangoContext*)object);
}


/* The implementation: */

PangoContext* Context::gobj_copy()
{
  reference();
  return gobj();
}

Context::Context(const Glib::ConstructParams& construct_params)
:
  Glib::Object(construct_params)
{}

Context::Context(PangoContext* castitem)
:
  Glib::Object((GObject*)(castitem))
{}

Context::~Context()
{}


Context::CppClassType Context::context_class_; // initialize static member

GType Context::get_type()
{
  return context_class_.init().get_type();
}

GType Context::get_base_type()
{
  return pango_context_get_type();
}


Context::Context()
:
  // Mark this class as non-derived to allow C++ vfuncs to be skipped.
  Glib::ObjectBase(0),
  Glib::Object(Glib::ConstructParams(context_class_.init()))
{
  }

Glib::RefPtr<FontMap> Context::get_font_map()
{
  return Glib::wrap(pango_context_get_font_map(gobj()));
}

Glib::RefPtr<const FontMap> Context::get_font_map() const
{
  return Glib::wrap(pango_context_get_font_map(const_cast<PangoContext*>(gobj())));
}

Glib::RefPtr<Font> Context::load_font(const FontDescription& desc) const
{
  return Glib::wrap(pango_context_load_font(const_cast<PangoContext*>(gobj()), (desc).gobj()));
}

Glib::RefPtr<Fontset> Context::load_fontset(const FontDescription& desc, const Language& language) const
{
  return Glib::wrap(pango_context_load_fontset(const_cast<PangoContext*>(gobj()), (desc).gobj(), const_cast<PangoLanguage*>((language).gobj())));
}

FontMetrics Context::get_metrics(const FontDescription& desc, const Language& language) const
{
  return FontMetrics((pango_context_get_metrics(const_cast<PangoContext*>(gobj()), (desc).gobj(), const_cast<PangoLanguage*>((language).gobj()))));
}

void Context::set_font_description(const FontDescription& desc)
{
pango_context_set_font_description(gobj(), (desc).gobj()); 
}

FontDescription Context::get_font_description() const
{
  return FontDescription((pango_context_get_font_description(const_cast<PangoContext*>(gobj()))));
}

Language Context::get_language() const
{
  return Language(pango_context_get_language(const_cast<PangoContext*>(gobj())));
}

void Context::set_language(const Language& language)
{
pango_context_set_language(gobj(), const_cast<PangoLanguage*>((language).gobj())); 
}

void Context::set_base_dir(Direction direction)
{
pango_context_set_base_dir(gobj(), ((PangoDirection)(direction))); 
}

Direction Context::get_base_dir() const
{
  return ((Direction)(pango_context_get_base_dir(const_cast<PangoContext*>(gobj()))));
}

void Context::set_base_gravity(Gravity gravity)
{
pango_context_set_base_gravity(gobj(), ((PangoGravity)(gravity))); 
}

Gravity Context::get_base_gravity() const
{
  return ((Gravity)(pango_context_get_base_gravity(const_cast<PangoContext*>(gobj()))));
}

Gravity Context::get_gravity() const
{
  return ((Gravity)(pango_context_get_gravity(const_cast<PangoContext*>(gobj()))));
}

void Context::set_gravity_hint(GravityHint hint)
{
pango_context_set_gravity_hint(gobj(), ((PangoGravityHint)(hint))); 
}

GravityHint Context::get_gravity_hint() const
{
  return ((GravityHint)(pango_context_get_gravity_hint(const_cast<PangoContext*>(gobj()))));
}

void Context::set_matrix(const Matrix& matrix)
{
pango_context_set_matrix(gobj(), &(matrix)); 
}

void Context::set_cairo_font_options(const Cairo::FontOptions& options)
{
pango_cairo_context_set_font_options(gobj(), (options).cobj()); 
}

Cairo::FontOptions Context::get_font_options() const
{
  return Cairo::FontOptions(const_cast< cairo_font_options_t*>(pango_cairo_context_get_font_options(const_cast<PangoContext*>(gobj()))), false /* take_copy */);
}

void Context::set_resolution(double dpi)
{
pango_cairo_context_set_resolution(gobj(), dpi); 
}

double Context::get_resolution() const
{
  return pango_cairo_context_get_resolution(const_cast<PangoContext*>(gobj()));
}


#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED


} // namespace Pango


