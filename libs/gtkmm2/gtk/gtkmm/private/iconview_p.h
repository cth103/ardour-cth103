// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_ICONVIEW_P_H
#define _GTKMM_ICONVIEW_P_H


#include <gtkmm/private/container_p.h>

#include <glibmm/class.h>

namespace Gtk
{

class IconView_Class : public Glib::Class
{
public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef IconView CppObjectType;
  typedef GtkIconView BaseObjectType;
  typedef GtkIconViewClass BaseClassType;
  typedef Gtk::Container_Class CppClassParent;
  typedef GtkContainerClass BaseClassParent;

  friend class IconView;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  const Glib::Class& init();

  static void class_init_function(void* g_class, void* class_data);

  static Glib::ObjectBase* wrap_new(GObject*);

protected:

#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  //Callbacks (default signal handlers):
  //These will call the *_impl member methods, which will then call the existing default signal callbacks, if any.
  //You could prevent the original default signal handlers being called by overriding the *_impl method.
  static void set_scroll_adjustments_callback(GtkIconView* self, GtkAdjustment* p0, GtkAdjustment* p1);
  static void item_activated_callback(GtkIconView* self, GtkTreePath* p0);
  static void selection_changed_callback(GtkIconView* self);
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

  //Callbacks (virtual functions):
#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED
};


} // namespace Gtk


#endif /* _GTKMM_ICONVIEW_P_H */

