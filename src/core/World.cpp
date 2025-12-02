#include "World.h"
#include "Backup.h"
namespace sequoia {
  void World::initialize() {
    
    
    
    nbt::tag_compound tag{};
    
    
    nbt::tag_compound data{};
    data["backups"] = nbt::tag_list{ nbt::tag_type::Compound };
    data["version"] = 1;
    tag["data"] = nbt::tag_compound{data};
    std::ofstream create(worldPath + "/sequoia.dat", std::ios::binary);
    zlib::ozlibstream sequoiaZlibStream{ create };
    nbt::io::write_tag("root", tag, sequoiaZlibStream);
    return;
  }
  World::World(std::string worldPath) : worldPath(worldPath) {
    std::ifstream sequoiaNbtStream{ worldPath + "/sequoia.dat", std::ios::binary };

   if (!sequoiaNbtStream) {
      // initialize sequoia.dat
      sequoiaNbtStream.close();
      initialize();
      sequoiaNbtStream = std::ifstream{ worldPath + "/sequoia.dat", std::ios::binary };
    }

    zlib::izlibstream sequoiaZlibStream{sequoiaNbtStream};
    auto [_, seqNbtPtr] = nbt::io::read_tag(sequoiaZlibStream);
    
    sequoiaNbt = nbt::value{ std::move(seqNbtPtr) };

    // level.dat
    std::ifstream levelNbtStream{ worldPath+"/level.dat", std::ios::binary};
    zlib::izlibstream levelZlibStream{ levelNbtStream };

    
    auto [__, levelNbtPtr] = nbt::io::read_tag(levelZlibStream);

    levelNbt = nbt::value{ std::move(levelNbtPtr) };
    worldName = static_cast<std::string>(levelNbt.at("Data").at("LevelName"));
  }
  World::~World() {

  }
  std::vector<Backup> World::getBackups() {

    nbt::tag_list& backups = static_cast<nbt::tag_list&>(sequoiaNbt.at("data").at("backups").as<nbt::tag_list>());
    std::vector<Backup> backupsList;
    for (nbt::value& tag : backups) {
      nbt::tag_compound c = tag.as<nbt::tag_compound>();
     
      Backup bc(*this, BackupConfig{});
      backupsList.push_back(bc);

    }
    return backupsList;

  }
}