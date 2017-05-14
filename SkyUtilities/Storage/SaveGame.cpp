#include "Storage/SaveGame.h"

#include "Plugin.h"

using namespace boost::filesystem;

namespace SKU::Storage {

	SaveGame::SaveGame()
	{

	}

	SaveGame::~SaveGame()
	{

	}

	SaveGame::Ptr SaveGame::Load(const boost::filesystem::path &path)
	{
		return nullptr;
	}

	SaveGame::Ptr SaveGame::Create()
	{
		return nullptr;
	}

	void SaveGame::Save(const boost::filesystem::path &path)
	{
		
	}

}