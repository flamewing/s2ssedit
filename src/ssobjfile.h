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

#ifndef _SSOBJFILE_H_
#define _SSOBJFILE_H_

#include <vector>
#include <istream>
#include <ostream>

#include "sslevelobjs.h"

class ssobj_file
{
protected:
	std::vector<sslevels> stages;
public:
	void read(std::istream& in, std::istream& lay);
	size_t size() const;
	void print() const;
	void print(size_t i) const;
	void write(std::ostream& out, std::ostream& lay) const;
};

#endif // _SSOBJFILE_H_
