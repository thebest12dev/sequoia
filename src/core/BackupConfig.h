#pragma once
#include "SequoiaExport.h"
#include "World.h"
namespace sequoia {
  enum class CompressionFormat : uint8_t {
    ZLIB_COMPRESSED = 1,
    RAW = 2,
    ZIP7_COMPRESSED = 3,
    ZIP_COMPRESSED = 4,
    GZIP_COMPRESSED = 5
  };
  struct SEQUOIA_EXPORT BackupConfig {
    std::string destinationFolder = "";
    bool versioned = true;
    bool retention = true;
    int retenionLifetime = 30;
    std::string backupNameFormat = "${datetime}_${worldName}";
    CompressionFormat backupFormat = CompressionFormat::RAW;
    int compressionRatio = 4;
  };
}
