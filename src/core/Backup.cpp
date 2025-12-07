#pragma once
#include "SequoiaExport.h"
#include "Backup.h"
#include "ZipCompressor.h"
#include "World.h"
namespace {
  std::string isoTime() {

    auto now = std::chrono::system_clock::now();
    auto timePoint = std::chrono::system_clock::to_time_t(now);


    std::tm tm;
    localtime_s(&tm, &timePoint);  


    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H-%M-%SZ");

    return ss.str();
  }
}
namespace sequoia {
  Backup::Backup(World& world, BackupConfig conf) : world(world), conf(conf) {};
  World& Backup::getWorld() {
    return world;
  };
  bool Backup::backup() {
    if (conf.backupFormat == CompressionFormat::ZIP_COMPRESSED) {
      auto replaceAll = [](std::string& str, const std::string& oldStr, const std::string& newStr) {
        size_t pos = 0;
        while ((pos = str.find(oldStr, pos)) != std::string::npos) {
          str.replace(pos, oldStr.length(), newStr);;
          pos += newStr.length();
        }
        };
      std::filesystem::path path = world.getWorldPath();
      std::string parsed = conf.backupNameFormat;
      
      replaceAll(parsed, "${worldName}", world.worldName);
      replaceAll(parsed, "${dateTime}", isoTime());

      std::filesystem::path relPath = std::string("/") + parsed + ".zip";
      std::filesystem::path dest = conf.destinationFolder.string() + relPath.string();
      

      sequoia::ZipCompressor cp(path, dest);
      if (cp.compress(true)) {
        nbt::tag& backupsTag = static_cast<nbt::tag&>(world.sequoiaNbt.at("data").at("backups"));
        nbt::tag_list& backupsList = static_cast<nbt::tag_list&>(backupsTag);
        nbt::tag_compound d{};
        nbt::tag_compound& data = d;

        using namespace std::chrono;

        auto now = system_clock::now();


        auto unixTime = duration_cast<seconds>(now.time_since_epoch()).count();
        
        data["backupTime"] = time(nullptr);
        data["compressionFormat"] = static_cast<uint8_t>(conf.backupFormat);
        data["relativePath"] = relPath.string();
        nbt::value_initializer val{ static_cast<nbt::tag&&>(d)};
        backupsList.push_back(static_cast<nbt::value_initializer&&>(val));
        std::ofstream create(world.worldPath + "/sequoia.dat", std::ios::binary);

        nbt::io::write_tag("root", world.sequoiaNbt, create);
        return true;
      }
      return false;
    }
  }
}