//
// This file is part of the WoW++ project.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#pragma once

#include "tiled_unit_finder.h"
#include "common/linear_set.h"

namespace wowpp
{
	class TiledUnitFinder::Tile final
	{
	public:

		typedef LinearSet<GameUnit *> UnitSet;
		typedef simple::signal<void (GameUnit &)> MoveSignal;

		//unique_ptr, so that Tile is movable
		std::unique_ptr<MoveSignal> moved;

		Tile();
		Tile(Tile &&other);
		Tile &operator = (Tile &&other);
		void swap(Tile &other);
		const UnitSet &getUnits() const;
		void addUnit(GameUnit &unit);
		void removeUnit(GameUnit &unit);

	private:

		UnitSet m_units;
	};
}
