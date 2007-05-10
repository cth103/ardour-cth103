// Generated by gtkmmproc -- DO NOT MODIFY!

#include <pangomm/renderer.h>
#include <pangomm/private/renderer_p.h>

#include <pango/pango-enum-types.h>
// -*- c++ -*-
/* $Id$ */

/* 
 *
 * Copyright 2004 The gtkmm Development Team
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

#include <pango/pango-renderer.h>

namespace Pango
{

} /* namespace Pango */


namespace
{
} // anonymous namespace

// static
GType Glib::Value<Pango::RenderPart>::value_type()
{
  return pango_render_part_get_type();
}


namespace Glib
{

Glib::RefPtr<Pango::Renderer> wrap(PangoRenderer* object, bool take_copy)
{
  return Glib::RefPtr<Pango::Renderer>( dynamic_cast<Pango::Renderer*> (Glib::wrap_auto ((GObject*)(object), take_copy)) );
  //We use dynamic_cast<> in case of multiple inheritance.
}

} /* namespace Glib */


namespace Pango
{


/* The *_Class implementation: */

const Glib::Class& Renderer_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &Renderer_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(pango_renderer_get_type());

    // Add derived versions of interfaces, if the C type implements any interfaces:
  }

  return *this;
}

void Renderer_Class::class_init_function(void* g_class, void* class_data)
{
  BaseClassType *const klass = static_cast<BaseClassType*>(g_class);
  CppClassParent::class_init_function(klass, class_data);

}


Glib::ObjectBase* Renderer_Class::wrap_new(GObject* object)
{
  return new Renderer((PangoRenderer*)object);
}


/* The implementation: */

PangoRenderer* Renderer::gobj_copy()
{
  reference();
  return gobj();
}

Renderer::Renderer(const Glib::ConstructParams& construct_params)
:
  Glib::Object(construct_params)
{}

Renderer::Renderer(PangoRenderer* castitem)
:
  Glib::Object((GObject*)(castitem))
{}

Renderer::~Renderer()
{}


Renderer::CppClassType Renderer::renderer_class_; // initialize static member

GType Renderer::get_type()
{
  return renderer_class_.init().get_type();
}

GType Renderer::get_base_type()
{
  return pango_renderer_get_type();
}


void Renderer::draw_layout(const Glib::RefPtr<Layout>& layout, int x, int y)
{
  pango_renderer_draw_layout(gobj(), Glib::unwrap(layout), x, y);
}

void Renderer::draw_layout_line(const Glib::RefPtr<LayoutLine>& line, int x, int y)
{
  pango_renderer_draw_layout_line(gobj(), Glib::unwrap(line), x, y);
}

void Renderer::draw_glyphs(const Glib::RefPtr<Font>& font, const GlyphString& glyphs, int x, int y)
{
  pango_renderer_draw_glyphs(gobj(), Glib::unwrap(font), const_cast<PangoGlyphString*>(glyphs.gobj()), x, y);
}

void Renderer::draw_rectangle(RenderPart part, int x, int y, int width, int height)
{
  pango_renderer_draw_rectangle(gobj(), ((PangoRenderPart)(part)), x, y, width, height);
}

void Renderer::draw_error_underline(int x, int y, int width, int height)
{
  pango_renderer_draw_error_underline(gobj(), x, y, width, height);
}

void Renderer::draw_trapezoid(RenderPart part, double y1, double x11, double x21, double y2, double x12, double x22)
{
  pango_renderer_draw_trapezoid(gobj(), ((PangoRenderPart)(part)), y1, x11, x21, y2, x12, x22);
}

void Renderer::draw_glyph(const Glib::RefPtr<Font>& font, Glyph glyph, double x, double y)
{
  pango_renderer_draw_glyph(gobj(), Glib::unwrap(font), glyph, x, y);
}

void Renderer::activate()
{
  pango_renderer_activate(gobj());
}

void Renderer::deactivate()
{
  pango_renderer_deactivate(gobj());
}

void Renderer::part_changed(RenderPart part)
{
  pango_renderer_part_changed(gobj(), ((PangoRenderPart)(part)));
}

void Renderer::set_color(RenderPart part, const Color& color)
{
  pango_renderer_set_color(gobj(), ((PangoRenderPart)(part)), (color).gobj());
}

Color Renderer::get_color(RenderPart part) const
{
  return Color(pango_renderer_get_color(const_cast<PangoRenderer*>(gobj()), ((PangoRenderPart)(part))));
}

void Renderer::set_matrix(const Matrix& matrix)
{
  pango_renderer_set_matrix(gobj(), &(matrix));
}


} // namespace Pango


