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

#ifndef _SSSEGMENTOBJS_H_
#define _SSSEGMENTOBJS_H_

#include <map>
#include <istream>
#include <ostream>

class sssegments
{
protected:
	//  pos        angle  type
	std::map<size_t,std::map<size_t,size_t> > objects;
	bool flip;
	unsigned char terminator, geometry;
public:
	void read(std::istream& in, std::istream& lay);
	size_t size() const;
	void print() const;
	void write(std::ostream& out, std::ostream& lay) const;
};

#endif // _SSSEGMENTOBJS_H_
