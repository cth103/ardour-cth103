// Generated by gtkmmproc -- DO NOT MODIFY!


#include <gtkmm/filechooserbutton.h>
#include <gtkmm/private/filechooserbutton_p.h>

// -*- c++ -*-
/* $Id$ */

/*
 *
 * Copyright 2003 The gtkmm Development Team
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

#include <gtk/gtkfilechooserbutton.h>

namespace Gtk
{

FileChooserButton::FileChooserButton(FileChooserAction action)
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::HBox(Glib::ConstructParams(filechooserbutton_class_.init(), "action",action, (char*) 0))
{
}
  

} // namespace Gtk


namespace
{
} // anonymous namespace


namespace Glib
{

Gtk::FileChooserButton* wrap(GtkFileChooserButton* object, bool take_copy)
{
  return dynamic_cast<Gtk::FileChooserButton *> (Glib::wrap_auto ((GObject*)(object), take_copy));
}

} /* namespace Glib */

namespace Gtk
{


/* The *_Class implementation: */

const Glib::Class& FileChooserButton_Class::init()
{
  if(!gtype_) // create the GType if necessary
  {
    // Glib::Class has to know the class init function to clone custom types.
    class_init_func_ = &FileChooserButton_Class::class_init_function;

    // This is actually just optimized away, apparently with no harm.
    // Make sure that the parent type has been created.
    //CppClassParent::CppObjectType::get_type();

    // Create the wrapper type, with the same class/instance size as the base type.
    register_derived_type(gtk_file_chooser_button_get_type());

    // Add derived versions of interfaces, if the C type implements any interfaces:
  FileChooser::add_interface(get_type());
  }

  return *this;
}

void FileChooserButton_Class::class_init_function(void* g_class, void* class_data)
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


Glib::ObjectBase* FileChooserButton_Class::wrap_new(GObject* o)
{
  return manage(new FileChooserButton((GtkFileChooserButton*)(o)));

}


/* The implementation: */

FileChooserButton::FileChooserButton(const Glib::ConstructParams& construct_params)
:
  Gtk::HBox(construct_params)
{
  }

FileChooserButton::FileChooserButton(GtkFileChooserButton* castitem)
:
  Gtk::HBox((GtkHBox*)(castitem))
{
  }

FileChooserButton::~FileChooserButton()
{
  destroy_();
}

FileChooserButton::CppClassType FileChooserButton::filechooserbutton_class_; // initialize static member

GType FileChooserButton::get_type()
{
  return filechooserbutton_class_.init().get_type();
}

GType FileChooserButton::get_base_type()
{
  return gtk_file_chooser_button_get_type();
}


FileChooserButton::FileChooserButton(const Glib::ustring& title, FileChooserAction action)
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::HBox(Glib::ConstructParams(filechooserbutton_class_.init(), "title", title.c_str(), "action", ((GtkFileChooserAction)(action)), (char*) 0))
{
  }

FileChooserButton::FileChooserButton(const Glib::ustring& title, FileChooserAction action, const Glib::ustring& backend)
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::HBox(Glib::ConstructParams(filechooserbutton_class_.init(), "title", title.c_str(), "action", ((GtkFileChooserAction)(action)), "backend", backend.c_str(), (char*) 0))
{
  }

FileChooserButton::FileChooserButton(FileChooserDialog& dialog)
:
  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
  Gtk::HBox(Glib::ConstructParams(filechooserbutton_class_.init(), "dialog", (dialog).Gtk::Widget::gobj(), (char*) 0))
{
  }

Glib::ustring FileChooserButton::get_title() const
{
  return Glib::convert_const_gchar_ptr_to_ustring(gtk_file_chooser_button_get_title(const_cast<GtkFileChooserButton*>(gobj())));
}

void FileChooserButton::set_title(const Glib::ustring& title)
{
gtk_file_chooser_button_set_title(gobj(), title.c_str()); 
}

int FileChooserButton::get_width_chars() const
{
  return gtk_file_chooser_button_get_width_chars(const_cast<GtkFileChooserButton*>(gobj()));
}

void FileChooserButton::set_width_chars(int n_chars)
{
gtk_file_chooser_button_set_width_chars(gobj(), n_chars); 
}

bool FileChooserButton::get_focus_on_click() const
{
  return gtk_file_chooser_button_get_focus_on_click(const_cast<GtkFileChooserButton*>(gobj()));
}

void FileChooserButton::set_focus_on_click(gboolean focus_on_click)
{
gtk_file_chooser_button_set_focus_on_click(gobj(), focus_on_click); 
}


#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<FileChooserDialog*> FileChooserButton::property_dialog() const
{
  return Glib::PropertyProxy_ReadOnly<FileChooserDialog*>(this, "dialog");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<bool> FileChooserButton::property_focus_on_click() 
{
  return Glib::PropertyProxy<bool>(this, "focus-on-click");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<bool> FileChooserButton::property_focus_on_click() const
{
  return Glib::PropertyProxy_ReadOnly<bool>(this, "focus-on-click");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<Glib::ustring> FileChooserButton::property_title() 
{
  return Glib::PropertyProxy<Glib::ustring>(this, "title");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<Glib::ustring> FileChooserButton::property_title() const
{
  return Glib::PropertyProxy_ReadOnly<Glib::ustring>(this, "title");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy<int> FileChooserButton::property_width_chars() 
{
  return Glib::PropertyProxy<int>(this, "width-chars");
}
#endif //GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
Glib::PropertyProxy_ReadOnly<int> FileChooserButton::property_width_chars() const
{
  return Glib::PropertyProxy_ReadOnly<int>(this, "width-chars");
}
#endif //GLIBMM_PROPERTIES_ENABLED


#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED


} // namespace Gtk


