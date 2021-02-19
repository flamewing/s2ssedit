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

#include "s2ssedit/ssobjfile.hh"

#include "s2ssedit/ignore_unused_variable_warning.hh"

#include <mdcomp/bigendian_io.hh>
#include <mdcomp/kosinski.hh>
#include <mdcomp/nemesis.hh>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

using std::fstream;
using std::ifstream;
using std::ios;
using std::istream;
using std::ofstream;
using std::ostream;
using std::string;
using std::stringstream;
using std::vector;

//#define DEBUG_DECODER 1
//#define DEBUG_ENCODER 1

ssobj_file::ssobj_file(string const& dir) {
    layoutfile = dir + SS_LAYOUT_FILE;
    objectfile = dir + SS_OBJECT_FILE;

    ifstream fobj(objectfile.c_str(), ios::in | ios::binary);
    ifstream flay(layoutfile.c_str(), ios::in | ios::binary);

    error = !(fobj.good() && flay.good());
    if (!error) {
        read();
    }
}

void ssobj_file::read() {
    ifstream fobj(objectfile.c_str(), ios::in | ios::binary);
    ifstream flay(layoutfile.c_str(), ios::in | ios::binary);

    stringstream objfile(ios::in | ios::out | ios::binary);
    stringstream layfile(ios::in | ios::out | ios::binary);

    kosinski::decode(fobj, objfile);
    fobj.close();
    objfile.seekg(0);

    nemesis::decode(flay, layfile);
    flay.close();
    layfile.seekg(0);

    read_internal(objfile, layfile);
}

void ssobj_file::read_internal(istream& objfile, istream& layfile) {
    stages.clear();

    objfile.seekg(0, ios::end);
    size_t sz = objfile.tellg();
    objfile.seekg(0, ios::beg);

    layfile.seekg(0, ios::end);
    size_t sz2 = layfile.tellg();
    layfile.seekg(0, ios::beg);

    vector<int> off;
    vector<int> end;
    vector<int> off2;
    vector<int> end2;
    int         term = BigEndian::Read2(objfile);
    off.push_back(term);
    while (objfile.tellg() < term) {
        int pos = BigEndian::Read2(objfile);
        off.push_back(pos);
        end.push_back(pos);
    }
    end.push_back(sz);

    int term2 = BigEndian::Read2(layfile);
    off2.push_back(term2);
    while (layfile.tellg() < term2) {
        size_t pos = BigEndian::Read2(layfile);
        off2.push_back(pos);
        end2.push_back(pos);
    }
    end2.push_back(sz2);

    for (size_t i = 0; i < off.size(); i++) {
        objfile.seekg(off[i]);
        layfile.seekg(off2[i]);
        sslevels sd;
        sd.read(objfile, layfile, end[i], end2[i]);
        stages.push_back(sd);
    }
}

size_t ssobj_file::size() const {
    size_t sz = 2 * stages.size();
    for (auto const& elem : stages) {
        sz += elem.size();
    }
    return sz;
}

void ssobj_file::write() const {
    stringstream objfile(ios::in | ios::out | ios::binary);
    stringstream layfile(ios::in | ios::out | ios::binary);

    write_internal(objfile, layfile);

    ofstream fobj(objectfile.c_str(), ios::out | ios::binary);
    ofstream flay(layoutfile.c_str(), ios::out | ios::binary);

    objfile.seekg(0);
    kosinski::encode(objfile, fobj);

    layfile.seekg(0);
    nemesis::encode(layfile, flay);
}

void ssobj_file::write_internal(ostream& objfile, ostream& layfile) const {
    size_t sz  = 2 * stages.size();
    size_t off = sz;
    for (auto const& sd : stages) {
        BigEndian::Write2(objfile, sz);
        BigEndian::Write2(layfile, off);
        sz += sd.size();
        off += sd.num_segments();
    }
    for (auto const& sd : stages) {
        sd.write(objfile, layfile);
    }
}
