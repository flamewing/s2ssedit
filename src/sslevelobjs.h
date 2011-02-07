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

#ifndef _SSLEVELOBJS_H_
#define _SSLEVELOBJS_H_

#include <vector>
#include <istream>
#include <ostream>

#include "sssegmentobjs.h"

class sslevels
{
protected:
	std::vector<sssegments> segments;
public:
	size_t size() const;

	void print() const;

	void read(std::istream& in, std::istream& lay, size_t const term,
	          size_t const term2);
	void write(std::ostream& out, std::ostream& lay) const;


	size_t num_segments() const
	{	return segments.size();	}
	sssegments *get_segment(size_t s)
	{	return &(segments[s]);	}
	sssegments *insert(sssegments const& lvl, size_t s)
	{	return &*(segments.insert(segments.begin() + s, lvl));	}
	sssegments *append(sssegments const& lvl)
	{
		segments.push_back(lvl);
		return &segments.back();
	}
	sssegments *remove(size_t s)
	{
		std::vector<sssegments>::iterator it = segments.erase(segments.begin() + s);
		if (it == segments.end())
			return &segments.back();
		return &*it;
	}
	sssegments *move_left(size_t s)
	{
		if (s == 0)
			return &segments.front();
		std::swap(segments[s - 1], segments[s]);
		return &segments[s - 1];
	}
	sssegments *move_right(size_t s)
	{
		if (s >= segments.size() - 1)
			return &segments.back();
		std::swap(segments[s], segments[s + 1]);
		return &segments[s + 1];
	}
};

#endif // _SSLEVELOBJS_H_
