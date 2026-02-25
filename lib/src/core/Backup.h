#pragma once
#include "SequoiaExport.h"
#include "World.h"
#include "BackupConfig.h"
#include <stdint.h>
namespace sequoia {
  
  class SEQUOIA_EXPORT Backup {
  protected:
    World& world;
    uintmax_t backupTime;
    std::string backupName;
    BackupConfig conf;
  public:
    /**
     * @brief Constructs a backup object.
     * 
     * @param world The world that when committed, will apply to.
     */
    Backup(World& world, BackupConfig conf);

    /**
     * @brief Gets the world that is associated with this backup.
     * 
     * @return The world.
     */
    World& getWorld();
    std::string getRelativeLocation();
    uintmax_t getBackupTime();
    /**
     * 
     * 
     */
    bool backup();
    bool isPresentOnDisk();
    bool restore();

    friend class World;
  };
}
