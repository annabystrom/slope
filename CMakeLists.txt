#
# Copyright (C) 2017  Elvis Teixeira
#
# This source code is free software: you can redistribute it
# and/or modify it under the terms of the GNU Lesser General
# Public License as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any
# later version.
#
# This source code is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU Lesser General Public License for
# more details.
#
# You should have received a copy of the GNU Lesser General
# Public License along with this program.
# If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.5)

project(slope C)

set(SLOPE_MAJOR_VERSION 2)
set(SLOPE_MINOR_VERSION 6)
set(SLOPE_PATCH_VERSION 0)
set(SLOPE_VERSION_STRING "${SLOPE_MAJOR_VERSION}.${SLOPE_MINOR_VERSION}.${SLOPE_PATCH_VERSION}")

set(SLOPE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Base directory for Slope" FORCE)
set(SLOPE_AUTOGEN_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "Autogen directory for Slope" FORCE)

subdirs(
   "${SLOPE_BASE_DIR}/slope"
   "${SLOPE_BASE_DIR}/demos"
)
