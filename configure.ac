# Copyright (C) 2010 Giovanni [dacav] Simoni
#                    <simgidacav@gmail.com>
# 
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation; either 
# version 3 of the License, or (at your option) any later 
# version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# 

AC_PREREQ([2.65])
AC_INIT([nxmodtools], [0.1], [simgidacav@gmail.com])
AC_CONFIG_SRCDIR([src/splitter.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

AC_CHECK_HEADER([dacav.h],[],
                [AC_MSG_ERROR(libdacav not found.  Please install libdacav)])
AC_CHECK_HEADER([elfsword.h],[],
                [AC_MSG_ERROR(libelfsword not found.  Please install libelfsword)])
AC_CHECK_HEADER([libusb-1.0/libusb.h],[],
                [AC_MSG_ERROR(libusb not found.  Please install libusb 1.x)])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_UINT32_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
