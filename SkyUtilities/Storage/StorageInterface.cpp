#include "Storage/StorageInterface.h"
#include "Storage/SaveGameManager.h"
#include "Plugin.h"

#include <skse/PluginAPI.h>

namespace SKU::Storage {

	Interface::Interface()
	{
		Plugin::Log(LOGL_INFO, "Storage: Initializing.");
		// TODO: locate savegame folder, fill savegame set 
	}

	Interface::~Interface()
	{

	}

	void Interface::OnSKSEMessage(SKSEMessagingInterface::Message *message)
	{
		std::string message_data = std::string((const char *)message->data, message->dataLen);

		switch (message->type)
		{
			case SKSEMessagingInterface::kMessage_SaveGame:
			{
				SaveGameManager::GetInstance()->OnSaveGame(std::move(message_data));
			} break;

			case SKSEMessagingInterface::kMessage_PostLoadGame:
			{
				SaveGameManager::GetInstance()->OnLoadGame(std::move(message_data));
			} break;

			case SKSEMessagingInterface::kMessage_DeleteGame:
			{
				SaveGameManager::GetInstance()->OnDeleteGame(std::move(message_data));
			} break;
		}
	}
}