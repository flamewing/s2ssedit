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

#include "s2ssedit/sslevelobjs.hh"

using std::istream;
using std::ostream;

void sslevels::read(istream& in, istream& lay, int term, int term2) {
    while (in.tellg() < term && lay.tellg() < term2) {
        sssegments nn;
        nn.read(in, lay);
        segments.push_back(nn);
    }
}

size_t sslevels::size() const {
    size_t sz = 0;
    for (auto const& sd : segments) {
        sz += sd.size();
    }
    return sz;
}

void sslevels::write(ostream& out, ostream& lay) const {
    for (auto const& sd : segments) {
        sd.write(out, lay);
    }
}
