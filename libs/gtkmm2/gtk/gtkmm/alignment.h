// -*- c++ -*-
// Generated by gtkmmproc -- DO NOT MODIFY!
#ifndef _GTKMM_ALIGNMENT_H
#define _GTKMM_ALIGNMENT_H


#include <glibmm.h>

/* $Id$ */

/* alignment.h
 * 
 * Copyright (C) 1998-2002 The gtkmm Development Team
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

#include <gtkmm/bin.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct _GtkAlignment GtkAlignment;
typedef struct _GtkAlignmentClass GtkAlignmentClass;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


namespace Gtk
{ class Alignment_Class; } // namespace Gtk
namespace Gtk
{

/** A widget which controls the alignment and size of its child.
 *
 * Normally, a widget is allocated at least as much size as it requests, and,
 * most widgets expand to fill any extra allocated space, but sometimes
 * this behavior is not desired. The alignment widget allows the
 * programmer to specify how a widget should expand and position itself
 * to fill the area it is allocated.
 *
 * It has four settings: xscale, yscale, xalign, and yalign:
 * The scale settings specify how much the child widget should expand to fill the space allocated to the Gtk::Alignment. The values can range from 0 (meaning the child doesn't expand at all) to 1 (meaning the child expands to fill all of the available space).
 * The align settings place the child widget within the available area. The values range from 0 (top or left) to 1 (bottom or right). Of course, if the scale settings are both set to 1, the alignment settings have no effect.
 *
 * @ingroup Widgets
 * @ingroup Containers
 */

class Alignment : public Bin
{
  public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef Alignment CppObjectType;
  typedef Alignment_Class CppClassType;
  typedef GtkAlignment BaseObjectType;
  typedef GtkAlignmentClass BaseClassType;
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  virtual ~Alignment();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

private:
  friend class Alignment_Class;
  static CppClassType alignment_class_;

  // noncopyable
  Alignment(const Alignment&);
  Alignment& operator=(const Alignment&);

protected:
  explicit Alignment(const Glib::ConstructParams& construct_params);
  explicit Alignment(GtkAlignment* castitem);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  static GType get_type()      G_GNUC_CONST;
  static GType get_base_type() G_GNUC_CONST;
#endif

  ///Provides access to the underlying C GtkObject.
  GtkAlignment*       gobj()       { return reinterpret_cast<GtkAlignment*>(gobject_); }

  ///Provides access to the underlying C GtkObject.
  const GtkAlignment* gobj() const { return reinterpret_cast<GtkAlignment*>(gobject_); }


public:
  //C++ methods used to invoke GTK+ virtual functions:
#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED

protected:
  //GTK+ Virtual Functions (override these to change behaviour):
#ifdef GLIBMM_VFUNCS_ENABLED
#endif //GLIBMM_VFUNCS_ENABLED

  //Default Signal Handlers::
#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED


private:

public:

  /** Constructor to create an Alignment object.
   * @param xalign The initial horizontal alignment of the child.
   * @param yalign The initial vertical alignment of the child.
   * @param xscale The initial amount that the child expands horizontally to fill up unused space.
   * @param yscale The initial amount that the child expands vertically to fill up unused space.
   */
  explicit Alignment(float xalign = 0.5, float yalign = 0.5, float xscale = 1.0, float yscale = 1.0);

  /** Constructor to create an Alignment object.
   * @param xalign A Gtk::AlignmentEnum describing the initial horizontal alignment of the child.
   * @param yalign A Gtk::AlignmentEnum describing the initial vertical alignment of the child.
   * @param xscale The initial amount that the child expands horizontally to fill up unused space.
   * @param yscale The initial amount that the child expands vertically to fill up unused space.
   */
  explicit Alignment(AlignmentEnum xalign, AlignmentEnum yalign = Gtk::ALIGN_CENTER, float xscale = 1.0, float yscale = 1.0);

  
  /** Sets the Alignment values.
   * @param xalign The horizontal alignment of the child of this Alignment, from 0 (left) to 1 (right).
   * @param yalign The vertical alignment of the child of this Alignment, from 0 (top) to 1 (bottom).
   * @param xscale The amount that the child expands horizontally to fill up unused space, from 0 to 1.  A value of 0 indicates that the child widget should never expand.  A value of 1 indicates that the child widget will expand to fill all the space allocated for the Alignment.
   * @param yscale The amount that the child widget expands vertically to fill up unused space from 0 to 1.  The values are similar to @a xscale .
   */
  void set(float xalign = 0.5, float yalign = 0.5, float xscale = 1.0, float yscale=  1.0);
  
  /** Sets the Alignment values.
   * @param xalign The horizontal alignment of the child of this Alignment, from 0 (left) to 1 (right).
   * @param yalign The vertical alignment of the child of this Alignment, from 0 (top) to 1 (bottom).
   * @param xscale The amount that the child expands horizontally to fill up unused space, from 0 to 1.  A value of 0 indicates that the child widget should never expand.  A value of 1 indicates that the child widget will expand to fill all the space allocated for the Alignment.
   * @param yscale The amount that the child widget expands vertically to fill up unused space from 0 to 1.  The values are similar to @a xscale .
   */
  void set(AlignmentEnum xalign, AlignmentEnum yalign = Gtk::ALIGN_CENTER, float xscale = 1.0, float yscale=  1.0);

  //New in GTK+ 2.4
  
  /** Sets the padding on the different sides of the widget.
   * The padding adds blank space to the sides of the widget. For instance,
   * this can be used to indent the child widget towards the right by adding
   * padding on the left.
   * 
   * @newin2p4
   * @param padding_top The padding at the top of the widget.
   * @param padding_bottom The padding at the bottom of the widget.
   * @param padding_left The padding at the left of the widget.
   * @param padding_right The padding at the right of the widget.
   */
  void set_padding(guint padding_top, guint padding_bottom, guint padding_left, guint padding_right);
  
  /** Gets the padding on the different sides of the widget.
   * See set_padding().
   * 
   * @newin2p4
   * @param padding_top Location to store the padding for the top of the widget, or <tt>0</tt>.
   * @param padding_bottom Location to store the padding for the bottom of the widget, or <tt>0</tt>.
   * @param padding_left Location to store the padding for the left of the widget, or <tt>0</tt>.
   * @param padding_right Location to store the padding for the right of the widget, or <tt>0</tt>.
   */
  void get_padding(guint& padding_top, guint& padding_bottom, guint& padding_left, guint& padding_right);
              
  #ifdef GLIBMM_PROPERTIES_ENABLED
/** Horizontal position of child in available space. 0.0 is left aligned
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<float> property_xalign() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** Horizontal position of child in available space. 0.0 is left aligned
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<float> property_xalign() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** Vertical position of child in available space. 0.0 is top aligned
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<float> property_yalign() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** Vertical position of child in available space. 0.0 is top aligned
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<float> property_yalign() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** If available horizontal space is bigger than needed for the child
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<float> property_xscale() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** If available horizontal space is bigger than needed for the child
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<float> property_xscale() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** If available vertical space is bigger than needed for the child
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<float> property_yscale() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** If available vertical space is bigger than needed for the child
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<float> property_yscale() const;
#endif //#GLIBMM_PROPERTIES_ENABLED


  //New in GTK+ 2.4
  #ifdef GLIBMM_PROPERTIES_ENABLED
/** The padding to insert at the top of the widget.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<guint> property_top_padding() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** The padding to insert at the top of the widget.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<guint> property_top_padding() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** The padding to insert at the bottom of the widget.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<guint> property_bottom_padding() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** The padding to insert at the bottom of the widget.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<guint> property_bottom_padding() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** The padding to insert at the left of the widget.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<guint> property_left_padding() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** The padding to insert at the left of the widget.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<guint> property_left_padding() const;
#endif //#GLIBMM_PROPERTIES_ENABLED

  #ifdef GLIBMM_PROPERTIES_ENABLED
/** The padding to insert at the right of the widget.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy<guint> property_right_padding() ;
#endif //#GLIBMM_PROPERTIES_ENABLED

#ifdef GLIBMM_PROPERTIES_ENABLED
/** The padding to insert at the right of the widget.
   *
   * You rarely need to use properties because there are get_ and set_ methods for almost all of them.
   * @return A PropertyProxy that allows you to get or set the property of the value, or receive notification when
   * the value of the property changes.
   */
  Glib::PropertyProxy_ReadOnly<guint> property_right_padding() const;
#endif //#GLIBMM_PROPERTIES_ENABLED


};

} /* namespace Gtk */


namespace Glib
{
  /** A Glib::wrap() method for this object.
   * 
   * @param object The C instance.
   * @param take_copy False if the result should take ownership of the C instance. True if it should take a new copy or ref.
   * @result A C++ instance that wraps this C instance.
   *
   * @relates Gtk::Alignment
   */
  Gtk::Alignment* wrap(GtkAlignment* object, bool take_copy = false);
} //namespace Glib


#endif /* _GTKMM_ALIGNMENT_H */

