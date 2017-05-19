#pragma once

#include "Singleton.h"
#include "Storage/SerializationObject.h"

#include <boost/filesystem.hpp>

#include <unordered_map>
#include <string>
#include <fstream>

using path_t = boost::filesystem::path;

namespace SKU::Storage {

	using SerializationMap = std::unordered_map<std::string, SerializationObject>;

	/*	File Format
		=========================================================

		<header> CRLFCRLF
		<version specific>

		=========================================================
		-	File Header
		=========================================================

		<magic: uint32_t>
		<version: uint32_t>
	*/

	namespace SaveGame
	{
		const uint32_t Magic = 'SKUS';
		const uint32_t Separator = '\r\n\r\n';

		struct Header
		{
			uint32_t magic;
			uint32_t version;
		};
	}

	namespace SaveGame::V1
	{
		/*	Version 1 Specific Format
			=========================================================

			<object header 0>
			<object data 0>
			..
			..
			..
			<object header N>
			<object data N>

			=========================================================
			-	-	Object Header Entry
			=========================================================

			<type_id: uint8_t[36]>
			<version: uint32_t>
			<size: uint32_t>
			<data: uint8_t[size]>
		*/

		const uint32_t Version = 1;
		
		struct SerializedObject
		{
			char type_id[GUID_SIZE];
			uint32_t version;
			uint32_t size;			
		};
	}

	class SaveGameManager : public Singleton<SaveGameManager>
	{
		IS_SINGLETON_CLASS(SaveGameManager)

		friend class Interface;
		
		protected:
			void OnSaveGame(const std::string &savegame_name);
			bool OnLoadGame(const std::string &savegame_name);
			void OnDeleteGame(const std::string &savegame_name);
			
		private:
			bool LoadGameV1(std::ifstream &savegame);
			void SaveGameV1(std::ofstream &savegame);

		public:
			void RegisterObject(SerializationObject& object);
			
		private:
			path_t savegame_folder;
			path_t current_savegame_path;

			SerializationMap serialization_object_map;
	};
}
