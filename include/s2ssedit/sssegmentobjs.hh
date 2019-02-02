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

#ifndef SSSEGMENTOBJS_H
#define SSSEGMENTOBJS_H

#include <istream>
#include <map>
#include <ostream>

#include <s2ssedit/ignore_unused_variable_warning.hh>

class sssegments {
public:
    enum SegmentTypes {
        eNormalSegment = 0xff,
        eRingsMessage  = 0xfc,
        eCheckpoint    = 0xfe,
        eChaosEmerald  = 0xfd
    };
    enum SegmentGeometry {
        eTurnThenRise = 0,
        eTurnThenDrop,
        eTurnThenStraight,
        eStraight,
        eStraightThenTurn
    };
    enum ObjectTypes : uint8_t {
        ePositionMask = 0x3fU,
        eRing         = 0x00U,
        eBomb         = 0x40U,
        eItemMask     = eBomb
    };
    enum AngleMasks : uint8_t { eOnAirMask = 0x80U };
    enum GeometryMasks : uint8_t {
        eNoFlip   = 0U,
        eGeomMask = 0x7fU,
        eFlipMask = 0x80U
    };
    enum SegmentSizes : uint32_t {
        eTurnThenRiseLen     = 24,
        eTurnThenDropLen     = 24,
        eTurnThenStraightLen = 12,
        eStraightLen         = 16,
        eStraightThenTurnLen = 11
    };
    //                       position          angle
    using segobjs = std::map<uint8_t, std::map<uint8_t, ObjectTypes>>;

    static bool is_aerial(uint8_t angle) noexcept {
        return (angle & eOnAirMask) != 0;
    }
    void add_obj(uint8_t angle, ObjectTypes type) noexcept {
        if (!is_aerial(angle)) {
            numshadows++;
        }
        if (type == eRing) {
            numrings++;
        } else {
            numbombs++;
        }
    }
    void del_obj(uint8_t angle, ObjectTypes type) noexcept {
        if (!is_aerial(angle)) {
            numshadows--;
        }
        if (type == eRing) {
            numrings--;
        } else {
            numbombs--;
        }
    }

private:
    segobjs         objects;
    bool            flip       = false;
    SegmentTypes    terminator = eNormalSegment;
    SegmentGeometry geometry   = eStraight;
    uint16_t        numrings   = 0;
    uint16_t        numbombs   = 0;
    uint16_t        numshadows = 0;

public:
    size_t size() const;

    uint16_t get_numrings() const noexcept { return numrings; }
    uint16_t get_numbombs() const noexcept { return numbombs; }
    uint16_t get_numshadows() const noexcept { return numshadows; }
    uint16_t get_totalobjs() const noexcept {
        return numrings + numbombs + numshadows;
    }
    SegmentTypes    get_type() const noexcept { return terminator; }
    SegmentGeometry get_geometry() const noexcept { return geometry; }
    static uint32_t get_length(SegmentGeometry geometry) noexcept {
        ignore_unused_variable_warning(geometry);
        // Not yet:
        /*
        switch (geometry)
        {
            case eTurnThenRise:
                return eTurnThenRiseLen;
            case eTurnThenDrop:
                return eTurnThenDropLen;
            case eTurnThenStraight:
                return eTurnThenStraightLen;
            case eStraight:
                return eStraightLen;
            case eStraightThenTurn:
                return eStraightThenTurnLen;
        }
        //*/
        return eTurnThenRiseLen;
    }
    int32_t get_length() const noexcept { return get_length(geometry); }
    bool    get_direction() const noexcept { return flip; }
    uint8_t get_flip_geom() const noexcept {
        return static_cast<uint8_t>(flip ? eFlipMask : eNoFlip) |
               static_cast<uint8_t>(geometry);
    }
    void set_type(SegmentTypes t) noexcept { terminator = t; }
    void set_geometry(SegmentGeometry g) noexcept { geometry = g; }
    void set_direction(bool tf) noexcept { flip = tf; }

    auto const& get_row(uint8_t row) noexcept { return objects[row]; }
    bool        exists(uint8_t row, uint8_t angle) const noexcept {
        ObjectTypes type;
        return exists(row, angle, type);
    }
    bool exists(uint8_t row, uint8_t angle, ObjectTypes& type) const noexcept {
        auto it = objects.find(row);
        if (it == objects.end()) {
            return false;
        }
        auto const& t   = it->second;
        auto        it2 = t.find(angle);
        if (it2 == t.end()) {
            return false;
        }
        type = it2->second;
        return true;
    }
    void update(uint8_t row, uint8_t angle, ObjectTypes type, bool insert) {
        auto it = objects.find(row);
        if (it == objects.end()) {
            if (!insert) {
                return;
            }
            segobjs::mapped_type t;
            t[angle]     = type;
            objects[row] = std::move(t);
        } else {
            auto& t   = it->second;
            auto  it2 = t.find(angle);
            if (it2 == t.end()) {
                if (!insert) {
                    return;
                }
                t[angle] = type;
            } else if (it2->second == type) {
                return;
            } else {
                del_obj(angle, it2->second);
                it2->second = type;
            }
        }
        add_obj(angle, type);
    }
    void remove(uint8_t row, uint8_t angle) noexcept {
        auto it = objects.find(row);
        if (it == objects.end()) {
            return;
        }
        auto& t   = it->second;
        auto  it2 = t.find(angle);
        if (it2 == t.end()) {
            return;
        }
        del_obj(angle, it2->second);
        t.erase(it2);
    }

    void read(std::istream& in, std::istream& lay);
    void write(std::ostream& out, std::ostream& lay) const;
};

#endif // SSSEGMENTOBJS_H
