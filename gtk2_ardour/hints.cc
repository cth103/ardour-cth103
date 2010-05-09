#include <gtkmm/box.h>
#include "ardour_ui.h"
#include "hints.h"
#include "i18n.h"

Hints::Hints ()
{
	_frame.set_shadow_type (Gtk::SHADOW_ETCHED_IN);

	Gtk::VBox* b = Gtk::manage (new Gtk::VBox);
	b->set_border_width (4);
	b->pack_start (_label);
	
	_frame.add (*b);

	_label.set_alignment (0, 0.5);
	_label.set_use_markup ();
	_label.set_markup (_("<b>Welcome to Ardour!</b>"));

	ARDOUR_UI::instance()->tipped_widget_entered.connect (sigc::mem_fun (*this, &Hints::set));
}

Gtk::Widget&
Hints::widget ()
{
	return _frame;
}

void
Hints::set (std::string const & h)
{
	_label.set_markup (h);
}
