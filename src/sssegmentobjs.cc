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

#include <mdcomp/bigendian_io.hh>
#include <s2ssedit/sssegmentobjs.hh>

#include <cstring>
#include <iostream>

using std::istream;
using std::ostream;

void sssegments::read(istream& in, istream& lay) {
    uint8_t geom = Read1(lay);
    flip         = (geom & eFlipMask) != 0;
    geometry     = static_cast<SegmentGeometry>(geom & eGeomMask);

    while (in.good()) {
        uint8_t type = Read1(in);
        uint8_t pos;
        uint8_t angle;
        switch (type) {
        case eChaosEmerald:     // Emerald
        case eCheckpoint:       // Checkpoint
        case eRingsMessage:     // Message
        case eNormalSegment:    // End of segment
            terminator = static_cast<SegmentTypes>(type);
            return;
        default:    // Ring, bomb
            pos = (type & ePositionMask);
            // Not yet:
            // pos = (type & ePositionMask) + get_length();
            type &= eItemMask;
            angle = Read1(in);
            break;
        }
        auto& posobjs = objects[pos];
        posobjs.emplace(angle, ObjectTypes(type));
        add_obj(angle, ObjectTypes(type));
    }
}

size_t sssegments::size() const {
    size_t sz = 0;    // Terminator
    for (auto const& elem : objects) {
        sz += elem.second.size();
    }
    return 2 * sz + 1;
}

void sssegments::write(ostream& out, ostream& lay) const {
    for (auto const& elem : objects) {
        auto const& posobjs = elem.second;
        uint8_t     pos     = elem.first;
        for (auto const& posobj : posobjs) {
            Write1(out, (posobj.second) | pos);
            Write1(out, posobj.first);
        }
    }
    Write1(out, terminator);
    Write1(lay, get_flip_geom());
}
