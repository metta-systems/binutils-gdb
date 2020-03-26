/* DWARF macro support for GDB.

   Copyright (C) 2003-2020 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef GDB_DWARF2_MACRO_H
#define GDB_DWARF2_MACRO_H

struct buildsym_compunit;

extern void dwarf_decode_macros (struct dwarf2_per_objfile *dwarf2_per_objfile,
				 buildsym_compunit *builder,
				 dwarf2_section_info *section,
				 struct line_header *lh,
				 unsigned int offset_size,
				 unsigned int offset,
				 int section_is_gnu);

#endif /* GDB_DWARF2_MACRO_H */