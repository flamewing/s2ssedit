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

#ifndef OBJECT_H
#define OBJECT_H

#include <s2ssedit/ssobjfile.hh>

class object {
private:
    int32_t segment = -1;
    int32_t pos     = 0;
    int32_t angle   = 0;

    sssegments::ObjectTypes type = sssegments::eRing;

public:
    object(int seg, unsigned x, unsigned y, sssegments::ObjectTypes t)
        : segment(seg), pos(y), angle(x), type(t) {}
    object() noexcept = default;
    sssegments::ObjectTypes get_type() const { return type; }

    int32_t get_segment() const { return segment; }
    int32_t get_pos() const { return pos; }
    int32_t get_angle() const { return angle; }
    bool    valid() const { return segment != -1; }
    bool    operator<(object const& other) const {
        if (segment < other.segment) {
            return true;
        }
        if (segment > other.segment) {
            return false;
        }
        if (pos < other.pos) {
            return true;
        }
        if (pos > other.pos) {
            return false;
        }
        return angle < other.angle;
    }
    bool operator==(object const& other) const {
        return !(*this < other) && !(other < *this);
    }
    bool operator!=(object const& other) const { return !(*this == other); }
    bool operator>(object const& other) const { return other < *this; }
    bool operator<=(object const& other) const { return !(other < *this); }
    bool operator>=(object const& other) const { return !(*this < other); }
    void reset() {
        segment = -1;
        angle   = 0;
        pos     = 0;
        type    = sssegments::eRing;
    }
    void set_segment(int seg) { segment = seg; }
    void set_pos(unsigned y) { pos = y; }
    void set_angle(unsigned x) { angle = x; }
    void set_type(sssegments::ObjectTypes t) { type = t; }
    void set(int seg, unsigned x, unsigned y, sssegments::ObjectTypes t) {
        segment = seg;
        angle   = x;
        pos     = y;
        type    = t;
    }
};

struct ObjectMatchFunctor {
    bool operator()(object const& obj1, object const& obj2) const {
        return obj1 == obj2 && obj1.get_type() == obj2.get_type();
    }
};

#endif // OBJECT_H
