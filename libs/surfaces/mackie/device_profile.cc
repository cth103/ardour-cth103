/*
	Copyright (C) 2006,2007 John Anderson
	Copyright (C) 2012 Paul Davis

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <cstdlib>
#include <cstring>
#include <glibmm/miscutils.h>

#include "pbd/xml++.h"
#include "pbd/error.h"
#include "pbd/pathscanner.h"

#include "ardour/filesystem_paths.h"

#include "mackie_control_protocol.h"
#include "device_profile.h"

#include "i18n.h"

using namespace Mackie;
using namespace PBD;
using namespace ARDOUR;
using std::string;
using std::vector;

std::map<std::string,DeviceProfile> DeviceProfile::device_profiles;

DeviceProfile::DeviceProfile (const string& n)
	: _name (n)
{
}

DeviceProfile::~DeviceProfile()
{
}

static const char * const devprofile_env_variable_name = "ARDOUR_MCP_PATH";
static const char* const devprofile_dir_name = "mcp";
static const char* const devprofile_suffix = ".profile";

static sys::path
system_devprofile_search_path ()
{
	bool devprofile_path_defined = false;
        sys::path spath_env (Glib::getenv (devprofile_env_variable_name, devprofile_path_defined));

	if (devprofile_path_defined) {
		return spath_env;
	}

	SearchPath spath (system_data_search_path());
	spath.add_subdirectory_to_paths(devprofile_dir_name);

	// just return the first directory in the search path that exists
	SearchPath::const_iterator i = std::find_if(spath.begin(), spath.end(), sys::exists);

	if (i == spath.end()) return sys::path();

	return *i;
}

static sys::path
user_devprofile_directory ()
{
	sys::path p(user_config_directory());
	p /= devprofile_dir_name;

	return p;
}

static bool
devprofile_filter (const string &str, void */*arg*/)
{
	return (str.length() > strlen(devprofile_suffix) &&
		str.find (devprofile_suffix) == (str.length() - strlen (devprofile_suffix)));
}

void
DeviceProfile::reload_device_profiles ()
{
	DeviceProfile dp;
	vector<string> s;
	vector<string *> *devprofiles;
	PathScanner scanner;
	SearchPath spath (system_devprofile_search_path());
	spath += user_devprofile_directory ();

	devprofiles = scanner (spath.to_string(), devprofile_filter, 0, false, true);
	device_profiles.clear ();

	if (!devprofiles) {
		error << "No MCP device info files found using " << spath.to_string() << endmsg;
		std::cerr << "No MCP device info files found using " << spath.to_string() << std::endl;
		return;
	}

	if (devprofiles->empty()) {
		error << "No MCP device info files found using " << spath.to_string() << endmsg;
		std::cerr << "No MCP device info files found using " << spath.to_string() << std::endl;
		return;
	}

	for (vector<string*>::iterator i = devprofiles->begin(); i != devprofiles->end(); ++i) {
		string fullpath = *(*i);

		XMLTree tree;

		std::cerr << "Loading " << fullpath << std::endl;
		
		if (!tree.read (fullpath.c_str())) {
			std::cerr << "XML read failed\n";
			continue;
		}

		XMLNode* root = tree.root ();
		if (!root) {
			std::cerr << "no root\n";
			continue;
		}

		if (dp.set_state (*root, 3000) == 0) { /* version is ignored for now */
			std::cerr << "saved profile " << dp.name() << std::endl;
			device_profiles[dp.name()] = dp;
		}
	}

	delete devprofiles;
}

int
DeviceProfile::set_state (const XMLNode& node, int /* version */)
{
	const XMLProperty* prop;
	const XMLNode* child;

	if (node.name() != "MackieDeviceProfile") {
		std::cerr << "wrong root\n";
		return -1;
	}

	/* name is mandatory */
 
	if ((child = node.child ("Name")) == 0 || (prop = child->property ("value")) == 0) {
		std::cerr << "no name\n";
		return -1;
	} else {
		_name = prop->value();
		std::cerr << "got name " << _name << std::endl;
	}

	if ((child = node.child ("Buttons")) != 0) {
		XMLNodeConstIterator i;
		const XMLNodeList& nlist (child->children());

		for (i = nlist.begin(); i != nlist.end(); ++i) {

			if ((*i)->name() == "Button") {

				std::cerr << "foudn a button\n";

				if ((prop = (*i)->property ("name")) == 0) {
					error << string_compose ("Button without name in device profile \"%1\" - ignored", _name) << endmsg;
					continue;
				}

				int id = Button::name_to_id (prop->value());
				if (id < 0) {
					error << string_compose ("Unknow button ID \"%1\"", prop->value()) << endmsg;
					continue;
				}

				Button::ID bid = (Button::ID) id;

				ButtonActionMap::iterator b = _button_map.find (bid);

				if (b == _button_map.end()) {
					b = _button_map.insert (_button_map.end(), std::pair<Button::ID,ButtonActions> (bid, ButtonActions()));
				}

				std::cerr << "checking bindings for button " << bid << std::endl;

				if ((prop = (*i)->property ("plain")) != 0) {
					std::cerr << "Loaded binding between " << bid << " and " << prop->value() << std::endl;
					b->second.plain = prop->value ();
				}
				if ((prop = (*i)->property ("control")) != 0) {
					b->second.control = prop->value ();
				}
				if ((prop = (*i)->property ("shift")) != 0) {
					b->second.shift = prop->value ();
				}
				if ((prop = (*i)->property ("option")) != 0) {
					b->second.option = prop->value ();
				}
				if ((prop = (*i)->property ("cmdalt")) != 0) {
					b->second.cmdalt = prop->value ();
				}
				if ((prop = (*i)->property ("shiftcontrol")) != 0) {
					b->second.shiftcontrol = prop->value ();
				}
			}
		}
	} else {
		std::cerr << " no buttons\n";
	}

	return 0;
}

XMLNode&
DeviceProfile::get_state () const
{
	XMLNode* node = new XMLNode ("MackieDeviceProfile");

	node->add_property ("name", _name);

	if (_button_map.empty()) {
		return *node;
	}

	XMLNode* buttons = new XMLNode ("Buttons");
	node->add_child_nocopy (*buttons);

	for (ButtonActionMap::const_iterator b = _button_map.begin(); b != _button_map.end(); ++b) {
		XMLNode* n = new XMLNode ("Button");

		n->add_property ("name", b->first);

		if (!b->second.plain.empty()) {
			n->add_property ("plain", b->second.plain);
		}
		if (!b->second.control.empty()) {
			n->add_property ("control", b->second.control);
		}
		if (!b->second.shift.empty()) {
			n->add_property ("shift", b->second.shift);
		}
		if (!b->second.option.empty()) {
			n->add_property ("option", b->second.option);
		}
		if (!b->second.cmdalt.empty()) {
			n->add_property ("cmdalt", b->second.cmdalt);
		}
		if (!b->second.shiftcontrol.empty()) {
			n->add_property ("shiftcontrol", b->second.shiftcontrol);
		}

		buttons->add_child_nocopy (*n);
	}

	return *node;
}

string
DeviceProfile::get_button_action (Button::ID id, int modifier_state) const
{
	ButtonActionMap::const_iterator i = _button_map.find (id);

	if (i == _button_map.end()) {
		return string();
	}

	if (modifier_state == MackieControlProtocol::MODIFIER_CONTROL) {
		return i->second.control;
	} else if (modifier_state == MackieControlProtocol::MODIFIER_SHIFT) {
		return i->second.shift;
	} else if (modifier_state == MackieControlProtocol::MODIFIER_OPTION) {
		return i->second.option;
	} else if (modifier_state == MackieControlProtocol::MODIFIER_CMDALT) {
		return i->second.cmdalt;
	} else if (modifier_state == (MackieControlProtocol::MODIFIER_CONTROL|MackieControlProtocol::MODIFIER_SHIFT)) {
		return i->second.shiftcontrol;
	}

	return i->second.plain;
}

void
DeviceProfile::set_button_action (Button::ID id, int modifier_state, const string& action)
{
	ButtonActionMap::iterator i = _button_map.find (id);

	if (i == _button_map.end()) {
		return;
	}

	if (modifier_state == MackieControlProtocol::MODIFIER_CONTROL) {
		i->second.control = action;
	} else if (modifier_state == MackieControlProtocol::MODIFIER_SHIFT) {
		i->second.shift = action;
	} else if (modifier_state == MackieControlProtocol::MODIFIER_OPTION) {
		i->second.option = action;
	} else if (modifier_state == MackieControlProtocol::MODIFIER_CMDALT) {
		i->second.cmdalt = action;
	} else if (modifier_state == (MackieControlProtocol::MODIFIER_CONTROL|MackieControlProtocol::MODIFIER_SHIFT)) {
		i->second.shiftcontrol = action;
	}

	if (modifier_state == 0) {
		i->second.plain = action;
	}
}

const string&
DeviceProfile::name() const
{
	return _name;
}
