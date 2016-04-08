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
#include <cstring>
#include "bigendian_io.h"

#include "sssegmentobjs.h"

using namespace std;

void sssegments::read(istream &in, istream &lay) {
	unsigned char geom = Read1(lay);
	flip = (geom & 0x80) != 0;
	geometry = static_cast<SegmentGeometry>(geom & 0x7f);

	while (in.good()) {
		unsigned char type = Read1(in), pos, angle;
		switch (type) {
			case eChaosEmerald:         // Emerald
			case eCheckpoint:           // Checkpoint
			case eRingsMessage:         // Message
			case eNormalSegment:        // End of segment
				terminator = static_cast<SegmentTypes>(type);
				return;
			default:            // Ring, bomb
				pos = (type & 0x3f);
				// Not yet:
				//pos = (type & 0x3f) + get_length();
				type &= 0x40;
				angle = Read1(in);
				break;
		}
		segobjs::mapped_type &posobjs = objects[pos];
		posobjs.emplace(angle, ObjectTypes(type));
		if ((angle & 0x80) == 0) {
			numshadows++;
		}
		if (ObjectTypes(type) == eRing) {
			numrings++;
		} else {
			numbombs++;
		}
	}
}

size_t sssegments::size() const {
	size_t sz = 1;  // Terminator
	for (const auto & elem : objects) {
		segobjs::mapped_type const &posobjs = elem.second;
		sz += (2 * posobjs.size());
	}
	return sz;
}

void sssegments::print() const {
	char buf[0x103];
	buf[0] = buf[0x101] = '|';
	buf[0x102] = 0;
	memset(buf + 1, '-', sizeof(buf) - 3);
	switch (terminator) {
		case 0xfc: {        // Message
			char const msg[] = "====== Message  ======";
			size_t const len = strlen(msg);
			memcpy(buf + (sizeof(buf) - len - 1) / 2, msg, len);
			break;
		}
		case 0xfd: {        // Emerald
			char const msg[] = "====== Emerald  ======";
			size_t const len = strlen(msg);
			memcpy(buf + (sizeof(buf) - len - 1) / 2, msg, len);
			break;
		}
		case 0xfe: {        // Checkpoint
			char const msg[] = "====== Checkpoint ======";
			size_t const len = strlen(msg);
			memcpy(buf + (sizeof(buf) - len - 1) / 2, msg, len);
			break;
		}
		default:            // Ring, bomb, normal segment terminator
			break;
	}

	cout << buf << endl;
	for (size_t i = 0; i < 0x40; i++) {
		memset(buf + 1, ' ', sizeof(buf) - 3);
		auto it0 = objects.find(i);
		if (it0 != objects.end()) {
			segobjs::mapped_type const &posobjs = it0->second;
			for (const auto & posobj : posobjs) {
				size_t angle = ((posobj.first + 0x40) & 0xff);
				switch ((posobj.second)) {
					case eBomb:         // Bomb
						buf[1 + angle] = '*';
						break;
					default:            // Ring
						buf[1 + angle] = 'o';
						break;
				}
			}
		}
		cout << buf << endl;
	}
}

void sssegments::write(ostream &out, ostream &lay) const {
	for (const auto & elem : objects) {
		segobjs::mapped_type const &posobjs = elem.second;
		unsigned char pos = elem.first;
		for (const auto & posobj : posobjs) {
			Write1(out, ((posobj.second) | pos));
			Write1(out, (posobj.first));
		}
	}
	Write1(out, terminator);

	Write1(lay, (flip ? 0x80 : 0) | geometry);
}
