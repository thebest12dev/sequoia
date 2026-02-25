#include "World.h"
#include <filesystem>
#include "Backup.h"
namespace sequoia {
  void World::initialize() {
    
    
    
    nbt::tag_compound tag{};
    
    
    nbt::tag_compound data{};
    data["backups"] = nbt::tag_list{ nbt::tag_type::Compound };
    data["version"] = 1;
    tag["data"] = nbt::tag_compound{data};
    std::ofstream create(worldPath + "/sequoia.dat", std::ios::binary);
    //zlib::ozlibstream sequoiaZlibStream{ create };
    nbt::io::write_tag("root", tag, create);
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

    //zlib::izlibstream sequoiaZlibStream{sequoiaNbtStream};
    auto [_, seqNbtPtr] = nbt::io::read_tag(sequoiaNbtStream);
    
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
  std::filesystem::path World::getWorldPath() {
    return std::filesystem::path(worldPath);
  }
  std::string World::getWorldName() {
    return worldName;
  }
  std::optional<BackupConfig> World::getBackupConfig() {
    try {
      nbt::tag_compound tag{ sequoiaNbt.at("data").at("backupConfig").as<nbt::tag_compound>()};
      BackupConfig bcf = {};
      bcf.backupFormat = static_cast<CompressionFormat>(static_cast<int8_t>(tag.at("backupFormat")));
      bcf.backupNameFormat = static_cast<std::string>(tag.at("backupNameFormat"));
      bcf.destinationFolder = static_cast<std::string>(tag.at("destinationFolder"));
      return bcf;
    }
    catch (std::exception) {
      return std::nullopt;
    }
  }
  void World::setBackupConfig(BackupConfig& config) {
    nbt::tag_compound tag{};
    tag["backupFormat"] = static_cast<int8_t>(config.backupFormat);
    tag["backupNameFormat"] = config.backupNameFormat.c_str();
    tag["destinationFolder"] = config.destinationFolder.string().c_str();

    sequoiaNbt["data"]["backupConfig"] = static_cast<nbt::tag&&>(static_cast<nbt::tag_compound&>(tag));

    std::ofstream create(worldPath + "/sequoia.dat", std::ios::binary);
    nbt::io::write_tag("root", sequoiaNbt, create);
  }
  std::vector<Backup> World::getBackups() {

    nbt::tag_list& backups = static_cast<nbt::tag_list&>(sequoiaNbt.at("data").at("backups").as<nbt::tag_list>());
    std::vector<Backup> backupsList;
    for (nbt::value& tag : backups) {
      nbt::tag_compound c = tag.as<nbt::tag_compound>();
      
      Backup bc(*this, getBackupConfig().has_value() ? getBackupConfig().value() : BackupConfig{});
      bc.backupName = c["relativePath"].as<nbt::tag_string>().get();
      bc.backupTime = c["backupTime"].as<nbt::tag_long>();
      
      backupsList.push_back(bc);

    }
    return backupsList;

  }
}