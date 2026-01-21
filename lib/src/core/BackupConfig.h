#pragma once
#include "SequoiaExport.h"
#include "World.h"
#include <filesystem>
namespace sequoia {
  enum class CompressionFormat : uint8_t {
    ZLIB_COMPRESSED = 1,
    RAW = 2,
    ZIP7_COMPRESSED = 3,
    ZIP_COMPRESSED = 4,
    GZIP_COMPRESSED = 5
  };
  struct SEQUOIA_EXPORT BackupConfig {
    std::filesystem::path destinationFolder = "";
    bool versioned = true;
    bool retention = true;
    int retenionLifetime = 30;
    std::string backupNameFormat = "${dateTime}_${worldName}";
    CompressionFormat backupFormat = CompressionFormat::RAW;
    bool incrementalBackups = false;

    int compressionRatio = 4;
  };
}
