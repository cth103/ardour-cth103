/*
    Copyright (C) 2000-2007 Paul Davis 

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

#ifndef __ardour_plugin_manager_h__
#define __ardour_plugin_manager_h__

#include <list>
#include <map>
#include <string>

#include <ardour/types.h>
#include <ardour/plugin.h>

namespace ARDOUR {

class Plugin;
class Session;
class AudioEngine;

class PluginManager {
  public:
	PluginManager ();
	~PluginManager ();

	ARDOUR::PluginInfoList &vst_plugin_info () { return _vst_plugin_info; }
	ARDOUR::PluginInfoList &ladspa_plugin_info () { return _ladspa_plugin_info; }

	void refresh ();

	int add_ladspa_directory (std::string dirpath);
	int add_vst_directory (std::string dirpath);

	static PluginManager* the_manager() { return _manager; }

  private:
	ARDOUR::PluginInfoList _vst_plugin_info;
	ARDOUR::PluginInfoList _ladspa_plugin_info;
	std::map<uint32_t, std::string> rdf_type;

	std::string ladspa_path;
	std::string vst_path;

	void ladspa_refresh ();
	void vst_refresh ();

	void add_lrdf_data (const std::string &path);
	void add_ladspa_presets ();
	void add_vst_presets ();
	void add_presets (std::string domain);

	int vst_discover_from_path (std::string path);
	int vst_discover (std::string path);

	int ladspa_discover_from_path (std::string path);
	int ladspa_discover (std::string path);

	std::string get_ladspa_category (uint32_t id);

	static PluginManager* _manager; // singleton
};

} /* namespace ARDOUR */

#endif /* __ardour_plugin_manager_h__ */

