/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * S2-SSEdit
 * Copyright (C) Flamewing 2011 <flamewing.sonic@gmail.com>
 * 
 * S2-SSEdit is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * S2-SSEdit is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtkmm.h>
#include <iostream>

#include "sseditor.h"

#include <fstream>

/* For testing propose use the local (not installed) ui file */
#define DEBUG 1
#ifdef DEBUG
#define UI_FILE "src/s2ssedit.ui"
#else
#define UI_FILE PACKAGE_DATA_DIR"/s2ssedit/ui/s2ssedit.ui"
#endif

int main(int argc, char *argv[])
{
	sseditor *Editor;
	try
	{
		Editor = sseditor::create_instance(argc, argv, UI_FILE);
	}
	catch (const Glib::FileError & ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	Editor->run();
	return 0;
}
