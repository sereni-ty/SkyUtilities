#pragma once

#include "Singleton.h"
#include "Events.h"

#include "Storage/SaveGame.h"

#include <boost/filesystem.hpp>

#include <unordered_set>
#include <string>

namespace SKU::Storage {

	class Interface : public Singleton<Interface>, public IEventHandler
	{
		IS_SINGLETON_CLASS(Interface)

		public:
			void OnSKSEMessage(SKSEMessagingInterface::Message*);

		public:	
			void OnSaveGame(const std::string &savegame_name);
			void OnLoadGame(const std::string &savegame_name);
			void OnNewGame();
			void OnDeleteGame(const std::string &savegame_name);

		private:
			boost::filesystem::path savegame_folder;
			std::unordered_set<std::string> savegame_set;
			SaveGame::Ptr current_savegame;
	};
}
