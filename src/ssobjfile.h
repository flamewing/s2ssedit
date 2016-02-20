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

#ifndef __SSOBJFILE_H
#define __SSOBJFILE_H

#include <algorithm>
#include <istream>
#include <vector>
#include <ostream>
#include <string>

#include "sslevelobjs.h"

#define SS_OBJECT_FILE "Special stage object location lists (Kosinski compression).bin"
#define SS_LAYOUT_FILE "Special stage level layouts (Nemesis compression).bin"

class ssobj_file {
private:
	ssobj_file();
	void read_internal(std::istream &objfile, std::istream &layfile);
	void write_internal(std::ostream &objfile, std::ostream &layfile) const;
protected:
	std::vector<sslevels> stages;
	std::string layoutfile, objectfile;
	bool error;
public:
	ssobj_file(std::string dir);
	size_t size() const;

	void print() const;
	void print(size_t i) const;

	void read();
	void read_backup(int i);
	void read_snapshot(int i);
	void write() const;
	void write_backup() const;
	void write_snapshot(int i) const;

	size_t num_stages() const {
		return stages.size();
	}
	sslevels *get_stage(size_t s) {
		return &(stages[s]);
	}
	sslevels *insert(sslevels const &lvl, size_t s) {
		return &*(stages.insert(stages.begin() + s, lvl));
	}
	sslevels *append(sslevels const &lvl) {
		stages.push_back(lvl);
		return &stages.back();
	}
	sslevels *remove(size_t s) {
		std::vector<sslevels>::iterator it = stages.erase(stages.begin() + s);
		if (it == stages.end()) {
			return &stages.back();
		}
		return &*it;
	}
	sslevels *move_left(size_t s) {
		if (s == 0) {
			return &stages.front();
		}
		std::swap(stages[s - 1], stages[s]);
		return &stages[s - 1];
	}
	sslevels *move_right(size_t s) {
		if (s >= stages.size() - 1) {
			return &stages.back();
		}
		std::swap(stages[s], stages[s + 1]);
		return &stages[s + 1];
	}

	bool good() const {
		return !error;
	}
};

#endif // __SSOBJFILE_H
