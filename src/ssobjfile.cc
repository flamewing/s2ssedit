/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
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

#include <iostream>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <fstream>

#include <mdcomp/kosinski.h>
#include <mdcomp/nemesis.h>
#include "ssobjfile.h"
#include "bigendian_io.h"

//#define DEBUG_DECODER 1
//#define DEBUG_ENCODER 1

ssobj_file::ssobj_file(std::string dir)
{
	layoutfile = dir + SS_LAYOUT_FILE;
	objectfile = dir + SS_OBJECT_FILE;

	std::ifstream fobj(objectfile.c_str(), std::ios::in|std::ios::binary),
	              flay(layoutfile.c_str(), std::ios::in|std::ios::binary);

	if (!fobj.good() || !flay.good())
	{
		error = true;
		return;
	}

	read();
}

void ssobj_file::read()
{
	std::ifstream fobj(objectfile.c_str(), std::ios::in|std::ios::binary),
	              flay(layoutfile.c_str(), std::ios::in|std::ios::binary);

	std::stringstream objfile(std::ios::in|std::ios::out|std::ios::binary),
	                  layfile(std::ios::in|std::ios::out|std::ios::binary);

	kosinski::decode(fobj, objfile);
	fobj.close();
	objfile.seekg(0);

	nemesis::decode(flay, layfile);
	flay.close();
	layfile.seekg(0);

	read_internal(objfile, layfile);
}

void ssobj_file::read_backup(int i)
{
	std::ifstream fobj((objectfile + "~").c_str(), std::ios::in|std::ios::binary),
	              flay((layoutfile + "~").c_str(), std::ios::in|std::ios::binary);

	if (fobj.good() && flay.good())
		read_internal(fobj, flay);
}

void ssobj_file::read_snapshot(int i)
{
	char buf[8];
	sprintf(buf, "%02x", i & 0xff);
	std::string num = buf;
	std::ifstream fobj((objectfile + num).c_str(), std::ios::in|std::ios::binary),
	              flay((layoutfile + num).c_str(), std::ios::in|std::ios::binary);

	if (fobj.good() && flay.good())
		read_internal(fobj, flay);
}

void ssobj_file::read_internal(std::istream& objfile, std::istream& layfile)
{
	stages.clear();
	
	objfile.seekg(0, std::ios::end);
	size_t sz = objfile.tellg();
	objfile.seekg(0, std::ios::beg);

	layfile.seekg(0, std::ios::end);
	size_t sz2 = layfile.tellg();
	layfile.seekg(0, std::ios::beg);

	std::vector<size_t> off,end, off2, end2;
	size_t const term = BigEndian::Read2(objfile);
	off.push_back(term);
	while (objfile.tellg() < term)
	{
		size_t pos = BigEndian::Read2(objfile);
		off.push_back(pos);
		end.push_back(pos);
	}
	end.push_back(sz);

	size_t const term2 = BigEndian::Read2(layfile);
	off2.push_back(term2);
	while (layfile.tellg() < term2)
	{
		size_t pos = BigEndian::Read2(layfile);
		off2.push_back(pos);
		end2.push_back(pos);
	}
	end2.push_back(sz2);

	for (size_t i = 0; i < off.size(); i++)
	{
		objfile.seekg(off[i]);
		layfile.seekg(off2[i]);
		sslevels sd;
		sd.read(objfile, layfile, end[i], end2[i]);
		stages.push_back(sd);
	}
}

size_t ssobj_file::size() const
{
	size_t sz = 2 * stages.size();
	for (std::vector<sslevels>::const_iterator it = stages.begin();
	     it != stages.end(); ++it)
		sz += it->size();
	return sz;
}

void ssobj_file::print() const
{
	char buf[0x103];
	buf[0x102] = 0;
	std::memset(buf,'=',sizeof(buf)-1);
	for (std::vector<sslevels>::const_iterator it = stages.begin();
	     it != stages.end(); ++it)
	{
		std::cout << buf << std::endl;
		sslevels const& sd = *it;
		sd.print();
	}
}

void ssobj_file::print(size_t i) const
{
	if (i < stages.size())
	{
		stages[i].print();
		char buf[0x103];
		buf[0] = buf[0x101] = '|';
		buf[0x102] = 0;
		std::memset(buf + 1,'-',sizeof(buf)-3);
	}
	else
		std::cout << "No such special stage in file: " << i << std::endl;
}

void ssobj_file::write() const
{
	std::stringstream objfile(std::ios::in|std::ios::out|std::ios::binary),
	                  layfile(std::ios::in|std::ios::out|std::ios::binary);

	write_internal(objfile, layfile);

	std::fstream fobj(objectfile.c_str(),
	                     std::ios::in|std::ios::out|std::ios::binary|std::ios::trunc);
	std::ofstream flay(layoutfile.c_str(), std::ios::out|std::ios::binary);
	objfile.seekg(0);
	kosinski::encode(objfile, fobj);

	layfile.seekg(0);
	nemesis::encode(layfile, flay);
}

void ssobj_file::write_backup() const
{
	std::ofstream fobj((objectfile + "~").c_str(), std::ios::out|std::ios::binary),
	              flay((layoutfile + "~").c_str(), std::ios::out|std::ios::binary);

	if (fobj.good() && flay.good())
		write_internal(fobj, flay);
}

void ssobj_file::write_snapshot(int i) const
{
	char buf[8];
	sprintf(buf, "%02x", i & 0xff);
	std::string num = buf;
	std::ofstream fobj((objectfile + num).c_str(), std::ios::out|std::ios::binary),
	              flay((layoutfile + num).c_str(), std::ios::out|std::ios::binary);

	if (fobj.good() && flay.good())
		write_internal(fobj, flay);
}

void ssobj_file::write_internal(std::ostream& objfile, std::ostream& layfile) const
{
	size_t sz = 2 * stages.size(), off = sz;
	for (std::vector<sslevels>::const_iterator it = stages.begin();
	     it != stages.end(); ++it)
	{
		sslevels const& sd = *it;
		BigEndian::Write2(objfile, sz);
		BigEndian::Write2(layfile, off);
		sz += sd.size();
		off += sd.num_segments();
	}
	for (std::vector<sslevels>::const_iterator it = stages.begin();
	     it != stages.end(); ++it)
	{
		sslevels const& sd = *it;
		sd.write(objfile, layfile);
	}
}
