#pragma once
#include <string>
#include "nbt_tags.h"
#include "io/stream_reader.h"
#include "io/stream_writer.h"
#include "io/izlibstream.h"
#include "io/ozlibstream.h"
#include "SequoiaExport.h"
#include <fstream>
#include <optional>
#include <functional>

namespace sequoia {
  class Backup;
  class BackupConfig;
  class SEQUOIA_EXPORT World {
  protected:
    nbt::value sequoiaNbt;
    
    std::string worldName;
    std::string worldPath;
    nbt::value levelNbt;
  public:
    World(std::string worldPath);
    std::filesystem::path getWorldPath();
    std::string getWorldName();
    std::optional<BackupConfig> getBackupConfig();
    void setBackupConfig(BackupConfig& config);
    void initialize();
    std::vector<Backup> getBackups();
    ~World();

    friend class Backup;
  };
}