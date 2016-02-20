/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Copyright (C) Flamewing 2011-2015 <flamewing.sonic@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>

#include "sseditor.h"

#include <fstream>

/* For testing propose use the local (not installed) ui file */
//#define DEBUG 1
#ifdef WIN32
#   define UI_FILE "./s2ssedit.ui"
#else
#   ifdef DEBUG
#       define UI_FILE "src/s2ssedit.ui"
#   else
#       define UI_FILE PACKAGE_DATA_DIR"/s2ssedit/ui/s2ssedit.ui"
#   endif
#endif

using namespace std;

int main(int argc, char *argv[]) {
	try {
		sseditor *Editor = sseditor::create_instance(argc, argv, UI_FILE);
		Editor->run();
	} catch (const Glib::FileError &ex) {
		cerr << ex.what() << endl;
		return 1;
	}
	return 0;
}
