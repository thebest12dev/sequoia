#pragma once
#include <string>
#include "nbt_tags.h"
#include "io/stream_reader.h"
#include "io/stream_writer.h"
#include "io/izlibstream.h"
#include "io/ozlibstream.h"
#include "SequoiaExport.h"
#include <fstream>

namespace sequoia {
  class Backup;
  class SEQUOIA_EXPORT World {
  protected:
    nbt::value sequoiaNbt;
    
    std::string worldName;
    std::string worldPath;
    nbt::value levelNbt;
  public:
    World(std::string worldPath);
    void initialize();
    std::vector<Backup> getBackups();
    ~World();
  };
}