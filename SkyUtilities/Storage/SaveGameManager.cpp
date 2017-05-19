#include "Storage/SaveGameManager.h"

#include "Plugin.h"

#include <shlobj.h>

#include <exception>
#include <cstring>

namespace SKU::Storage {

	//
	// SaveGame
	//
	namespace SaveGame {
		inline
		bool ReadHeader(std::ifstream &savegame, SaveGame::Header &header)
		{
			if (savegame.is_open() == false
			|| savegame.bad() == true)
			{
				return false;
			}

			try
			{
				savegame.read(reinterpret_cast<char*>(&header), sizeof(Header));
			}
			catch (std::exception)
			{
				return false;
			}

			return true;
		}

		inline
		SaveGame::Header GenerateHeader(uint32_t version)
		{
			SaveGame::Header header;

			header.magic = SaveGame::Magic;
			header.version = version;

			return header;
		}
	}

	// ====================================================
	// SaveGame::V1
	// ====================================================
	
	namespace SaveGame::V1 {
		inline
		bool LoadObject(std::ifstream &savegame, SaveGame::V1::SerializedObject &object, std::vector<char> &serialized_data)
		{
			savegame.read(object.type_id, GUID_SIZE);

			savegame >> object.version;
			savegame >> object.size;

			serialized_data.resize(object.size);
			savegame.read(&serialized_data[0], object.size);

			return true;
		}
	}

	// ====================================================
	// SaveGameManager
	// ====================================================
	SaveGameManager::SaveGameManager()
	{
		char path[MAX_PATH] = { 0 };
		HRESULT res = SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path);

		if (res != S_OK)
		{
			Plugin::Log(LOGL_CRITICAL, "SaveGameManager: Unable to locate personal documents folder. Stopping.");
			throw std::bad_exception();
		}

		savegame_folder = boost::filesystem::path(path);
		savegame_folder.append(PLUGIN_RELATIVE_SAVES_PATH);
	}

	SaveGameManager::~SaveGameManager()
	{

	}

	void SaveGameManager::OnSaveGame(const std::string &savegame_name)
	{
		std::ofstream savegame((current_savegame_path = savegame_folder / savegame_name).c_str());				

		Plugin::Log(LOGL_INFO, "SaveGameManager: Saving '%s'.", current_savegame_path.c_str());

		if (savegame.fail() == true)
		{
			Plugin::Log(LOGL_WARNING, "SaveGameManager: Unable to write to file and thus not create a save."); // TODO: This is actually a critical error and should stop the plugin completely.
			return;
		}		

		try
		{
			SaveGameV1(savegame);
		}
		catch (const std::exception)
		{

		}
	}

	bool SaveGameManager::OnLoadGame(const std::string &savegame_name)
	{
		std::ifstream savegame((current_savegame_path = savegame_folder / savegame_name).c_str());
		const char *reason = "";
		SaveGame::Header header;		

		Plugin::Log(LOGL_INFO, "SaveGameManager: Loading '%s'.", current_savegame_path.c_str());

		if (savegame.fail() == true)
		{
			reason = "unable to open file";
		}
		else if (SaveGame::ReadHeader(savegame, header) == false)
		{
			reason = "unable to read header. Corrupt save?";
		}		
		else if (header.magic != SaveGame::Magic)
		{
			reason = "file is not a valid save";
		}
		else
		{
			switch (header.version)
			{
				case SaveGame::V1::Version:
				{
					if (LoadGameV1(savegame) == true)
					{
						return true;
					}

					reason = "corrupt save";
				} break;

				default:
				{
					reason = "version not recognized. Save corrupt.";
				} break;
			}
		}

		Plugin::Log(LOGL_INFO, "SaveGameManager: Loading '%s' has failed (reason: %s).", 
			current_savegame_path.c_str(), reason);

		return false;
	}
	
	void SaveGameManager::OnDeleteGame(const std::string &savegame_name)
	{
		path_t savegame_path = savegame_folder / savegame_name;

		Plugin::Log(LOGL_INFO, "SaveGameManager: Removal of save is not implemented yet.");

		//Plugin::Log(LOGL_INFO, "Storage: Removing '%s'.", savegame_path.c_str());

		//TODO: check if deletion savegame is the current savegame
		//boost::filesystem::remove(savegame_path);
	}

	//
	// Version 1
	bool SaveGameManager::LoadGameV1(std::ifstream &savegame)
	{
		SaveGame::V1::SerializedObject object;

		try
		{
			while (savegame.eof() == false || savegame.bad() == false)
			{
				std::vector<char> serialized_data;

				if (SaveGame::V1::LoadObject(savegame, object, serialized_data) == true)
				{
					if (serialization_object_map.find(object.type_id) == serialization_object_map.end())
						continue;

					std::stringstream object_data_stream(std::string(&serialized_data[0], object.size));
					
					if (serialization_object_map.at(object.type_id).Deserialize(object_data_stream, object.version) == false)
					{
						Plugin::Log(LOGL_WARNING, "SaveGameManager: Unable to propery load savegame. Object (type: %36s) couldn't deserialize its data.",
							object.type_id);
					}
				}
			}
		}
		catch (std::exception)
		{
			return false;
		}

		return !savegame.bad();
	}

	void SaveGameManager::SaveGameV1(std::ofstream &savegame)
	{
		SaveGame::Header header = SaveGame::GenerateHeader(SaveGame::V1::Version);

		savegame.write((char *)&header, sizeof(SaveGame::Header));
		savegame.write((char *)SaveGame::Separator, sizeof(SaveGame::Separator));
		
		for (auto &object : serialization_object_map)
		{
			SaveGame::V1::SerializedObject serialized_object;
			std::stringstream serialized_data;

			
			serialized_object.version = object.second.GetClassVersion();
			serialized_object.size = serialized_data.tellg();
			
			if (object.second.GetClassID() == nullptr
			|| serialized_object.version == 0)
			{
				Plugin::Log(LOGL_WARNING, "SaveGameManager: Invalid SerializationObject (version: %d, id: %d). Skipping.",
					object.second.GetClassID() == nullptr ? "invalid" : "valid",
					serialized_object.version);

				continue;
			}

			memcpy(serialized_object.type_id, object.second.GetClassID(), GUID_SIZE);

			object.second.Serialize(serialized_data);

			savegame.write((char*)&serialized_object, sizeof(SaveGame::V1::SerializedObject));
			savegame.write(serialized_data.str().c_str(), serialized_object.size);
		}
	}

	//
	// Serialization
	void SaveGameManager::RegisterObject(SerializationObject& object)
	{
		serialization_object_map.try_emplace(object.GetClassID(), object);
	}
}