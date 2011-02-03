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

#include <iostream>
#include <cstring>

#include "ssobjfile.h"
#include "bigendian_io.h"

void ssobj_file::read(std::istream& in, std::istream& lay)
{
	in.seekg(0,std::ios::end);
	size_t sz = in.tellg();
	in.seekg(0,std::ios::beg);

	lay.seekg(0,std::ios::end);
	size_t sz2 = in.tellg();
	lay.seekg(0,std::ios::beg);

	std::vector<size_t> off,end,	off2,end2;
	size_t const term = BigEndian::Read2(in);
	off.push_back(term);
	while (in.tellg() < term)
	{
		size_t pos = BigEndian::Read2(in);
		off.push_back(pos);
		end.push_back(pos);
	}
	end.push_back(sz);

	size_t const term2 = BigEndian::Read2(lay);
	off2.push_back(term2);
	while (lay.tellg() < term2)
	{
		size_t pos = BigEndian::Read2(lay);
		off2.push_back(pos);
		end2.push_back(pos);
	}
	end2.push_back(sz2);

	for (size_t i = 0; i < off.size(); i++)
	{
		in.seekg(off[i]);
		lay.seekg(off2[i]);
		sslevels sd;
		sd.read(in, lay, end[i], end2[i]);
		stages.push_back(sd);
	}
}

size_t ssobj_file::size() const
{
	size_t sz = 2 * stages.size();
	for (std::vector<sslevels>::const_iterator it = stages.begin();
	     it != stages.end(); ++it)
	{
		sslevels const& sd = *it;
		sz += sd.size();
	}
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

void ssobj_file::write(std::ostream& out, std::ostream& lay) const
{
	size_t sz = 2 * stages.size(), off = sz;
	for (std::vector<sslevels>::const_iterator it = stages.begin();
	     it != stages.end(); ++it)
		{
		sslevels const& sd = *it;
		BigEndian::Write2(out, sz);
		BigEndian::Write2(lay, off);
		sz += sd.size();
		off += sd.count();
		}
	for (std::vector<sslevels>::const_iterator it = stages.begin();
	     it != stages.end(); ++it)
		{
		sslevels const& sd = *it;
		sd.write(out, lay);
		}
}
