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

#ifndef __SSSEGMENTOBJS_H
#define __SSSEGMENTOBJS_H

#include <map>
#include <istream>
#include <ostream>

#ifdef UNUSED
#elif defined(__GNUC__)
#	define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
#	define UNUSED(x) /*@unused@*/ x
#elif defined(__cplusplus)
#	define UNUSED(x)
#else
#	define UNUSED(x) x
#endif

class sssegments {
public:
	enum SegmentTypes {
		eNormalSegment = 0xff,
		eRingsMessage = 0xfc,
		eCheckpoint = 0xfe,
		eChaosEmerald = 0xfd
	};
	enum SegmentGeometry {
		eTurnThenRise = 0,
		eTurnThenDrop,
		eTurnThenStraight,
		eStraight,
		eStraightThenTurn
	};
	enum ObjectTypes {
		eRing = 0x00,
		eBomb = 0x40
	};
	//               pos
	typedef std::map<unsigned char, std::map<unsigned char, ObjectTypes> > segobjs;
protected:
	segobjs objects;
	bool flip;
	SegmentTypes terminator;
	SegmentGeometry geometry;
	unsigned short numrings, numbombs, numshadows;
public:
	sssegments()
		: flip(false), terminator(eNormalSegment), geometry(eStraight),
		  numrings(0), numbombs(0), numshadows(0) {
	}
	sssegments(sssegments const &other) {
		copy(other);
	}
	sssegments &operator=(sssegments const &other) {
		if (this != &other) {
			copy(other);
		}
		return *this;
	}
	void copy(sssegments const &other) {
		objects = other.objects;
		flip = other.flip;
		terminator = other.terminator;
		geometry = other.geometry;
		numrings = other.numrings;
		numbombs = other.numbombs;
		numshadows = other.numshadows;
	}
	size_t size() const;

	void print() const;

	unsigned short get_numrings() const {
		return numrings;
	}
	unsigned short get_numbombs() const {
		return numbombs;
	}
	unsigned short get_numshadows() const {
		return numshadows;
	}
	unsigned short get_totalobjs() const {
		return numrings + numbombs + numshadows;
	}
	SegmentTypes get_type() const {
		return terminator;
	}
	SegmentGeometry get_geometry() const {
		return geometry;
	}
	static unsigned int get_length(SegmentGeometry UNUSED(geometry)) {
		// Not yet:
		/*
		switch (geometry)
		{
		    case eTurnThenRise:
		    case eTurnThenDrop:
		        return 24;
		    case eTurnThenStraight:
		        return 12;
		    case eStraight:
		        return 16;
		    case eStraightThenTurn:
		        return 11;
		}
		//*/
		return 24;
	}
	unsigned int get_length() const {
		return get_length(geometry);
	}
	bool get_direction() const {
		return flip;
	}
	void set_type(SegmentTypes t) {
		terminator = t;
	}
	void set_geometry(SegmentGeometry g) {
		geometry = g;
	}
	void set_direction(bool tf) {
		flip = tf;
	}
	segobjs::mapped_type const &get_row(unsigned char row) {
		return objects[row];
	}
	bool exists(unsigned char row, unsigned char angle, ObjectTypes &type) const {
		auto it = objects.find(row);
		if (it == objects.end()) {
			return false;
		}
		segobjs::mapped_type const &t = it->second;
		auto it2 = t.find(angle);
		if (it2 == t.end()) {
			return false;
		}
		type = it2->second;
		return true;
	}
	void update(unsigned char row, unsigned char angle, ObjectTypes type, bool insert) {
		auto it = objects.find(row);
		if (it == objects.end()) {
			if (!insert) {
				return;
			}
			segobjs::mapped_type t;
			t[angle] = type;
			objects[row] = t;
		} else {
			segobjs::mapped_type &t = it->second;
			auto it2 = t.find(angle);
			if (it2 == t.end()) {
				if (!insert) {
					return;
				}
				t[angle] = type;
			} else {
				if (it2->second == type) {
					return;
				}
				if (it2->second == eRing) {
					numrings--;
					numbombs++;
				} else {
					numbombs--;
					numrings++;
				}
				it2->second = type;
				return;
			}
		}
		if ((angle & 0x80) == 0) {
			numshadows++;
		}
		if (type == eRing) {
			numrings++;
		} else {
			numbombs++;
		}
	}
	void remove(unsigned char row, unsigned char angle) {
		auto it = objects.find(row);
		if (it == objects.end()) {
			return;
		}
		segobjs::mapped_type &t = it->second;
		auto it2 = t.find(angle);
		if (it2 == t.end()) {
			return;
		}
		if ((angle & 0x80) == 0) {
			numshadows--;
		}
		if (it2->second == eRing) {
			numrings--;
		} else {
			numbombs--;
		}
		t.erase(it2);
	}

	void read(std::istream &in, std::istream &lay);
	void write(std::ostream &out, std::ostream &lay) const;
};

#endif // __SSSEGMENTOBJS_H
