// Generated by gtkmmproc -- DO NOT MODIFY!

#include <gtkmm/layout.h>
#include <gtkmm/private/layout_p.h>

// -*- c++ -*-
/* $Id$ */

/* 
 *
 * Copyright 1998-2002 The gtkmm Development Team
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

#include <gtk/gtklayout.h>

namespace Gtk
{

/* layout.cc
 * 
 * Copyright (C) 1998 EMC Capital Management Inc.
 * Developed by Havoc Pennington <hp@pobox.com>
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

Layout::Layout()
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::Container(Glib::ConstructParams(layout_class_.init(), (char*) 0))
{
  set_hadjustment();
  set_vadjustment();
}

Layout::Layout(Adjustment& hadjustment, Adjustment& vadjustment)
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::Container(Glib::ConstructParams(layout_class_.init(), (char*) 0))
{
  set_hadjustment(hadjustment);
  set_vadjustment(vadjustment);
}

void Layout::set_hadjustment()
{
  gtk_layout_set_hadjustment(gobj(), 0);
}

void Layout::set_vadjustment()
{
  gtk_layout_set_vadjustment(gobj(), 0);
}

} // namespace Gtk


namespace
{

void Layout_signal_set_scroll_adjustments_callback(GtkLayout* self, GtkAdjustment* p0,GtkAdjustment* p1,void* data)
{
  using namespace Gtk;
  typedef sigc::slot< void,Adjustment*,Adjustment* > SlotType;

  // Do not try to call a signal on a disassociated wrapper.
  if(Glib::ObjectBase::_get_current_wrapper((GObject*) self))
  {
    try
    {
      if(sigc::slot_base *const slot = Glib::SignalProxyNormal::data_to_slot(data))
        (*static_cast<SlotType*>(slot))(Glib::wrap(p0)
, Glib::wrap(p1)
);
    }
    catch(...)
    {
      Glib::exception_handlers_invoke();
    }
  }
}

const Glib::SignalProxyInfo Layout_signal_set_scroll_adjustments_info =
{
  "set_scroll_adjustments",
  (GCallback) &Layout_signal_set_scroll_adjustments_callback,
  (GCallback) &Layout_signal_set_scroll_adjustments_callback
};

} // anonymous namespace


namespace Glib
{

Gtk::Layout* wrap(GtkLayout* object, bool take_copy)
{
  return dynamic_cast<Gtk::Layout *> (Glib::wrap_auto ((GObject*)(object), take_copy));
}

} /* namespace Glib */

namespace Gtk
{


/* The *_Class implementation: */

const Glib::Class& Layout_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &Layout_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(gtk_layout_get_type());

    // Add derived versions of interfaces, if the C type implements any interfaces:
  }

  return *this;
}

void Layout_Class::class_init_function(void* g_class, void* class_data)
{
  BaseClassType *const klass = static_cast<BaseClassType*>(g_class);
  CppClassParent::class_init_function(klass, class_data);

  klass->set_scroll_adjustments = &set_scroll_adjustments_callback;
}


void Layout_Class::set_scroll_adjustments_callback(GtkLayout* self, GtkAdjustment* p0, GtkAdjustment* p1)
{
  CppObjectType *const obj = dynamic_cast<CppObjectType*>(
      Glib::ObjectBase::_get_current_wrapper((GObject*)self));

  // Non-gtkmmproc-generated custom classes implicitly call the default
  // Glib::ObjectBase constructor, which sets is_derived_. But gtkmmproc-
  // generated classes can use this optimisation, which avoids the unnecessary
  // parameter conversions if there is no possibility of the virtual function
  // being overridden:
  if(obj && obj->is_derived_())
  {
    try // Trap C++ exceptions which would normally be lost because this is a C callback.
    {
      // Call the virtual member method, which derived classes might override.
      obj->on_set_scroll_adjustments(Glib::wrap(p0)
, Glib::wrap(p1)
);
    }
    catch(...)
    {
      Glib::exception_handlers_invoke();
    }
  }
  else
  {
    BaseClassType *const base = static_cast<BaseClassType*>(
        g_type_class_peek_parent(G_OBJECT_GET_CLASS(self)) // Get the parent class of the object class (The original underlying C class).
    );

    // Call the original underlying C function:
    if(base && base->set_scroll_adjustments)
      (*base->set_scroll_adjustments)(self, p0, p1);
  }
}


Glib::ObjectBase* Layout_Class::wrap_new(GObject* o)
{
  return manage(new Layout((GtkLayout*)(o)));

}


/* The implementation: */

Layout::Layout(const Glib::ConstructParams& construct_params)
:
  Gtk::Container(construct_params)
{
  }

Layout::Layout(GtkLayout* castitem)
:
  Gtk::Container((GtkContainer*)(castitem))
{
  }

Layout::~Layout()
{
  destroy_();
}

Layout::CppClassType Layout::layout_class_; // initialize static member

GType Layout::get_type()
{
  return layout_class_.init().get_type();
}

GType Layout::get_base_type()
{
  return gtk_layout_get_type();
}


void Layout::put(Widget& child_widget, int x, int y)
{
  gtk_layout_put(gobj(), (child_widget).gobj(), x, y);
}

void Layout::move(Widget& child_widget, int x, int y)
{
  gtk_layout_move(gobj(), (child_widget).gobj(), x, y);
}

void Layout::set_size(guint width, guint height)
{
  gtk_layout_set_size(gobj(), width, height);
}

void Layout::get_size(guint& width, guint& height) const
{
  gtk_layout_get_size(const_cast<GtkLayout*>(gobj()), &(width), &(height));
}

void Layout::set_hadjustment(Adjustment& adjustment)
{
  gtk_layout_set_hadjustment(gobj(), (adjustment).gobj());
}

Adjustment* Layout::get_hadjustment()
{
  return Glib::wrap(gtk_layout_get_hadjustment(gobj()));
}

const Adjustment* Layout::get_hadjustment() const
{
  return Glib::wrap(gtk_layout_get_hadjustment(const_cast<GtkLayout*>(gobj())));
}

void Layout::set_vadjustment(Adjustment& adjustment)
{
  gtk_layout_set_vadjustment(gobj(), (adjustment).gobj());
}

Adjustment* Layout::get_vadjustment()
{
  return Glib::wrap(gtk_layout_get_vadjustment(gobj()));
}

const Adjustment* Layout::get_vadjustment() const
{
  return Glib::wrap(gtk_layout_get_vadjustment(const_cast<GtkLayout*>(gobj())));
}

Glib::RefPtr<Gdk::Window> Layout::get_bin_window()
{
  Glib::RefPtr<Gdk::Window> ref_ptr(Glib::wrap((GdkWindowObject*)(gobj()->bin_window)));

  if(ref_ptr)
    ref_ptr->reference();

  return ref_ptr;
}

Glib::RefPtr<const Gdk::Window> Layout::get_bin_window() const
{
  Glib::RefPtr<const Gdk::Window> ref_ptr(Glib::wrap((GdkWindowObject*)(gobj()->bin_window)));

  if(ref_ptr)
    ref_ptr->reference();

  return ref_ptr;
}


Glib::SignalProxy2< void,Adjustment*,Adjustment* > Layout::signal_set_scroll_adjustments()
{
  return Glib::SignalProxy2< void,Adjustment*,Adjustment* >(this, &Layout_signal_set_scroll_adjustments_info);
}


Glib::PropertyProxy<Adjustment*> Layout::property_hadjustment() 
{
  return Glib::PropertyProxy<Adjustment*>(this, "hadjustment");
}

Glib::PropertyProxy_ReadOnly<Adjustment*> Layout::property_hadjustment() const
{
  return Glib::PropertyProxy_ReadOnly<Adjustment*>(this, "hadjustment");
}

Glib::PropertyProxy<Adjustment*> Layout::property_vadjustment() 
{
  return Glib::PropertyProxy<Adjustment*>(this, "vadjustment");
}

Glib::PropertyProxy_ReadOnly<Adjustment*> Layout::property_vadjustment() const
{
  return Glib::PropertyProxy_ReadOnly<Adjustment*>(this, "vadjustment");
}

Glib::PropertyProxy<guint> Layout::property_width() 
{
  return Glib::PropertyProxy<guint>(this, "width");
}

Glib::PropertyProxy_ReadOnly<guint> Layout::property_width() const
{
  return Glib::PropertyProxy_ReadOnly<guint>(this, "width");
}

Glib::PropertyProxy<guint> Layout::property_height() 
{
  return Glib::PropertyProxy<guint>(this, "height");
}

Glib::PropertyProxy_ReadOnly<guint> Layout::property_height() const
{
  return Glib::PropertyProxy_ReadOnly<guint>(this, "height");
}


void Gtk::Layout::on_set_scroll_adjustments(Adjustment* hadj, Adjustment* vadj)
{
  BaseClassType *const base = static_cast<BaseClassType*>(
      g_type_class_peek_parent(G_OBJECT_GET_CLASS(gobject_)) // Get the parent class of the object class (The original underlying C class).
  );

  if(base && base->set_scroll_adjustments)
    (*base->set_scroll_adjustments)(gobj(),(GtkAdjustment*)Glib::unwrap(hadj),(GtkAdjustment*)Glib::unwrap(vadj));
}


} // namespace Gtk


