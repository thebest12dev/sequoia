#pragma once
#include "SequoiaExport.h"
#include "Backup.h"

namespace sequoia {
  Backup::Backup(World world, BackupConfig conf) : world(world), conf(conf) {};
}