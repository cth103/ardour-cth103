// Generated by gtkmmproc -- DO NOT MODIFY!


#include <gtkmm/menu.h>
#include <gtkmm/private/menu_p.h>

// -*- c++ -*-
/* $Id$ */

/* Copyright 1998-2002 The gtkmm Development Team
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

#include <gtk/gtkdnd.h>
#include <gtk/gtkmenu.h>
#include <gtkmm/accelgroup.h>


static void SignalProxy_PopupPosition_gtk_callback(GtkMenu*, int* x, int* y, gboolean* push_in, void* data)
{
  Gtk::Menu::SlotPositionCalc* the_slot = static_cast<Gtk::Menu::SlotPositionCalc*>(data);

  int  temp_x = (x) ? *x : 0;
  int  temp_y = (y) ? *y : 0;
  bool temp_push_in = (push_in) ? bool(*push_in) : false;

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
  #endif //GLIBMM_EXCEPTIONS_ENABLED
    (*the_slot)(temp_x, temp_y, temp_push_in);
  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  }
  catch(...)
  {
    Glib::exception_handlers_invoke();
  }
  #endif //GLIBMM_EXCEPTIONS_ENABLED

  if(x) *x = temp_x;
  if(y) *y = temp_y;
  if(push_in) *push_in = temp_push_in;
}


namespace Gtk
{

void Menu::popup(const SlotPositionCalc& position_calc_slot, guint button, guint32 activate_time)
{
  // Tell GTK+ to call the static function with the slot's address as the extra
  // data, so that the static function can then call the sigc::slot:
  gtk_menu_popup(gobj(), 0, 0, &SignalProxy_PopupPosition_gtk_callback, const_cast<SlotPositionCalc*>(&position_calc_slot), button, activate_time);
}

void Menu::popup(MenuShell& parent_menu_shell, MenuItem& parent_menu_item, const SlotPositionCalc& position_calc_slot, guint button, guint32 activate_time)
{
  // Tell GTK+ to call the static function with the slot's address as the extra
  // data, so that the static function can then call the sigc::slot:
  gtk_menu_popup(gobj(), parent_menu_shell.Gtk::Widget::gobj(), parent_menu_item.Gtk::Widget::gobj(), &SignalProxy_PopupPosition_gtk_callback, const_cast<SlotPositionCalc*>(&position_calc_slot), button, activate_time);
}

void Menu::popup(guint button, guint32 activate_time)
{
  gtk_menu_popup(gobj(), 0, 0, 0, 0, button, activate_time);
}

void Menu::reorder_child(const MenuItem& child, int position)
{
  gtk_menu_reorder_child(
      gobj(),
      const_cast<GtkWidget*>(child.Gtk::Widget::gobj()),
      position);
}

void Menu::unset_accel_group()
{
  gtk_menu_set_accel_group(gobj(), 0);
}

void Menu::unset_title()
{
  gtk_menu_set_title(gobj(), 0);
}

void Menu::attach_to_widget(Widget& attach_widget)
{
  gtk_menu_attach_to_widget(gobj(), (attach_widget).gobj(), 0 /* allowed by the C docs. */);
}


} // namespace Gtk


namespace
{
} // anonymous namespace


namespace Glib
{

Gtk::Menu* wrap(GtkMenu* object, bool take_copy)
{
  return dynamic_cast<Gtk::Menu *> (Glib::wrap_auto ((GObject*)(object), take_copy));
}

} /* namespace Glib */

namespace Gtk
{


/* The *_Class implementation: */

const Glib::Class& Menu_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &Menu_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(gtk_menu_get_type());

    // Add derived versions of interfaces, if the C type implements any interfaces:
  }

  return *this;
}

void Menu_Class::class_init_function(void* g_class, void* class_data)
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


Glib::ObjectBase* Menu_Class::wrap_new(GObject* o)
{
  return manage(new Menu((GtkMenu*)(o)));

}


/* The implementation: */

Menu::Menu(const Glib::ConstructParams& construct_params)
:
  Gtk::MenuShell(construct_params)
{
  }

Menu::Menu(GtkMenu* castitem)
:
  Gtk::MenuShell((GtkMenuShell*)(castitem))
{
  }

Menu::~Menu()
{
  destroy_();
}

Menu::CppClassType Menu::menu_class_; // initialize static member

GType Menu::get_type()
{
  return menu_class_.init().get_type();
}

GType Menu::get_base_type()
{
  return gtk_menu_get_type();
}


Menu::Menu()
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::MenuShell(Glib::ConstructParams(menu_class_.init()))
{
  }

void Menu::reposition()
{
gtk_menu_reposition(gobj()); 
}

void Menu::popdown()
{
gtk_menu_popdown(gobj()); 
}

MenuItem* Menu::get_active()
{
  return Glib::wrap((GtkMenuItem*)(gtk_menu_get_active(gobj())));
}

const MenuItem* Menu::get_active() const
{
  return const_cast<Menu*>(this)->get_active();
}

void Menu::set_active(guint index)
{
gtk_menu_set_active(gobj(), index); 
}

void Menu::set_accel_group(const Glib::RefPtr<AccelGroup>& accel_group)
{
gtk_menu_set_accel_group(gobj(), Glib::unwrap(accel_group)); 
}

Glib::RefPtr<AccelGroup> Menu::get_accel_group()
{

  Glib::RefPtr<AccelGroup> retvalue = Glib::wrap(gtk_menu_get_accel_group(gobj()));
  if(retvalue)
    retvalue->reference(); //The function does not do a ref for us.
  return retvalue;

}

Glib::RefPtr<const AccelGroup> Menu::get_accel_group() const
{
  return const_cast<Menu*>(this)->get_accel_group();
}

void Menu::set_accel_path(const Glib::ustring& accel_path)
{
gtk_menu_set_accel_path(gobj(), accel_path.c_str()); 
}

void Menu::detach()
{
gtk_menu_detach(gobj()); 
}

Widget* Menu::get_attach_widget()
{
  return Glib::wrap(gtk_menu_get_attach_widget(gobj()));
}

const Widget* Menu::get_attach_widget() const
{
  return const_cast<Menu*>(this)->get_attach_widget();
}

void Menu::set_tearoff_state(bool torn_off)
{
gtk_menu_set_tearoff_state(gobj(), static_cast<int>(torn_off)); 
}

bool Menu::get_tearoff_state() const
{
  return gtk_menu_get_tearoff_state(const_cast<GtkMenu*>(gobj()));
}

void Menu::set_title(const Glib::ustring& title)
{
gtk_menu_set_title(gobj(), title.c_str()); 
}

Glib::ustring Menu::get_title() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_menu_get_title(const_cast<GtkMenu*>(gobj())));
}

void Menu::set_screen(const Glib::RefPtr<Gdk::Screen>& screen)
{
gtk_menu_set_screen(gobj(), Glib::unwrap(screen)); 
}

void Menu::attach(Gtk::Widget& child, guint left_attach, guint right_attach, guint top_attach, guint bottom_attach)
{
gtk_menu_attach(gobj(), (child).gobj(), left_attach, right_attach, top_attach, bottom_attach); 
}

void Menu::set_monitor(int monitor_num)
{
gtk_menu_set_monitor(gobj(), monitor_num); 
}

void Menu::attach_to_widget(Widget& attach_widget, GtkMenuDetachFunc detacher)
{
gtk_menu_attach_to_widget(gobj(), (attach_widget).gobj(), detacher); 
}


#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> Menu::property_tearoff_title() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "tearoff-title");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> Menu::property_tearoff_title() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "tearoff-title");
}
#endif //GLIBMM_PROPERTIES_ENABLED


#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED


} // namespace Gtk


