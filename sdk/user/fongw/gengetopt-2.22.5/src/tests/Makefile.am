# Copyright (C) 1999-2010  Free Software Foundation, Inc.
#  
# This file is part of GNU gengetopt 
#
# GNU gengetopt is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License as published by 
# the Free Software Foundation; either version 3, or (at your option) 
# any later version. 
#
# GNU gengetopt is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details. 
#
# You should have received a copy of the GNU General Public License along 
# with gengetopt; see the file COPYING. If not, write to the Free Software 
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 

if NO_CROSSCOMPILING

# these are for gnulib
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/gl -I$(top_builddir)/gl

LDADD = $(top_builddir)/src/libgengetopt.la

VALGRINDTESTS = test_specialchars

TESTS = $(VALGRINDTESTS)

check_PROGRAMS = $(VALGRINDTESTS)

test_specialchars_SOURCES = test_specialchars.cc

noinst_HEADERS = asserttestexit.h

endif