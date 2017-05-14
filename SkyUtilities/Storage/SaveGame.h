#pragma once

#include <boost/filesystem.hpp>

#include <memory>

namespace SKU::Storage {

	class SaveGame	// Conceptional
	{				// TODO: Most likely will end up as a base. Savegame will end up as an implementation of an interface for the appropiate savegame version found
		public:
			using Ptr = std::unique_ptr<SaveGame>;

		private:
			enum Version
			{
				v1 = 0x0001,
			};

			const Version latest_verion = v1;

		private:
			SaveGame();
			SaveGame(const SaveGame&) = delete;

		public:
			~SaveGame();

		public:
			static Ptr Load(const boost::filesystem::path &path);
			static Ptr Create();

		public:
			void Save(const boost::filesystem::path &path);

		private:
			boost::filesystem::path path;
	};
}
