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

#ifndef _SSSEGMENTOBJS_H_
#define _SSSEGMENTOBJS_H_

#include <map>
#include <istream>
#include <ostream>

class sssegments
{
public:
	enum SegmentTypes
	{
		eNormalSegment = 0xff,
		eRingsMessage = 0xfc,
		eCheckpoint = 0xfe,
		eChaosEmerald = 0xfd
	};
	enum SegmentGeometry
	{
		eTurnThenRise = 0,
		eTurnThenDrop,
		eTurnThenStraight,
		eStraight,
		eStraightThenTurn
	};
protected:
	//               pos             angle  type
	typedef std::map<size_t,std::map<size_t,size_t> > segobjs;
	segobjs objects;
	bool flip;
	SegmentTypes terminator;
	SegmentGeometry geometry;
public:
	sssegments() : flip(false), terminator(eNormalSegment), geometry(eStraight) {		}
	size_t size() const;

	void print() const;

	SegmentTypes get_type() const
	{	return terminator;	}
	SegmentGeometry get_geometry() const
	{	return geometry;	}
	bool get_direction() const
	{	return flip;	}
	void set_type(SegmentTypes t)
	{	terminator = t;	}
	void set_geometry(SegmentGeometry g)
	{	geometry = g;	}
	void set_direction(bool tf)
	{	flip = tf;	}
	std::map<size_t,size_t> const& get_row(size_t row)
	{	return objects[row];	}
	bool exists(size_t row, size_t angle, size_t& type) const
	{
		segobjs::const_iterator it = objects.find(row);
		if (it == objects.end())
			return false;

		std::map<size_t,size_t> const& t = it->second;
		std::map<size_t,size_t>::const_iterator it2 = t.find(angle);
		if (it2 == t.end())
			return false;

		type = it2->second;
		return true;
	}
	void update(size_t row, size_t angle, size_t type, bool insert)
	{
		segobjs::iterator it = objects.find(row);
		if (it == objects.end())
		{
			if (!insert)
				return;
			std::map<size_t,size_t> t;
			t[angle] = type;
			objects[row] = t;
		}
		else
		{
			std::map<size_t,size_t>& t = it->second;
			std::map<size_t,size_t>::iterator it2 = t.find(angle);
			if (it2 == t.end())
			{
				if (!insert)
					return;
				t[angle] = type;
			}
			else
				it2->second = size_t(type);
		}
	}
	void remove(size_t row, size_t angle)
	{
		segobjs::iterator it = objects.find(row);
		if (it == objects.end())
			return;

		std::map<size_t,size_t>& t = it->second;
		std::map<size_t,size_t>::iterator it2 = t.find(angle);
		if (it2 == t.end())
			return;

		t.erase(it2);
	}

	void read(std::istream& in, std::istream& lay);
	void write(std::ostream& out, std::ostream& lay) const;
};

#endif // _SSSEGMENTOBJS_H_
