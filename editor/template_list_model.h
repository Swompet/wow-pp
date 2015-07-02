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

#pragma once

#include <QAbstractListModel>
#include <QObject>

namespace wowpp
{
	namespace editor
	{
		template<class T>
		class TemplateListModel : public QAbstractListModel
		{
		public:

			explicit TemplateListModel(const T &entries, QObject *parent = nullptr)
				: QAbstractListModel(parent)
				, m_entries(entries)
			{
			}

			int rowCount(const QModelIndex &parent = QModelIndex()) const
			{
				return m_entries.getTemplates().size();
			}

			QVariant data(const QModelIndex &index, int role) const
			{
				if (!index.isValid())
					return QVariant();

				if (index.row() >= m_entries.getTemplates().size())
					return QVariant();

				if (role == Qt::DisplayRole)
				{
					const auto &templates = m_entries.getTemplates();
					return QString(templates[index.row()]->name.c_str());
				}

				return QVariant();
			}

			QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
			{
				if (role != Qt::DisplayRole)
					return QVariant();

				if (orientation == Qt::Horizontal)
				{
					return QString("Column %1").arg(section);
				}
				else
				{
					return QString("Row %1").arg(section);
				}
			}

			Qt::ItemFlags flags(const QModelIndex &index) const
			{
				if (!index.isValid())
					return Qt::ItemIsEnabled;

				return QAbstractItemModel::flags(index) & ~Qt::ItemIsEditable;
			}

			bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
			{
				// Not supported
				return false;
			}

			bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex())
			{
				// Not supported
				return false;
			}

			bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex())
			{
				// Not supported
				return false;
			}

		private:

			const T &m_entries;
		};
	}
}