/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Copyright (C) Flamewing 2011-2013 <flamewing.sonic@gmail.com>
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

#ifndef _SSLEVELOBJS_H_
#define _SSLEVELOBJS_H_

#include <vector>
#include <istream>
#include <ostream>

#include "sssegmentobjs.h"

class sslevels {
protected:
	std::vector<sssegments> segments;
public:
	sslevels() {        }
	sslevels(sslevels const &other) {
		copy(other);
	}
	sslevels &operator=(sslevels const &other) {
		if (this != &other)
			copy(other);
		return *this;
	}
	void copy(sslevels const &other) {
		segments = other.segments;
	}
	size_t size() const;

	void print() const;

	void read(std::istream &in, std::istream &lay, int term, int term2);
	void write(std::ostream &out, std::ostream &lay) const;

	size_t fill_position_array(std::vector<size_t> &segpos) const {
		segpos.clear();
		segpos.reserve(segments.size());
		size_t tally = 0;
		for (std::vector<sssegments>::const_iterator it = segments.begin();
		        it != segments.end(); ++it) {
			segpos.push_back(tally);
			tally += it->get_length();
		}
		// Total size.
		return tally;
	}

	size_t num_segments() const {
		return segments.size();
	}
	sssegments *get_segment(size_t s) {
		return &(segments[s]);
	}
	sssegments *insert(sssegments const &lvl, size_t s) {
		return &*(segments.insert(segments.begin() + s, lvl));
	}
	sssegments *append(sssegments const &lvl) {
		segments.push_back(lvl);
		return &segments.back();
	}
	sssegments *remove(size_t s) {
		std::vector<sssegments>::iterator it = segments.erase(segments.begin() + s);
		if (it == segments.end())
			return &segments.back();
		return &*it;
	}
	sssegments *move_left(size_t s) {
		if (s == 0)
			return &segments.front();
		std::swap(segments[s - 1], segments[s]);
		return &segments[s - 1];
	}
	sssegments *move_right(size_t s) {
		if (s >= segments.size() - 1)
			return &segments.back();
		std::swap(segments[s], segments[s + 1]);
		return &segments[s + 1];
	}
};

#endif // _SSLEVELOBJS_H_
