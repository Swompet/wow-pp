//
// This file is part of the WoW++ project.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Genral Public License as published by
// the Free Software Foudnation; either version 2 of the Licanse, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// World of Warcraft, and all World of Warcraft or Warcraft art, images,
// and lore are copyrighted by Blizzard Entertainment, Inc.
//

#include <iomanip>
#include <cmath>
#include <cstddef>
#include <ostream>
#include <cassert>
#include <limits>
#include "vector3.h"
#include "binary_io/reader.h"
#include "binary_io/writer.h"

namespace wowpp
{
	namespace math
	{
		std::ostream &operator << (std::ostream &os, const Vector3 &right)
		{
			std::size_t i = 0;
			for (; i < 3; ++i)
			{
				os << std::fixed << right[i] << ' ';
			}
			return os;
		}

		io::Writer &operator << (io::Writer &w, Vector3 const &vector)
		{
			return w
				<< io::write<float>(vector.x)
				<< io::write<float>(vector.y)
				<< io::write<float>(vector.z);
		}
		io::Reader &operator >> (io::Reader &r, Vector3 &vector)
		{
			return r
				>> io::read<float>(vector.x)
				>> io::read<float>(vector.y)
				>> io::read<float>(vector.z);
		}
	}
}