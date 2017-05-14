#include "Storage/StorageInterface.h"
#include "Plugin.h"

#include <skse/PluginAPI.h>

namespace SKU::Storage {

	Interface::Interface()
	{
		// TODO: locate savegame folder, fill savegame set 
	}

	Interface::~Interface()
	{

	}

	void Interface::OnSKSEMessage(SKSEMessagingInterface::Message *message)
	{
		switch (message->type)
		{
			case SKSEMessagingInterface::kMessage_NewGame:
			{
				OnNewGame();
			} break;

			case SKSEMessagingInterface::kMessage_SaveGame:
			{
				OnSaveGame(std::string((const char *)message->data, message->dataLen));
			} break;

			case SKSEMessagingInterface::kMessage_PostLoadGame:
			{
				OnLoadGame(std::string((const char *)message->data, message->dataLen));
			} break;

			case SKSEMessagingInterface::kMessage_DeleteGame:
			{
				OnDeleteGame(std::string((const char *)message->data, message->dataLen));
			} break;
		}
	}

	void Interface::OnSaveGame(const std::string &savegame_name)
	{
		boost::filesystem::path savegame_path = savegame_folder;
		savegame_path.append(savegame_name);

		if (current_savegame == nullptr)
		{
			Plugin::Log(LOGL_WARNING, "Storage: Attempted to save game but no game was loaded. Skipping.");
			return;
		}

		Plugin::Log(LOGL_INFO, "Storage: Loading '%s'.", savegame_path.c_str());

		current_savegame->Save(savegame_path);
	}

	void Interface::OnLoadGame(const std::string &savegame_name)
	{
		boost::filesystem::path savegame_path = savegame_folder;
		savegame_path.append(savegame_name);

		Plugin::Log(LOGL_INFO, "Storage: Saving '%s'.", savegame_path.c_str());

		current_savegame = SaveGame::Load(savegame_name);
	}

	void Interface::OnNewGame()
	{
		current_savegame = SaveGame::Create();
	}

	void Interface::OnDeleteGame(const std::string &savegame_name)
	{
		if (savegame_set.find(savegame_name) == savegame_set.end())
		{
			Plugin::Log(LOGL_INFO, "Storage: Attempted to delete '%s' which couldn't be found. Skipping.", savegame_name.c_str());
			return;
		}
			
		boost::filesystem::path savegame_path = savegame_folder;
		savegame_path.append(savegame_name);

		Plugin::Log(LOGL_INFO, "Storage: Removing '%s'.", savegame_path.c_str());

		//TODO: check if deletion savegame is the current savegame
		//boost::filesystem::remove(savegame_path);
	}
}