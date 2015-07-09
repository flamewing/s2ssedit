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

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "ssobjfile.h"

class object {
private:
	int segment, pos, angle;
	sssegments::ObjectTypes type;
public:
	object(int seg, int x, int y, sssegments::ObjectTypes t)
		: segment(seg), pos(y), angle(x), type(t)
	{  }
	object()
		: segment(-1), pos(0), angle(0), type(sssegments::eRing)
	{  }
	sssegments::ObjectTypes get_type() const {
		return type;
	}
	int get_segment() const {
		return segment;
	}
	int get_pos() const {
		return pos;
	}
	int get_angle() const {
		return angle;
	}
	bool valid() const {
		return segment != -1;
	}
	bool operator<(object const &other) const {
		if (segment < other.segment)
			return true;
		else if (segment > other.segment)
			return false;
		if (pos < other.pos)
			return true;
		else if (pos > other.pos)
			return false;
		return angle < other.angle;
	}
	bool operator==(object const &other) const {
		return !(*this < other) && !(other < *this);
	}
	bool operator!=(object const &other) const {
		return !(*this == other);
	}
	bool operator>(object const &other) const {
		return other < *this;
	}
	bool operator<=(object const &other) const {
		return !(other < *this);
	}
	bool operator>=(object const &other) const {
		return !(*this < other);
	}
	void reset() {
		segment = -1;
		angle = pos = 0;
		type = sssegments::eRing;
	}
	void set_segment(int seg) {
		segment = seg;
	}
	void set_pos(int y) {
		pos = y;
	}
	void set_angle(int x) {
		angle = x;
	}
	void set_type(sssegments::ObjectTypes t) {
		type = t;
	}
	void set(int seg, int x, int y, sssegments::ObjectTypes t = sssegments::eRing) {
		segment = seg;
		angle = x;
		pos = y;
		type = t;
	}
};

struct ObjectMatchFunctor {
	bool operator()(object const &obj1, object const &obj2) const {
		return obj1 == obj2 && obj1.get_type() == obj2.get_type();
	}
};

#endif // _OBJECT_H_
