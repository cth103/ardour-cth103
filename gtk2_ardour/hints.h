#include <gtkmm/label.h>
#include <gtkmm/frame.h>

class Hints
{
public:

	Hints ();

	void set (std::string const &);
	
	Gtk::Widget& widget ();
	
private:

	Gtk::Frame _frame;
	Gtk::Label _label;
};
