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

#include "common/endian_convert.h"
#include "common/clock.h"
#include "game_protocol.h"
#include "game/game_item.h"
#include "log/default_log_levels.h"
#include "binary_io/stream_source.h"
#include "binary_io/stream_sink.h"
#include "binary_io/reader.h"
#include "binary_io/writer.h"
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include "binary_io/vector_sink.h"

namespace wowpp
{
	namespace game
	{
		namespace server_write
		{
			void triggerCinematic(game::OutgoingPacket &out_packet, UInt32 cinematicId)
			{
				out_packet.start(game::server_packet::TriggerCinematic);
				out_packet
					<< io::write<NetUInt32>(cinematicId);
				out_packet.finish();
			}

			void authChallenge(game::OutgoingPacket &out_packet, UInt32 seed)
			{
				out_packet.start(game::server_packet::AuthChallenge);
				out_packet
					<< io::write<NetUInt32>(seed);
				out_packet.finish();
			}

			void pong(game::OutgoingPacket &out_packet, UInt32 ping)
			{
				out_packet.start(game::server_packet::Pong);
				out_packet
					<< io::write<NetUInt32>(ping);
				out_packet.finish();
			}

			void addonInfo(game::OutgoingPacket &out_packet, const AddonEntries &addons)
			{
				// Vector initialization
				const std::vector<unsigned char> tdata
				{
					0xC3, 0x5B, 0x50, 0x84, 0xB9, 0x3E, 0x32, 0x42, 0x8C, 0xD0, 0xC7, 0x48, 0xFA, 0x0E, 0x5D, 0x54,
					0x5A, 0xA3, 0x0E, 0x14, 0xBA, 0x9E, 0x0D, 0xB9, 0x5D, 0x8B, 0xEE, 0xB6, 0x84, 0x93, 0x45, 0x75,
					0xFF, 0x31, 0xFE, 0x2F, 0x64, 0x3F, 0x3D, 0x6D, 0x07, 0xD9, 0x44, 0x9B, 0x40, 0x85, 0x59, 0x34,
					0x4E, 0x10, 0xE1, 0xE7, 0x43, 0x69, 0xEF, 0x7C, 0x16, 0xFC, 0xB4, 0xED, 0x1B, 0x95, 0x28, 0xA8,
					0x23, 0x76, 0x51, 0x31, 0x57, 0x30, 0x2B, 0x79, 0x08, 0x50, 0x10, 0x1C, 0x4A, 0x1A, 0x2C, 0xC8,
					0x8B, 0x8F, 0x05, 0x2D, 0x22, 0x3D, 0xDB, 0x5A, 0x24, 0x7A, 0x0F, 0x13, 0x50, 0x37, 0x8F, 0x5A,
					0xCC, 0x9E, 0x04, 0x44, 0x0E, 0x87, 0x01, 0xD4, 0xA3, 0x15, 0x94, 0x16, 0x34, 0xC6, 0xC2, 0xC3,
					0xFB, 0x49, 0xFE, 0xE1, 0xF9, 0xDA, 0x8C, 0x50, 0x3C, 0xBE, 0x2C, 0xBB, 0x57, 0xED, 0x46, 0xB9,
					0xAD, 0x8B, 0xC6, 0xDF, 0x0E, 0xD6, 0x0F, 0xBE, 0x80, 0xB3, 0x8B, 0x1E, 0x77, 0xCF, 0xAD, 0x22,
					0xCF, 0xB7, 0x4B, 0xCF, 0xFB, 0xF0, 0x6B, 0x11, 0x45, 0x2D, 0x7A, 0x81, 0x18, 0xF2, 0x92, 0x7E,
					0x98, 0x56, 0x5D, 0x5E, 0x69, 0x72, 0x0A, 0x0D, 0x03, 0x0A, 0x85, 0xA2, 0x85, 0x9C, 0xCB, 0xFB,
					0x56, 0x6E, 0x8F, 0x44, 0xBB, 0x8F, 0x02, 0x22, 0x68, 0x63, 0x97, 0xBC, 0x85, 0xBA, 0xA8, 0xF7,
					0xB5, 0x40, 0x68, 0x3C, 0x77, 0x86, 0x6F, 0x4B, 0xD7, 0x88, 0xCA, 0x8A, 0xD7, 0xCE, 0x36, 0xF0,
					0x45, 0x6E, 0xD5, 0x64, 0x79, 0x0F, 0x17, 0xFC, 0x64, 0xDD, 0x10, 0x6F, 0xF3, 0xF5, 0xE0, 0xA6,
					0xC3, 0xFB, 0x1B, 0x8C, 0x29, 0xEF, 0x8E, 0xE5, 0x34, 0xCB, 0xD1, 0x2A, 0xCE, 0x79, 0xC3, 0x9A,
					0x0D, 0x36, 0xEA, 0x01, 0xE0, 0xAA, 0x91, 0x20, 0x54, 0xF0, 0x72, 0xD8, 0x1E, 0xC7, 0x89, 0xD2
				};

				out_packet.start(game::server_packet::AddonInfo);
				for (auto &addon : addons)
				{
					UInt8 unk1 = 1;
					out_packet
						<< io::write<NetUInt8>(0x02)
						<< io::write<NetUInt8>(unk1);

					if (unk1)
					{
						UInt8 unk2 = (addon.crc != 0x1c776d01);		// 0x1c776d01 = crc used by blizzards standard addons
						out_packet
							<< io::write<NetUInt8>(unk2);
						if (unk2)
						{
							out_packet
								<< io::write_range(tdata.begin(), tdata.end());
						}

						out_packet
							<< io::write<NetUInt32>(0x00);
					}

					UInt8 unk3 = 0;
					out_packet
						<< io::write<NetUInt8>(unk3);
				}
				out_packet.finish();
			}

			void authResponse(game::OutgoingPacket &out_packet, ResponseCode code, Expansions expansion)
			{
				out_packet.start(game::server_packet::AuthResponse);
				out_packet
					<< io::write<NetUInt8>(code);

				if (code == response_code::AuthOk)
				{
					out_packet
						<< io::write<NetUInt32>(0x00)		// BillingTimeRemaining in seconds
						<< io::write<NetUInt8>(0x00)		// BillingPlanFlags
						<< io::write<NetUInt32>(0x00)		// BillingTimeRested in seconds
						<< io::write<NetUInt8>(expansion);	// 0 = Classic, 1 = TBC
				}

				out_packet.finish();
			}

			void charEnum(game::OutgoingPacket &out_packet, const game::CharEntries &characters)
			{
				out_packet.start(game::server_packet::CharEnum);

				out_packet
					<< io::write<NetUInt8>(characters.size());
				
				for (auto &entry : characters)
				{
					out_packet
						<< io::write<NetUInt64>(entry.id)
						<< io::write_range(entry.name.begin(), entry.name.end())
						<< io::write<NetUInt8>(0x00)	// 0-terminated c-style string
						<< io::write<NetUInt8>(entry.race)
						<< io::write<NetUInt8>(entry.class_)
						<< io::write<NetUInt8>(entry.gender)
						<< io::write<NetUInt8>(entry.skin)
						<< io::write<NetUInt8>(entry.face)
						<< io::write<NetUInt8>(entry.hairStyle)
						<< io::write<NetUInt8>(entry.hairColor)
						<< io::write<NetUInt8>(entry.facialHair)
						<< io::write<NetUInt8>(entry.level)
						<< io::write<NetUInt32>(entry.zoneId)				// zone
						<< io::write<NetUInt32>(entry.mapId)				// map
						<< io::write<float>(entry.x)						// x
						<< io::write<float>(entry.y)						// y
						<< io::write<float>(entry.z)						// z
						<< io::write<NetUInt32>(0x00);						// guild guid

					UInt32 charFlags = 0;
					//UInt32 playerFlags = 0;
					//UInt32 atLoginFlags = 0x20;

					out_packet
						<< io::write<NetUInt32>(charFlags)
						<< io::write<NetUInt8>(1);			// only 1 if atLoginFlags == 0x20

					// Pets info
					{
						UInt32 petDisplayId = 0;
						UInt32 petLevel = 0;
						UInt32 petFamily = 0;

						out_packet
							<< io::write<NetUInt32>(petDisplayId)
							<< io::write<NetUInt32>(petLevel)
							<< io::write<NetUInt32>(petFamily);
					}

					// Equipment
					{
						UInt32 displayInfoID = 0;
						UInt8 inventoryType = 0;
						UInt32 enchantAuraId = 0;

						for (size_t i = 0; i < 19; ++i)	// 19 = max equipment slots
						{
							out_packet
								<< io::write<NetUInt32>(displayInfoID)
								<< io::write<NetUInt8>(inventoryType)
								<< io::write<NetUInt32>(enchantAuraId);
						}

						out_packet
							<< io::write<NetUInt32>(0x00)	// first bag display id
							<< io::write<NetUInt8>(0x00)	// first bag inventory type
							<< io::write<NetUInt32>(0);		// enchant?
					}
				}

				out_packet.finish();
			}

			void charCreate(game::OutgoingPacket &out_packet, ResponseCode code)
			{
				out_packet.start(game::server_packet::CharCreate);
				out_packet
					<< io::write<NetUInt8>(code);
				out_packet.finish();
			}

			void charDelete(game::OutgoingPacket &out_packet, ResponseCode code)
			{
				out_packet.start(game::server_packet::CharDelete);
				out_packet
					<< io::write<NetUInt8>(code);
				out_packet.finish();
			}

			void charLoginFailed(game::OutgoingPacket &out_packet, ResponseCode code)
			{
				// Write packet
				out_packet.start(server_packet::CharacterLoginFailed);
				out_packet
					<< io::write<NetUInt8>(code);
				out_packet.finish();
			}

			void accountDataTimes(game::OutgoingPacket &out_packet, const std::array<UInt32, 32> &times)
			{
				out_packet.start(game::server_packet::AccountDataTimes);
				out_packet
					<< io::write_range(times);
				out_packet.finish();
			}

			void initWorldStates(game::OutgoingPacket &out_packet, UInt32 mapId, UInt32 zoneId /*TODO */)
			{
				/*const std::vector<UInt8> data =
				{
					0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x57, 0x00, 0x00, 0x00, 0x06, 0x00, 0xD8, 0x08, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD7, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD6, 0x08, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD5, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD4, 0x08, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD3, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				};*/

				out_packet.start(game::server_packet::InitWorldStates);
				out_packet
					//<< io::write_range(data);
					<< io::write<NetUInt32>(mapId)
					<< io::write<NetUInt32>(zoneId)
					<< io::write<NetUInt16>(0x00);
				out_packet.finish();
			}

			void loginVerifyWorld(game::OutgoingPacket &out_packet, UInt32 mapId, float x, float y, float z, float o)
			{
				out_packet.start(game::server_packet::LoginVerifyWorld);
				out_packet
					<< io::write<NetUInt32>(mapId)
					<< io::write<float>(x)
					<< io::write<float>(y)
					<< io::write<float>(z)
					<< io::write<float>(o);
				out_packet.finish();
			}

			void loginSetTimeSpeed(game::OutgoingPacket &out_packet, GameTime time_)
			{
				//TODO
				time_t secs = ::time(nullptr);
				tm* lt = localtime(&secs);

				// Convert
				UInt32 convertedTime = 
					static_cast<UInt32>((lt->tm_year - 100) << 24 | lt->tm_mon << 20 | (lt->tm_mday - 1) << 14 | lt->tm_wday << 11 | lt->tm_hour << 6 | lt->tm_min);

				out_packet.start(game::server_packet::LoginSetTimeSpeed);
				out_packet
					<< io::write<UInt32>(convertedTime)
					<< io::write<float>(1.0f / 60.0f);	// Update frequency is at 60 ticks / second
				out_packet.finish();
			}

			void tutorialFlags(game::OutgoingPacket &out_packet /*TODO */)
			{
				out_packet.start(game::server_packet::TutorialFlags);
				for (UInt32 i = 0; i < 8; ++i)	// 8 = number of tutorial flags
				{
					out_packet
						<< io::write<NetUInt32>(0x00000000);		// 0x00 = unchanged
				}
				out_packet.finish();
			}

			void bindPointUpdate(game::OutgoingPacket &out_packet, UInt32 mapId, UInt32 areaId, float x, float y, float z)
			{
				out_packet.start(game::server_packet::BindPointUpdate);
				out_packet
					<< io::write<float>(x)
					<< io::write<float>(y)
					<< io::write<float>(z)
					<< io::write<UInt32>(mapId)
					<< io::write<UInt32>(areaId);
				out_packet.finish();
			}

			void setRestStart(game::OutgoingPacket &out_packet)
			{
				out_packet.start(game::server_packet::SetRestStart);
				out_packet
					<< io::write<NetUInt32>(0x00);		// Unknown
				out_packet.finish();
			}

			void initializeFactions(game::OutgoingPacket &out_packet /*TODO */)
			{
				out_packet.start(game::server_packet::InitializeFactions);
				out_packet
					<< io::write<NetUInt32>(0x00000040);

				UInt32 factionCount = 0;
				for (UInt32 a = factionCount; a != 64; ++a)
				{
					out_packet 
						<< io::write<NetUInt8>(0x00)
						<< io::write<NetUInt32>(0x00000000);
				}
				
				out_packet.finish();
			}

			void initialSpells(game::OutgoingPacket &out_packet, const std::vector<const SpellEntry*> &spells)
			{
				out_packet.start(game::server_packet::InitialSpells);
				
				out_packet
					<< io::write<NetUInt8>(0x00);

				const UInt16 spellCount = spells.size();
				out_packet
					<< io::write<NetUInt16>(spellCount);

				for (UInt16 i = 0; i < spellCount; ++i)
				{
					out_packet
						<< io::write<NetUInt16>(spells[i]->id)
						<< io::write<NetUInt16>(0x00);				// On Cooldown?
				}

				//TODO
				const UInt16 spellCooldowns = 0;
				out_packet
					<< io::write<NetUInt16>(spellCooldowns);

				out_packet.finish();
			}

			void updateObject(game::OutgoingPacket &out_packet, const std::vector<std::vector<char>> &blocks)
			{
				out_packet.start(game::server_packet::UpdateObject);

				// Write blocks
				out_packet
					<< io::write<NetUInt32>(blocks.size())
					<< io::write<NetUInt8>(0x00);			// hasTransport = false

				// Append blocks uncompressed
				for (auto &block : blocks)
				{
					out_packet
						<< io::write_range(block);
				}

				out_packet.finish();
			}

			void unlearnSpells(game::OutgoingPacket &out_packet /*TODO */)
			{
				out_packet.start(game::server_packet::UnlearnSpells);
				out_packet
					<< io::write<NetUInt32>(0x00);	// Spell count

				out_packet.finish();
			}

			void motd(
				game::OutgoingPacket &out_packet,
				const std::string &motd
				)
			{
				out_packet.start(game::server_packet::Motd);
				const UInt32 sizePos = out_packet.sink().position();
				out_packet
					<< io::write<NetUInt32>(0x00);	// Line count

				std::size_t lineCount = 0;
				std::stringstream strm(motd);
				std::string line;

				while (std::getline(strm, line))
				{
					out_packet
						<< io::write_range(line) << io::write<NetUInt8>(0);
					++lineCount;
				}

				// Write correct line count
				out_packet.writePOD(sizePos, static_cast<UInt32>(lineCount));
				out_packet.finish();
			}

			void featureSystemStatus(
				game::OutgoingPacket &out_packet
				)
			{
				out_packet.start(game::server_packet::FeatureSystemStatus);
				out_packet
					<< io::write<NetUInt16>(0x02);
				out_packet.finish();
			}

			void setDungeonDifficulty(
				game::OutgoingPacket &out_packet
				)
			{
				out_packet.start(game::server_packet::SetDungeonDifficulty);
				const std::vector<UInt8> data =
				{
					0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				};

				out_packet
					<< io::write_range(data);
				out_packet.finish();
			}

			void compressedUpdateObject(game::OutgoingPacket &out_packet, const std::vector<std::vector<char>> &blocks)
			{
				// Create buffer
				std::stringstream strm;
				io::StreamSink sink(strm);
				io::Writer writer(sink);

				// Write blocks
				writer
					<< io::write<NetUInt32>(blocks.size())
					<< io::write<NetUInt8>(0x00);			// hasTransport = false

				// Append blocks uncompressed
				for (auto &block : blocks)
				{
					writer
						<< io::write_range(block);
				}

				// Get original size
				size_t origSize = writer.sink().position();

				// Compress using ZLib
				boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
				in.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
				in.push(strm);

				// Copy to output stream
				std::stringstream outStrm;
				boost::iostreams::copy(in, outStrm);
				outStrm.seekg(0, std::ios::beg);

				String buffer = outStrm.str();

				// Write packet
				out_packet.start(server_packet::CompressedUpdateObject);
				out_packet
					<< io::write<NetUInt32>(origSize)
					<< io::write_range(buffer);
				out_packet.finish();
			}

			void nameQueryResponse(game::OutgoingPacket &out_packet, UInt64 objectGuid, const String &name, const String &realmName, UInt32 raceId, UInt32 genderId, UInt32 classId)
			{
				out_packet.start(server_packet::NameQueryResponse);
				out_packet
					<< io::write<NetUInt64>(objectGuid)
					<< io::write_range(name) << io::write<NetUInt8>(0x00)	// Terminator
					<< io::write_range(realmName) << io::write<NetUInt8>(0x00)	// Terminator realm name
					<< io::write<NetUInt8>(raceId)
					<< io::write<NetUInt8>(genderId)
					<< io::write<NetUInt8>(classId)
					<< io::write<NetUInt8>(0x00);
				out_packet.finish();
			}

			void setProficiency(game::OutgoingPacket &out_packet, UInt8 itemClass, UInt32 itemSubclassMask)
			{
				out_packet.start(server_packet::SetProficiency);
				out_packet
					<< io::write<NetUInt8>(itemClass)
					<< io::write<NetUInt32>(itemSubclassMask);
				out_packet.finish();
			}

			void actionButtons(game::OutgoingPacket &out_packet, const ActionButtons &buttons)
			{
				const UInt8 maxButtons = 132;

				out_packet.start(server_packet::ActionButtons);

				// For each possible action button the player could have
				for (UInt8 button = 0; button < maxButtons; ++button)
				{
					auto btn = buttons.find(button);
					if (btn != buttons.end())
					{
						out_packet
							<< io::write<NetUInt16>(btn->second.action)
							<< io::write<NetUInt8>(btn->second.misc)
							<< io::write<NetUInt8>(btn->second.type);
					}
					else
					{
						out_packet
							<< io::write<NetUInt32>(0);
					}
				}

				out_packet.finish();
			}

			void contactList(game::OutgoingPacket &out_packet, const game::SocialInfoMap &contacts)
			{
				out_packet.start(server_packet::ContactList);
				out_packet
					<< io::write<NetUInt32>(game::social_flag::Friend | game::social_flag::Ignored | game::social_flag::Muted)
					<< io::write<NetUInt32>(contacts.size());

				for (const auto &contact : contacts)
				{
					out_packet
						<< io::write<NetUInt64>(contact.first)
						<< io::write<NetUInt32>(contact.second.flags)
						<< io::write_range(contact.second.note) << io::write<NetUInt8>(0);

					if (contact.second.flags & game::social_flag::Friend)
					{
						out_packet
							<< io::write<NetUInt8>(contact.second.status);
						if (contact.second.status)
						{
							out_packet
								<< io::write<NetUInt32>(contact.second.area)
								<< io::write<NetUInt32>(contact.second.level)
								<< io::write<NetUInt32>(contact.second.class_);
						}
					}
				}
				out_packet.finish();
			}

			void creatureQueryResponse(game::OutgoingPacket &out_packet, const UnitEntry &unit)
			{
				out_packet.start(server_packet::CreatureQueryResponse);

				out_packet
					<< io::write<NetUInt32>(unit.id)
					<< io::write_range(unit.name) << io::write<NetUInt8>(0x00)	// Terminator: name
					<< io::write<NetUInt8>(0x00)	// Terminator: name2 (always empty)
					<< io::write<NetUInt8>(0x00)	// Terminator: name3 (always empty)
					<< io::write<NetUInt8>(0x00)	// Terminator: name4 (always empty)
					<< io::write_range(unit.subname) << io::write<NetUInt8>(0x00)	// Terminator: name4 (always empty)
					<< io::write<NetUInt8>(0x00)
					<< io::write<NetUInt32>(unit.creatureTypeFlags)
					<< io::write<NetUInt32>(0x07)	//TODO: Creature Type
					<< io::write<NetUInt32>(unit.family)
					<< io::write<NetUInt32>(unit.rank)
					<< io::write<NetUInt32>(0x00)	// Unknown
					<< io::write<NetUInt32>(0x00)	//TODO: CreatureSpellData
					<< io::write<NetUInt32>(unit.maleModel)	//TODO
					<< io::write<NetUInt32>(unit.maleModel)	//TODO
					<< io::write<NetUInt32>(unit.maleModel)	//TODO
					<< io::write<NetUInt32>(unit.maleModel)	//TODO
					<< io::write<float>(1.0f)	//TODO
					<< io::write<float>(1.0f)	//TODO
					<< io::write<NetUInt8>(0x00)	//TODO Is Racial Leader
					;

				out_packet.finish();
			}

			void timeSyncReq(
				game::OutgoingPacket &out_packet,
				UInt32 counter
				)
			{
				out_packet.start(server_packet::TimeSyncReq);
				out_packet
					<< io::write<NetUInt32>(counter);
				out_packet.finish();
			}

			void monsterMove(
				game::OutgoingPacket &out_packet,
				UInt64 guid,
				const Vector<float, 3> &oldPosition,
				const Vector<float, 3> &position,
				UInt32 time
				)
			{
				out_packet.start(server_packet::MonsterMove);

				UInt8 packGUID[8 + 1];
				packGUID[0] = 0;
				size_t size = 1;

				for (UInt8 i = 0; guid != 0; ++i)
				{
					if (guid & 0xFF)
					{
						packGUID[0] |= UInt8(1 << i);
						packGUID[size] = UInt8(guid & 0xFF);
						++size;
					}

					guid >>= 8;
				}

				out_packet
					<< io::write_range(&packGUID[0], &packGUID[size])
					<< io::write<float>(oldPosition[0])
					<< io::write<float>(oldPosition[1])
					<< io::write<float>(oldPosition[2])
					<< io::write<NetUInt32>(getCurrentTime())
					<< io::write<NetUInt8>(0);

				// Movement flags
				out_packet
					<< io::write<NetUInt32>(0x01)
					<< io::write<NetUInt32>(time)
					<< io::write<NetUInt32>(1)				// One waypoint
					<< io::write<float>(position[0])
					<< io::write<float>(position[1])
					<< io::write<float>(position[2]);
				out_packet.finish();
			}

			void logoutResponse(
				game::OutgoingPacket &out_packet,
				bool success
				)
			{
				out_packet.start(server_packet::LogoutResponse);
				if (!success)
				{
					out_packet
						<< io::write<NetUInt8>(0x0C);
				}
				out_packet
					<< io::write<NetUInt32>(0)
					<< io::write<NetUInt8>(0);
				out_packet.finish();
			}

			void logoutCancelAck(
				game::OutgoingPacket &out_packet
				)
			{
				out_packet.start(server_packet::LogoutCancelAck);
				out_packet.finish();
			}

			void logoutComplete(
				game::OutgoingPacket &out_packet
				)
			{
				out_packet.start(server_packet::LogoutComplete);
				out_packet.finish();
			}

			void messageChat(game::OutgoingPacket &out_packet, ChatMsg type, Language language, const String &channelname, UInt64 targetGUID, const String &message, GameUnit *speaker)
			{
				out_packet.start(server_packet::MessageChat);
				out_packet
					<< io::write<NetUInt8>(type);

				// Language
				if ((type != chat_msg::Channel && type != chat_msg::Whisper) || language == language::Addon)
				{
					out_packet << io::write<NetUInt32>(language);
				}
				else
				{
					out_packet << io::write<NetUInt32>(language::Universal);
				}

				// TODO: Get speaker name from player if available
				static const String speakerName = "TODO";
				switch (type)
				{
					case chat_msg::MonsterSay:
					case chat_msg::MonsterParty:
					case chat_msg::MonsterYell:
					case chat_msg::MonsterWhisper:
					case chat_msg::MonsterEmote:
					case chat_msg::RaidBossWhisper:
					case chat_msg::RaidBossEmote:
					{
						assert(speaker);
						out_packet
							<< io::write<NetUInt64>(speaker->getGuid())
							<< io::write<NetUInt32>(0x00)
							<< io::write<NetUInt32>(speakerName.size() + 1)
							<< io::write_range(speakerName) << io::write<NetUInt8>(0);

						UInt64 listenerGuid = 0x00;
						out_packet
							<< io::write<NetUInt64>(listenerGuid);

						if (listenerGuid && !isPlayerGUID(listenerGuid))
						{
							out_packet
								<< io::write<NetUInt32>(0x01)		// String listener name length
								<< io::write<NetUInt8>(0x00);		// String listener name
						}

						out_packet
							<< io::write<NetUInt32>(message.size() + 1)
							<< io::write_range(message) << io::write<NetUInt8>(0)
							<< io::write<NetUInt8>(0);				// Chat-Tag always 0 since it's a creature which can't be AFK, DND etc.
						out_packet.finish();
						return;
					}
					default:
						break;
// 					default:
// 						if (type == chat_msg::Reply && type != chat_msg::Ignored && type != chat_msg::Dnd && type != chat_msg::Afk)
// 						{
// 							targetGUID = 0x00;
// 						}
// 						break;
				}

				out_packet
					<< io::write<NetUInt64>(targetGUID)
					<< io::write<NetUInt32>(0);

				if (type == chat_msg::Channel)
				{
					out_packet
						<< io::write_range(channelname) << io::write<NetUInt8>(0);
				}

				out_packet
					<< io::write<NetUInt64>(targetGUID)
					<< io::write<NetUInt32>(message.size() + 1)
					<< io::write_range(message) << io::write<NetUInt8>(0)
					<< io::write<NetUInt8>(0);			// Chat tag: 1: AFK  2: DND  4: GM
				out_packet.finish();
			}

			void movePacket(game::OutgoingPacket &out_packet, UInt16 opCode, UInt64 guid, const MovementInfo &movement)
			{
				//TODO: Check given op-code
				out_packet.start(opCode);
				
				UInt8 packGUID[8 + 1];
				packGUID[0] = 0;
				size_t size = 1;

				for (UInt8 i = 0; guid != 0; ++i)
				{
					if (guid & 0xFF)
					{
						packGUID[0] |= UInt8(1 << i);
						packGUID[size] = UInt8(guid & 0xFF);
						++size;
					}

					guid >>= 8;
				}

				out_packet
					<< io::write_range(&packGUID[0], &packGUID[size])
					<< movement;
				out_packet.finish();
			}

			void destroyObject(game::OutgoingPacket &out_packet, UInt64 guid, bool death)
			{
				out_packet.start(game::server_packet::DestroyObject);
				out_packet
					<< io::write<NetUInt64>(guid)
					<< io::write<NetUInt8>(death ? 1 : 0);
				out_packet.finish();
			}

			void standStateUpdate(game::OutgoingPacket &out_packet, UnitStandState standState)
			{
				out_packet.start(game::server_packet::StandStateUpdate);
				out_packet
					<< io::write<NetUInt8>(standState);
				out_packet.finish();
			}

			void friendStatus(game::OutgoingPacket &out_packet, UInt64 guid, game::FriendResult result, const game::SocialInfo &info)
			{
				out_packet.start(game::server_packet::FriendStatus);
				out_packet
					<< io::write<NetUInt8>(result)
					<< io::write<NetUInt64>(guid);

				switch (result)
				{
					case game::friend_result::AddedOffline:
					case game::friend_result::AddedOnline:
						out_packet << io::write_range(info.note) << io::write<NetUInt8>(0);
						break;
					default:
						break;
				}

				switch (result)
				{
					case game::friend_result::AddedOnline:
					case game::friend_result::Online:
						out_packet
							<< io::write<NetUInt8>(info.status)
							<< io::write<NetUInt32>(info.area)
							<< io::write<NetUInt32>(info.level)
							<< io::write<NetUInt32>(info.class_);
						break;
					default:
						break;
				}

				out_packet.finish();
			}

			void chatPlayerNotFound(game::OutgoingPacket &out_packet, const String &name)
			{
				out_packet.start(game::server_packet::ChatPlayerNotFound);
				out_packet
					<< io::write_range(name) << io::write<NetUInt8>(0);
				out_packet.finish();
			}

			void castFailed(game::OutgoingPacket &out_packet, game::SpellCastResult result, const SpellEntry &spell, UInt8 castCount)
			{
				out_packet.start(game::server_packet::CastFailed);
				out_packet
					<< io::write<NetUInt32>(spell.id)
					<< io::write<NetUInt8>(result)
					<< io::write<NetUInt8>(castCount);
				// TODO: Send more informations based on the cast result code (which area is required etc.)
				out_packet.finish();
			}

			void spellStart(game::OutgoingPacket &out_packet, UInt64 casterGUID, UInt64 casterItemGUID, const SpellEntry &spell, const SpellTargetMap &targets, game::SpellCastFlags castFlags, Int32 castTime, UInt8 castCount)
			{
				out_packet.start(game::server_packet::SpellStart);

				// Cast item GUID (or caster GUID if not caused by item)
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterItemGUID != 0; ++i)
					{
						if (casterItemGUID & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterItemGUID & 0xFF);
							++size;
						}

						casterItemGUID >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}

				// Cast GUID
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterGUID != 0; ++i)
					{
						if (casterGUID & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterGUID & 0xFF);
							++size;
						}

						casterGUID >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}

				out_packet
					<< io::write<NetUInt32>(spell.id)
					<< io::write<NetUInt8>(castCount)
					<< io::write<NetUInt16>(castFlags)
					<< io::write<NetInt32>(castTime)
					<< targets;

				if (castFlags && game::spell_cast_flags::Ammo)
				{
					//TODO: Ammo ids
				}
				out_packet.finish();
			}

			void spellGo(game::OutgoingPacket &out_packet, UInt64 casterGUID, UInt64 casterItemGUID, const SpellEntry &spell, const SpellTargetMap &targets, game::SpellCastFlags castFlags /*TODO: HitInformation */ /*TODO: AmmoInformation */)
			{
				out_packet.start(game::server_packet::SpellGo);
				// Cast item GUID (or caster GUID if not caused by item)
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterItemGUID != 0; ++i)
					{
						if (casterItemGUID & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterItemGUID & 0xFF);
							++size;
						}

						casterItemGUID >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}

				// Cast GUID
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterGUID != 0; ++i)
					{
						if (casterGUID & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterGUID & 0xFF);
							++size;
						}

						casterGUID >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}

				out_packet
					<< io::write<NetUInt32>(spell.id)
					<< io::write<NetUInt16>(castFlags)
					<< io::write<NetUInt32>(getCurrentTime());

				// TODO: Hit information
				{
					UInt8 unitHitCount = (targets.hasUnitTarget() ? 1 : 0);
					out_packet
						<< io::write<NetUInt8>(unitHitCount);	// Hit count
					if (unitHitCount > 0)
					{
						out_packet
							<< io::write<NetUInt64>(targets.getUnitTarget());
					}

					UInt8 goHitCount = (targets.hasGOTarget() ? 1 : 0);
					if (goHitCount > 0)
					{
						out_packet
							<< io::write<NetUInt64>(targets.getGOTarget());
					}

					out_packet
						<< io::write<NetUInt8>(0);	// Miss count
				}

				out_packet
					<< targets;

				if (castFlags && game::spell_cast_flags::Ammo)
				{
					//TODO: Ammo ids
				}

				out_packet.finish();
			}

			void spellFailure(game::OutgoingPacket &out_packet, UInt64 casterGUID, UInt32 spellId, game::SpellCastResult result)
			{
				out_packet.start(game::server_packet::SpellFailure);
				out_packet
					<< io::write<NetUInt64>(casterGUID)
					<< io::write<NetUInt32>(spellId)
					<< io::write<NetUInt8>(result);
				out_packet.finish();
			}

			void spellFailedOther(game::OutgoingPacket &out_packet, UInt64 casterGUID, UInt32 spellId)
			{
				out_packet.start(game::server_packet::SpellFailedOther);
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterGUID != 0; ++i)
					{
						if (casterGUID & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterGUID & 0xFF);
							++size;
						}

						casterGUID >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt32>(spellId);
				out_packet.finish();
			}

			void spellCooldown(game::OutgoingPacket &out_packet /*TODO */)
			{
				out_packet.start(game::server_packet::SpellCooldown);
				out_packet.finish();
			}

			void cooldownEvent(game::OutgoingPacket &out_packet /*TODO */)
			{
				out_packet.start(game::server_packet::CooldownEvent);
				out_packet.finish();
			}

			void spellNonMeleeDamageLog(game::OutgoingPacket &out_packet, UInt64 targetGuid, UInt64 casterGuid, UInt32 spellID, UInt32 damage, UInt8 damageSchoolMask, UInt32 absorbedDamage, UInt32 resistedDamage, bool PhysicalDamage, UInt32 blockedDamage, bool criticalHit)
			{
				out_packet.start(game::server_packet::SpellNonMeleeDamageLog);
				// Target GUID
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; targetGuid != 0; ++i)
					{
						if (targetGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(targetGuid & 0xFF);
							++size;
						}

						targetGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				// Caster GUID
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterGuid != 0; ++i)
					{
						if (casterGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterGuid & 0xFF);
							++size;
						}

						casterGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt32>(spellID)
					<< io::write<NetUInt32>(damage - absorbedDamage - resistedDamage - blockedDamage)
					<< io::write<NetUInt8>(damageSchoolMask)
					<< io::write<NetUInt32>(absorbedDamage)
					<< io::write<NetUInt32>(resistedDamage)
					<< io::write<NetUInt8>(PhysicalDamage)
					<< io::write<NetUInt8>(0)
					<< io::write<NetUInt32>(blockedDamage)
					<< io::write<NetUInt32>(criticalHit ? 0x27 : 0x25)
					<< io::write<NetUInt8>(0);
				out_packet.finish();
			}

			void itemQuerySingleResponse(game::OutgoingPacket &out_packet, const ItemEntry &item)
			{
				out_packet.start(game::server_packet::ItemQuerySingleResponse);
				out_packet
					<< io::write<NetUInt32>(item.id)
					<< io::write<NetUInt32>(item.itemClass)
					<< io::write<NetUInt32>(item.subClass)
					<< io::write<NetUInt32>(0)				// SoundClassOverride (TODO)
					<< io::write_range(item.name) << io::write<NetUInt8>(0)
					<< io::write<NetUInt8>(0)				// Second name?
					<< io::write<NetUInt8>(0)				// Third name?
					<< io::write<NetUInt8>(0)				// Fourth name?
					<< io::write<NetUInt32>(item.displayId)
					<< io::write<NetUInt32>(item.quality)
					<< io::write<NetUInt32>(item.flags)
					<< io::write<NetUInt32>(item.buyPrice)
					<< io::write<NetUInt32>(item.sellPrice)
					<< io::write<NetUInt32>(item.inventoryType)
					<< io::write<NetUInt32>(item.allowedRaces)
					<< io::write<NetUInt32>(item.allowedRaces)
					<< io::write<NetUInt32>(item.itemLevel)
					<< io::write<NetUInt32>(item.requiredLevel)
					<< io::write<NetUInt32>(item.requiredSkill)
					<< io::write<NetUInt32>(item.requiredSkillRank)
					<< io::write<NetUInt32>(item.requiredSpell)
					<< io::write<NetUInt32>(item.requiredHonorRank)
					<< io::write<NetUInt32>(item.requiredCityRank)
					<< io::write<NetUInt32>(item.requiredReputation)
					<< io::write<NetUInt32>(item.requiredReputationRank)
					<< io::write<NetUInt32>(item.maxCount)
					<< io::write<NetUInt32>(item.maxStack)
					<< io::write<NetUInt32>(item.containerSlots);
				for (auto &stat : item.itemStats)
				{
					out_packet
						<< io::write<NetUInt32>(stat.statType)
						<< io::write<NetUInt32>(stat.statValue);
				}
				for (auto &dmg : item.itemDamage)
				{
					out_packet
						<< io::write<float>(dmg.min)
						<< io::write<float>(dmg.max)
						<< io::write<NetUInt32>(dmg.type);
				}
				out_packet
					<< io::write<NetUInt32>(item.armor)
					<< io::write<NetUInt32>(item.holyResistance)
					<< io::write<NetUInt32>(item.fireResistance)
					<< io::write<NetUInt32>(item.natureResistance)
					<< io::write<NetUInt32>(item.frostResistance)
					<< io::write<NetUInt32>(item.shadowResistance)
					<< io::write<NetUInt32>(item.arcaneResistance)
					<< io::write<NetUInt32>(item.delay)
					<< io::write<NetUInt32>(item.ammoType)
					<< io::write<float>(0.0f)		// TODO: RangedModRange (?)
					;
				// TODO: Spells
				for (auto &spell : item.itemSpells)
				{
					if (spell.spell)
					{
						out_packet
							<< io::write<NetUInt32>(spell.spell->id)
							<< io::write<NetUInt32>(spell.trigger)
							<< io::write<NetUInt32>(-::abs(spell.charges))
							;

						// TODO: Spell cooldowns
						const bool useDBData = spell.cooldown >= 0 || spell.categoryCooldown >= 0;
						if (useDBData)
						{
							out_packet
								<< io::write<NetUInt32>(spell.cooldown)
								<< io::write<NetUInt32>(spell.category)
								<< io::write<NetUInt32>(spell.categoryCooldown);
						}
						else
						{
							out_packet
								<< io::write<NetUInt32>(-1)						// TODO
								<< io::write<NetUInt32>(0)						// TODO
								<< io::write<NetUInt32>(-1);					// TODO
						}
					}
					else
					{
						out_packet
							<< io::write<NetUInt32>(0)
							<< io::write<NetUInt32>(0)
							<< io::write<NetUInt32>(0)
							<< io::write<NetUInt32>(-1)
							<< io::write<NetUInt32>(0)
							<< io::write<NetUInt32>(-1);
					}
				}
				out_packet
					<< io::write<NetUInt32>(item.bonding)
					<< io::write_range(item.description) << io::write<NetUInt8>(0)
					<< io::write<NetUInt32>(0)					// TODO: Page Text
					<< io::write<NetUInt32>(0)					// TODO: Language ID
					<< io::write<NetUInt32>(0)					// TODO: Page Material
					<< io::write<NetUInt32>(0)					// TODO: Start Quest
					<< io::write<NetUInt32>(item.lockId)
					<< io::write<NetUInt32>(0)					// TODO: Material
					<< io::write<NetUInt32>(item.sheath)
					<< io::write<NetUInt32>(item.randomProperty)
					<< io::write<NetUInt32>(item.randomSuffix)
					<< io::write<NetUInt32>(item.block)
					<< io::write<NetUInt32>(item.set)
					<< io::write<NetUInt32>(item.durability)
					<< io::write<NetUInt32>(item.area)
					<< io::write<NetUInt32>(item.map)			// Added in 1.12.X
					<< io::write<NetUInt32>(item.bagFamily)
					<< io::write<NetUInt32>(item.totemCategory)
					;
				// TODO: Sockets
				for (size_t i = 0; i < 3; ++i)
				{
					out_packet
						<< io::write<NetUInt32>(0)
						<< io::write<NetUInt32>(0);
				}
				out_packet
					<< io::write<NetUInt32>(item.socketBonus)
					<< io::write<NetUInt32>(item.gemProperties)
					<< io::write<NetUInt32>(item.requiredDisenchantSkill)
					<< io::write<float>(0.0f)					// TODO: Armor Damage Modifier
					<< io::write<NetUInt32>(0);					// Added in 2.4.2.8209
				out_packet.finish();
			}

			void spellEnergizeLog(game::OutgoingPacket &out_packet, UInt64 targetGuid, UInt64 casterGuid, UInt32 spellID, UInt8 powerType, UInt32 amount)
			{
				out_packet.start(game::server_packet::SpellEnergizeLog);
				// Target GUID
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; targetGuid != 0; ++i)
					{
						if (targetGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(targetGuid & 0xFF);
							++size;
						}

						targetGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				// Caster GUID
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterGuid != 0; ++i)
					{
						if (casterGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterGuid & 0xFF);
							++size;
						}

						casterGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt32>(spellID)
					<< io::write<NetUInt32>(powerType)
					<< io::write<NetUInt32>(amount);
				out_packet.finish();
			}

			void attackStart(game::OutgoingPacket &out_packet, UInt64 attackerGUID, UInt64 attackedGUID)
			{
				out_packet.start(game::server_packet::AttackStart);
				out_packet
					<< io::write<NetUInt64>(attackerGUID)
					<< io::write<NetUInt64>(attackedGUID);
				out_packet.finish();
			}

			void attackStop(game::OutgoingPacket &out_packet, UInt64 attackerGUID, UInt64 attackedGUID)
			{
				out_packet.start(game::server_packet::AttackStop);
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; attackerGUID != 0; ++i)
					{
						if (attackerGUID & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(attackerGUID & 0xFF);
							++size;
						}

						attackerGUID >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; attackedGUID != 0; ++i)
					{
						if (attackedGUID & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(attackedGUID & 0xFF);
							++size;
						}

						attackedGUID >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt32>(0);		// TODO
				out_packet.finish();
			}

			void attackStateUpdate(game::OutgoingPacket &out_packet, UInt64 attackerGUID, UInt64 attackedGUID, HitInfo hitInfo, UInt32 totalDamage, UInt32 absorbedDamage, UInt32 resistedDamage, UInt32 blockedDamage, VictimState targetState, WeaponAttack swingType, UInt32 damageSchool)
			{
				out_packet.start(game::server_packet::AttackerStateUpdate);
				out_packet
					<< io::write<NetUInt32>(hitInfo);
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; attackerGUID != 0; ++i)
					{
						if (attackerGUID & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(attackerGUID & 0xFF);
							++size;
						}

						attackerGUID >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; attackedGUID != 0; ++i)
					{
						if (attackedGUID & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(attackedGUID & 0xFF);
							++size;
						}

						attackedGUID >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}

				const UInt32 realDamage = (totalDamage - absorbedDamage - resistedDamage - blockedDamage);

				out_packet
					<< io::write<NetUInt32>(realDamage)
					<< io::write<NetUInt8>(1)
					<< io::write<NetUInt32>(damageSchool)
					<< io::write<float>(realDamage)				// WTF?
					<< io::write<NetUInt32>(realDamage)
					<< io::write<NetUInt32>(absorbedDamage)
					<< io::write<NetUInt32>(resistedDamage)
					<< io::write<NetUInt32>(targetState)
					<< io::write<NetUInt32>(absorbedDamage == 0 ? 0 : -1)
					<< io::write<NetUInt32>(0)					// Generated rage?
					<< io::write<NetUInt32>(blockedDamage);
				out_packet.finish();
			}

			void attackSwingBadFacing(game::OutgoingPacket &out_packet)
			{
				out_packet.start(game::server_packet::AttackSwingBadFacing);
				out_packet.finish();
			}

			void attackSwingCantAttack(game::OutgoingPacket &out_packet)
			{
				out_packet.start(game::server_packet::AttackSwingCantAttack);
				out_packet.finish();
			}

			void attackSwingDeadTarget(game::OutgoingPacket &out_packet)
			{
				out_packet.start(game::server_packet::AttackSwingDeadTarget);
				out_packet.finish();
			}

			void attackSwingNotStanding(game::OutgoingPacket &out_packet)
			{
				out_packet.start(game::server_packet::AttackSwingNotStanding);
				out_packet.finish();
			}

			void attackSwingNotInRange(game::OutgoingPacket &out_packet)
			{
				out_packet.start(game::server_packet::AttackSwingNotInRange);
				out_packet.finish();
			}

			void inventoryChangeFailure(game::OutgoingPacket &out_packet, InventoryChangeFailure failure, GameItem *itemA, GameItem* itemB)
			{
				out_packet.start(game::server_packet::InventoryChangeFailure);
				out_packet << io::write<NetUInt8>(failure);

				if (failure != inventory_change_failure::Okay)
				{
					out_packet 
						<< io::write<NetUInt64>((itemA != nullptr) ? itemA->getGuid() : 0)
						<< io::write<NetUInt64>((itemB != nullptr) ? itemB->getGuid() : 0)
						<< io::write<NetUInt8>(0);

					if (failure == inventory_change_failure::CantEquipLevel)
					{
						UInt32 level = 0;
						if (itemA != nullptr)
						{
							level = itemA->getEntry().requiredLevel;
						}

						out_packet << io::write<NetUInt32>(level);                          // new 2.4.0
					}
				}
				out_packet.finish();
			}

			void updateComboPoints(game::OutgoingPacket &out_packet, UInt64 targetGuid, UInt8 comboPoints)
			{
				out_packet.start(game::server_packet::UpdateComboPoints);
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; targetGuid != 0; ++i)
					{
						if (targetGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(targetGuid & 0xFF);
							++size;
						}

						targetGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt8>(comboPoints);
				out_packet.finish();
			}

			void spellHealLog(game::OutgoingPacket &out_packet, UInt64 targetGuid, UInt64 casterGuid, UInt32 spellId, UInt32 amount, bool critical)
			{
				out_packet.start(game::server_packet::SpellHealLog);
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; targetGuid != 0; ++i)
					{
						if (targetGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(targetGuid & 0xFF);
							++size;
						}

						targetGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterGuid != 0; ++i)
					{
						if (casterGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterGuid & 0xFF);
							++size;
						}

						casterGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt32>(spellId)
					<< io::write<NetUInt32>(amount)
					<< io::write<NetUInt8>(critical ? 1 : 0)
					<< io::write<NetUInt8>(0);
				out_packet.finish();
			}

			void periodicAuraLog(game::OutgoingPacket &out_packet, UInt64 targetGuid, UInt64 casterGuid, UInt32 spellId, UInt32 auraType, UInt32 damage, UInt32 dmgSchool, UInt32 absorbed, UInt32 resisted)
			{
				out_packet.start(game::server_packet::PeriodicAuraLog);
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; targetGuid != 0; ++i)
					{
						if (targetGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(targetGuid & 0xFF);
							++size;
						}

						targetGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterGuid != 0; ++i)
					{
						if (casterGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterGuid & 0xFF);
							++size;
						}

						casterGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt32>(spellId)
					<< io::write<NetUInt32>(1)
					<< io::write<NetUInt32>(auraType)
					<< io::write<NetUInt32>(damage)
					<< io::write<NetUInt32>(dmgSchool)
					<< io::write<NetUInt32>(absorbed)
					<< io::write<NetUInt32>(resisted);
				out_packet.finish();
			}

			void periodicAuraLog(game::OutgoingPacket &out_packet, UInt64 targetGuid, UInt64 casterGuid, UInt32 spellId, UInt32 auraType, UInt32 heal)
			{
				out_packet.start(game::server_packet::PeriodicAuraLog);
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; targetGuid != 0; ++i)
					{
						if (targetGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(targetGuid & 0xFF);
							++size;
						}

						targetGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; casterGuid != 0; ++i)
					{
						if (casterGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(casterGuid & 0xFF);
							++size;
						}

						casterGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt32>(spellId)
					<< io::write<NetUInt32>(1)
					<< io::write<NetUInt32>(auraType)
					<< io::write<NetUInt32>(heal);
				out_packet.finish();
			}

			void updateAuraDuration(game::OutgoingPacket &out_packet, UInt8 slot, UInt32 durationMS)
			{
				out_packet.start(game::server_packet::UpdateAuraDuration);
				out_packet
					<< io::write<NetUInt8>(slot)
					<< io::write<NetUInt32>(durationMS);
				out_packet.finish();
			}

			void setExtraAuraInfo(game::OutgoingPacket &out_packet, UInt64 targetGuid, UInt8 slot, UInt32 spellId, UInt32 maxDurationMS, UInt32 durationMS)
			{
				out_packet.start(game::server_packet::SetExtraAuraInfo);
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; targetGuid != 0; ++i)
					{
						if (targetGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(targetGuid & 0xFF);
							++size;
						}

						targetGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt8>(slot)
					<< io::write<NetUInt32>(spellId)
					<< io::write<NetUInt32>(maxDurationMS)
					<< io::write<NetUInt32>(durationMS);
				out_packet.finish();
			}

			void setExtraAuraInfoNeedUpdate(game::OutgoingPacket &out_packet, UInt64 targetGuid, UInt8 slot, UInt32 spellId, UInt32 maxDurationMS, UInt32 durationMS)
			{
				out_packet.start(game::server_packet::SetExtraAuraInfoNeedUpdate);
				{
					UInt8 packGUID[8 + 1];
					packGUID[0] = 0;
					size_t size = 1;

					for (UInt8 i = 0; targetGuid != 0; ++i)
					{
						if (targetGuid & 0xFF)
						{
							packGUID[0] |= UInt8(1 << i);
							packGUID[size] = UInt8(targetGuid & 0xFF);
							++size;
						}

						targetGuid >>= 8;
					}

					out_packet
						<< io::write_range(&packGUID[0], &packGUID[size]);
				}
				out_packet
					<< io::write<NetUInt8>(slot)
					<< io::write<NetUInt32>(spellId)
					<< io::write<NetUInt32>(maxDurationMS)
					<< io::write<NetUInt32>(durationMS);
				out_packet.finish();
			}

			void logXPGain(game::OutgoingPacket &out_packet, UInt64 victimGUID, UInt32 givenXP, UInt32 restXP, bool hasReferAFriendBonus)
			{
				out_packet.start(game::server_packet::LogXPGain);
				out_packet
					<< io::write<NetUInt64>(victimGUID)
					<< io::write<NetUInt32>(givenXP + restXP)
					<< io::write<NetUInt8>(victimGUID != 0 ? 1 : 0);
				if (victimGUID != 0)
				{
					out_packet
						<< io::write<NetUInt32>(givenXP)
						<< io::write<float>(1.0f);
				}
				out_packet
					<< io::write<NetUInt8>(hasReferAFriendBonus ? 1 : 0);
				out_packet.finish();
			}
		}

		namespace client_read
		{
			bool ping(io::Reader &packet, UInt32 &out_ping, UInt32 &out_latency)
			{
				return packet
					>> io::read<NetUInt32>(out_ping)
					>> io::read<NetUInt32>(out_latency);
			}

			bool authSession(io::Reader &packet, UInt32 &out_clientBuild, String &out_Account, UInt32 &out_clientSeed, SHA1Hash &out_digest, AddonEntries &out_addons)
			{
				UInt32 unknown;

				if (!(packet
					>> io::read<NetUInt32>(out_clientBuild)
					>> io::read<NetUInt32>(unknown)))
				{
					return false;
				}

				// Read account name
				out_Account.clear();
				char c = 0x00;
				do 
				{
					if (!(packet >> c))
					{
						return false;
					}

					if (c != 0)
					{
						out_Account.push_back(c);
					}
				} while (c != 0);

				UInt32 addonSize;
				if (!(packet
					>> io::read<NetUInt32>(out_clientSeed)
					>> io::read_range(out_digest.begin(), out_digest.end())
					>> io::read<NetUInt32>(addonSize)))
				{
					return false;
				}

				// Validate addon size
				if (addonSize == 0 || addonSize > 0xFFFFF)
				{
					return false;
				}

				// Get remaining packet size
				const size_t remainingSize = packet.getSource()->size() - packet.getSource()->position();

				// Create buffer
				String buffer;
				buffer.resize(remainingSize);
				packet.getSource()->read(&buffer[0], remainingSize);
				std::istringstream inStrm(buffer);

				// Uncompress using ZLib
				boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
				in.push(boost::iostreams::zlib_decompressor());
				in.push(inStrm);

				std::stringstream outStrm;
				boost::iostreams::copy(in, outStrm);
				outStrm.seekg(0);

				io::StreamSource source(outStrm);
				io::Reader addonReader(source);

				while (!source.end())
				{
					AddonEntry addon;
					
					// Read 0-terminated c-style string from stream
					char c = 0;
					do 
					{
						if (!(addonReader >> c))
						{
							break;
						}

						if (c != 0)
						{
							addon.addonNames.push_back(c);
						}
					} while (c != 0);

					// Read other addon values
					if (!(addonReader
						>> io::read<NetUInt32>(addon.crc)
						>> io::read<NetUInt32>(addon.unk7)
						>> io::read<NetUInt8>(addon.unk6)))
					{
						break;
					}

					out_addons.push_back(addon);
				}

				return packet;
			}

			bool charEnum(io::Reader &packet)
			{
				// This is an empty packet
				return packet;
			}

			bool charCreate(io::Reader &packet, CharEntry &out_entry)
			{
				// Read 0-terminated name
				out_entry.name.clear();
				char c = 0;
				do 
				{
					// Can no longer read
					if (!(packet >> c))
					{
						break;
					}

					if (c != 0)
					{
						out_entry.name.push_back(c);
					}
				} while (c != 0);

				if (!(packet
					>> io::read<NetUInt8>(out_entry.race)
					>> io::read<NetUInt8>(out_entry.class_)
					>> io::read<NetUInt8>(out_entry.gender)
					>> io::read<NetUInt8>(out_entry.skin)
					>> io::read<NetUInt8>(out_entry.face)
					>> io::read<NetUInt8>(out_entry.hairStyle)
					>> io::read<NetUInt8>(out_entry.hairColor)
					>> io::read<NetUInt8>(out_entry.facialHair)
					>> io::read<NetUInt8>(out_entry.outfitId)))
				{
					return false;
				}

				return packet;
			}

			bool charDelete(io::Reader &packet, DatabaseId &out_characterId)
			{
				return packet
					>> io::read<NetUInt64>(out_characterId);
			}

			bool nameQuery(io::Reader &packet, NetUInt64 &out_objectId)
			{
				return packet
					>> io::read<NetUInt64>(out_objectId);
			}

			bool playerLogin(io::Reader &packet, DatabaseId &out_characterId)
			{
				return packet
					>> io::read<NetUInt64>(out_characterId);
			}

			bool creatureQuery(io::Reader &packet, UInt32 &out_entry, UInt64 &out_guid)
			{
				return packet
					>> io::read<NetUInt32>(out_entry)
					>> io::read<NetUInt64>(out_guid);
			}

			bool playerLogout(
				io::Reader &packet
				)
			{
				return packet;
			}

			bool logoutRequest(
				io::Reader &packet
				)
			{
				return packet;
			}

			bool logoutCancel(
				io::Reader &packet
				)
			{
				return packet;
			}

			bool moveStop(io::Reader &packet, MovementInfo &out_info)
			{
				return packet
					>> out_info;
			}

			bool moveStartForward(io::Reader &packet, MovementInfo &out_info)
			{
				return packet
					>> out_info;
			}

			bool moveStartBackward(io::Reader &packet, MovementInfo &out_info)
			{
				return packet
					>> out_info;
			}

			bool moveHeartBeat(io::Reader &packet, MovementInfo &out_info)
			{
				return packet
					>> out_info;
			}

			bool setSelection(io::Reader &packet, UInt64 &out_targetGUID)
			{
				return packet
					>> io::read<NetUInt64>(out_targetGUID);
			}

			bool standStateChange(io::Reader &packet, UnitStandState &out_standState)
			{
				return packet
					>> io::read<NetUInt32>(out_standState);
			}

			bool messageChat(io::Reader &packet, ChatMsg &out_type, Language &out_lang, String &out_to, String &out_channel, String &out_message)
			{
				packet
					>> io::read<NetUInt32>(out_type)
					>> io::read<NetUInt32>(out_lang);

				// TODO: Prevent player to speak in unknown language
				switch (out_type)
				{
					case chat_msg::Say:
					case chat_msg::Emote:
					case chat_msg::Yell:
					case chat_msg::Party:
					case chat_msg::Guild:
					case chat_msg::Officer:
					case chat_msg::Raid:
					case chat_msg::RaidLeader:
					case chat_msg::RaidWarning:
					case chat_msg::Battleground:
					case chat_msg::BattlegroundLeader:
					case chat_msg::Afk:
					case chat_msg::Dnd:
					{
						char c = 0x00;
						do
						{
							if (!(packet >> c))
							{
								return false;
							}
							if (c != 0)
							{
								out_message.push_back(c);
							}
						} while (c != 0);

						break;
					}
					case chat_msg::Whisper:
					{
						char c = 0x00;
						do
						{
							if (!(packet >> c))
							{
								return false;
							}
							if (c != 0)
							{
								out_to.push_back(c);
							}
						} while (c != 0);

						do
						{
							if (!(packet >> c))
							{
								return false;
							}
							if (c != 0)
							{
								out_message.push_back(c);
							}
						} while (c != 0);

						break;
					}
					case chat_msg::Channel:
					{
						char c = 0x00;
						do
						{
							if (!(packet >> c))
							{
								return false;
							}
							if (c != 0)
							{
								out_channel.push_back(c);
							}
						} while (c != 0);

						do
						{
							if (!(packet >> c))
							{
								return false;
							}
							if (c != 0)
							{
								out_message.push_back(c);
							}
						} while (c != 0);

						break;
					}
					default:
						// Unknown / Unhandled chat type
						return false;
				}

				return packet;
			}

			bool contactList(io::Reader &packet)
			{
				// Skip the next four bytes as they don't seem to be important...
				packet.skip(4);
				return packet;
			}

			bool addFriend(io::Reader &packet, String &out_name, String &out_note)
			{
				char c = 0x00;
				do
				{
					if (!(packet >> c))
					{
						return false;
					}
					if (c != 0)
					{
						out_name.push_back(c);
					}
				} while (c != 0);

				do
				{
					if (!(packet >> c))
					{
						return false;
					}
					if (c != 0)
					{
						out_note.push_back(c);
					}
				} while (c != 0);

				return true;
			}

			bool deleteFriend(io::Reader &packet, UInt64 &out_guid)
			{
				return packet
					>> io::read<NetUInt64>(out_guid);
			}

			bool setContactNotes(io::Reader &packet /*TODO */)
			{
				//TODO
				return packet;
			}

			bool addIgnore(io::Reader &packet, String &out_name)
			{
				char c = 0x00;
				do
				{
					if (!(packet >> c))
					{
						return false;
					}
					if (c != 0)
					{
						out_name.push_back(c);
					}
				} while (c != 0);

				return true;
			}

			bool deleteIgnore(io::Reader &packet, UInt64 &out_guid)
			{
				return packet
					>> io::read<NetUInt64>(out_guid);
			}

			bool castSpell(io::Reader &packet, UInt32 &out_spellID, UInt8 &out_castCount, SpellTargetMap &out_targetMap)
			{
				return packet
					>> io::read<NetUInt32>(out_spellID)
					>> io::read<NetUInt8>(out_castCount)
					>> out_targetMap;
			}

			bool cancelCast(io::Reader &packet, UInt32 &out_spellID)
			{
				return packet
					>> io::read<NetUInt32>(out_spellID);
			}

			bool itemQuerySingle(io::Reader &packet, UInt32 &out_itemID)
			{
				return packet
					>> io::read<NetUInt32>(out_itemID);
			}

			bool attackSwing(io::Reader &packet, UInt64 &out_targetGUID)
			{
				return packet
					>> io::read<NetUInt64>(out_targetGUID);
			}

			bool attackStop(io::Reader &packet)
			{
				return true;
			}

			bool setSheathed(io::Reader &packet, UInt32 &out_sheath)
			{
				return packet
					>> io::read<NetUInt32>(out_sheath);
			}

			bool togglePvP(io::Reader &packet, UInt8 &out_enabled)
			{
				return packet
					>> io::read<NetUInt8>(out_enabled);
			}

			bool autoStoreLootItem(io::Reader &packet, UInt8 &out_lootSlot)
			{
				return packet
					>> io::read<NetUInt8>(out_lootSlot);
			}

			bool autoEquipItem(io::Reader &packet, UInt8 &out_srcBag, UInt8 &out_srcSlot)
			{
				return packet
					>> io::read<NetUInt8>(out_srcBag)
					>> io::read<NetUInt8>(out_srcSlot);
			}

			bool autoStoreBagItem(io::Reader &packet, UInt8 &out_srcBag, UInt8 &out_srcSlot, UInt8 &out_dstBag)
			{
				return packet
					>> io::read<NetUInt8>(out_srcBag)
					>> io::read<NetUInt8>(out_srcSlot)
					>> io::read<NetUInt8>(out_dstBag);
			}

			bool swapItem(io::Reader &packet, UInt8 &out_dstBag, UInt8 &out_dstSlot, UInt8 &out_srcBag, UInt8 &out_srcSlot)
			{
				return packet
					>> io::read<NetUInt8>(out_dstBag)
					>> io::read<NetUInt8>(out_dstSlot)
					>> io::read<NetUInt8>(out_srcBag)
					>> io::read<NetUInt8>(out_srcSlot);
			}

			bool swapInvItem(io::Reader &packet, UInt8 &out_srcSlot, UInt8 &out_dstSlot)
			{
				return packet
					>> io::read<NetUInt8>(out_srcSlot)
					>> io::read<NetUInt8>(out_dstSlot);
			}

			bool splitItem(io::Reader &packet, UInt8 &out_srcBag, UInt8 &out_srcSlot, UInt8 &out_dstBag, UInt8 &out_dstSlot, UInt8 &out_count)
			{
				return packet
					>> io::read<NetUInt8>(out_srcBag)
					>> io::read<NetUInt8>(out_srcSlot)
					>> io::read<NetUInt8>(out_dstBag)
					>> io::read<NetUInt8>(out_dstSlot)
					>> io::read<NetUInt8>(out_count);
			}

			bool autoEquipItemSlot(io::Reader &packet, UInt64 &out_itemGUID, UInt8 &out_dstSlot)
			{
				return packet
					>> io::read<NetUInt64>(out_itemGUID)
					>> io::read<NetUInt8>(out_dstSlot);
			}

			bool destroyItem(io::Reader &packet, UInt8 &out_bag, UInt8 &out_slot, UInt8 &out_count, UInt8 &out_data1, UInt8 &out_data2, UInt8 &out_data3)
			{
				return packet
					>> io::read<NetUInt8>(out_bag)
					>> io::read<NetUInt8>(out_slot)
					>> io::read<NetUInt8>(out_count)
					>> io::read<NetUInt8>(out_data1)
					>> io::read<NetUInt8>(out_data2)
					>> io::read<NetUInt8>(out_data3);
			}
		}
	}
}
