# ===============================================================================
#
# PROGRAMMERS:
#
# info@r-lidar.com  -  https://github.com/r-lidar/rlas
#
# COPYRIGHT:
#
# Copyright 2017-2026 Jean-Romain Roussel
#
# This file is part of rlas R package.
#
# rlas is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>
#
# ===============================================================================

# nocov start
.onUnload <- function (libpath) {
  library.dynam.unload("rlas", libpath)
}
# nocov end