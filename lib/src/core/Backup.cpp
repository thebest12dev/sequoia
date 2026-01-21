#pragma once
#include "SequoiaExport.h"
#include "Backup.h"
#include "ZipCompressor.h"
#include "World.h"
#include <unordered_set>
#include <unordered_map>
namespace {
  enum class BinaryTag : uint8_t {
    LONG = 1,
    INTEGER = 2,
    SHORT = 3,
    BYTE = 4,
    ARRAY = 5,
    STRING = 6,
    BINARY = 7,
    LIST = 8,
    MAP = 9,
    FLOAT = 10,
    DOUBLE = 11,
  };
  struct Serialized {
    BinaryTag type;
    int length;
    std::vector<uint8_t> payload;
  };
  Serialized serializeString(std::string str) {
    Serialized ser = {};
    ser.type = BinaryTag::STRING;
    ser.length = str.length();
    std::vector<uint8_t> payload;
    for (char c : str) {
      payload.push_back(c);
    }
    ser.payload = payload;
    return ser;
  }
  Serialized serializeByte(uint8_t v) {
    Serialized ser = {};
    ser.type = BinaryTag::BYTE;
    ser.length = -1;
    ser.payload = std::vector<uint8_t>{ v };
    return ser;
  }

  Serialized serializeShort(uint16_t v) {
    Serialized ser = {};
    ser.type = BinaryTag::SHORT;
    ser.length = -1;
    ser.payload = std::vector<uint8_t>{
      static_cast<uint8_t>(v & 0xFF),
      static_cast<uint8_t>((v >> 8) & 0xFF)
    };
    return ser;
  }
  Serialized serializeInt(uint32_t v) {
    Serialized ser = {};
    ser.type = BinaryTag::INTEGER;
    ser.length = -1;
    uint8_t b0 = static_cast<uint8_t>(v & 0xFF);
    uint8_t b1 = static_cast<uint8_t>((v >> 8) & 0xFF);
    uint8_t b2 = static_cast<uint8_t>((v >> 16) & 0xFF);
    uint8_t b3 = static_cast<uint8_t>((v >> 24) & 0xFF);
    ser.payload = std::vector<uint8_t>{
      b0, b1, b2, b3
    };

    return ser;
  }
  Serialized serializeLong(uint64_t v) {
    Serialized ser = {};
    ser.type = BinaryTag::LONG;
    ser.length = -1;

    uint8_t b0 = static_cast<uint8_t>(v & 0xFF);
    uint8_t b1 = static_cast<uint8_t>((v >> 8) & 0xFF);
    uint8_t b2 = static_cast<uint8_t>((v >> 16) & 0xFF);
    uint8_t b3 = static_cast<uint8_t>((v >> 24) & 0xFF);
    uint8_t b4 = static_cast<uint8_t>((v >> 32) & 0xFF);
    uint8_t b5 = static_cast<uint8_t>((v >> 40) & 0xFF);
    uint8_t b6 = static_cast<uint8_t>((v >> 48) & 0xFF);
    uint8_t b7 = static_cast<uint8_t>((v >> 56) & 0xFF);
    ser.payload = std::vector<uint8_t>{
      b0, b1, b2, b3, b4, b5, b6, b7
    };

    return ser;
  }
  std::vector<uint8_t> serializeTag(Serialized v) {
    std::vector<uint8_t> raw;
    int dataSize = v.length;
    bool predeterminedValue = false;
    switch (v.type) {
    case BinaryTag::LONG:
      dataSize = 8;
      predeterminedValue = true;
      break;
    case BinaryTag::INTEGER:
      dataSize = 4;
      predeterminedValue = true;
      break;
    case BinaryTag::SHORT:
      dataSize = 2;
      predeterminedValue = true;
      break;
    case BinaryTag::BYTE:
      dataSize = 1;
      predeterminedValue = true;
      break;
    }
    raw.reserve(dataSize + 5);

    //serialization process
    raw.push_back(static_cast<uint8_t>(v.type));
    if (!predeterminedValue) {
      uint8_t b0 = static_cast<uint8_t>(v.length & 0xFF);
      uint8_t b1 = static_cast<uint8_t>((v.length >> 8) & 0xFF);
      uint8_t b2 = static_cast<uint8_t>((v.length >> 16) & 0xFF);
      uint8_t b3 = static_cast<uint8_t>((v.length >> 24) & 0xFF);
      raw.push_back(b0);
      raw.push_back(b1);
      raw.push_back(b2);
      raw.push_back(b3);
    }
    for (size_t i = 0; i < dataSize; i++) {
      raw.push_back(v.payload[i]);
    }
    raw.push_back(0);
    return raw;

  }
  Serialized serializeMap(std::unordered_map<std::string, Serialized> map) {
    Serialized ser = {};
    ser.type = BinaryTag::MAP;
    std::vector<uint8_t> payload;
    

    for (auto [key, value] : map) {
      auto a = serializeTag(value);
      
      payload.push_back(static_cast<uint8_t>(static_cast<uint8_t>(key.length()) & 0xFF));
      payload.push_back(static_cast<uint8_t>((static_cast<uint8_t>(key.length()) >> 8) & 0xFF));

      for (char c : key) {
        payload.push_back(c);
      }
      payload.insert(payload.end(), a.begin(), a.end());
    }
    ser.length = payload.size();
    ser.payload = payload;
    return ser;
  }
  
  bool copyDirectory(const fs::path& source, const fs::path& destination) {

    if (!fs::exists(source) || !fs::is_directory(source)) {
      throw std::runtime_error("Source directory does not exist");
      return false;
    }


    fs::create_directories(destination);

    for (const auto& entry : fs::recursive_directory_iterator(source)) {
      const auto& path = entry.path();
      auto relative_path = fs::relative(path, source);
      auto target = destination / relative_path;

      if (entry.is_directory()) {
        fs::create_directories(target);
      }
      else if (entry.is_regular_file()) {
        fs::copy_file(
          path,
          target,
          fs::copy_options::overwrite_existing
        );
      }
      else if (entry.is_symlink()) {
        fs::copy(path, target, fs::copy_options::overwrite_existing);
      }
    }
    return true;
  }
  enum class ChangeType {
    CREATED,
    MODIFIED,
    DELETED
  };

  struct FileInfo {
    uintmax_t size;
    fs::file_time_type lastWrite;
  };

  struct DiffResult {
    std::vector<std::string> created;
    std::vector<std::string> modified;
    std::vector<std::string> deleted;

    bool empty() const {
      return created.empty() && modified.empty() && deleted.empty();
    }

    size_t totalChanges() const {
      return created.size() + modified.size() + deleted.size();
    }
  };

  static constexpr uintmax_t DIR_SENTINEL_SIZE = 0;

  inline bool isDirectorySentinel(const FileInfo& info) {
    return info.lastWrite == fs::file_time_type::min();
  }

  std::unordered_map<std::string, FileInfo>
    indexDirectory(const fs::path& root) {
    std::unordered_map<std::string, FileInfo> result;
    std::unordered_set<std::string> dirsWithFiles;

    for (const auto& entry : fs::recursive_directory_iterator(root)) {
      auto relative = fs::relative(entry.path(), root).string();

      if (entry.is_regular_file()) {
        result.emplace(relative, FileInfo{
          entry.file_size(),
          entry.last_write_time()
          });


        auto parent = fs::path(relative).parent_path();
        while (!parent.empty()) {
          dirsWithFiles.insert(parent.string() + "/");
          parent = parent.parent_path();
        }
      }
      else if (entry.is_directory()) {

        result.emplace(relative + "/", FileInfo{
          DIR_SENTINEL_SIZE,
          fs::file_time_type::min()
          });
      }
    }


    for (const auto& dir : dirsWithFiles) {
      result.erase(dir);
    }

    return result;
  }

  DiffResult compareDirectories(
    const std::unordered_map<std::string, FileInfo>& oldDir,
    const std::unordered_map<std::string, FileInfo>& newDir
  ) {
    DiffResult diff;

    for (const auto& [path, newInfo] : newDir) {
      auto it = oldDir.find(path);

      if (it == oldDir.end()) {
        diff.created.push_back(path);
        continue;
      }

      const auto& oldInfo = it->second;


      if (isDirectorySentinel(newInfo) || isDirectorySentinel(oldInfo))
        continue;

      if (oldInfo.size != newInfo.size ||
        oldInfo.lastWrite != newInfo.lastWrite) {
        diff.modified.push_back(path);
      }
    }

    for (const auto& [path, _] : oldDir) {
      if (!newDir.contains(path)) {
        diff.deleted.push_back(path);
      }
    }

    return diff;
  }



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

//random lib
#include <random>
#include <iomanip>

void print_hex(const std::vector<uint8_t>& data) {
  for (uint8_t b : data) {
    std::cout
      << std::hex << std::setw(2) << std::setfill('0')
      << static_cast<int>(b) << ' ';
  }
  std::cout << std::dec << '\n'; // restore decimal
}
namespace sequoia {
  Backup::Backup(World& world, BackupConfig conf) : world(world), conf(conf) {};
  World& Backup::getWorld() {
    return world;
  };
  bool Backup::backup() {
    auto root = std::unordered_map<std::string, Serialized>{};
    auto elements1 = std::unordered_map<std::string, Serialized>{};
    auto elements2 = std::unordered_map<std::string, Serialized>{};
    auto elements3 = std::unordered_map<std::string, Serialized>{};
    elements1["level.dat"] = serializeShort(1);
    elements1["data/raids.dat"] = serializeShort(2);
    elements2["test.dat"] = serializeShort(3);
    elements3["icon.png"] = serializeShort(3);

    root["created"] = serializeMap(elements1);
    root["modified"] = serializeMap(elements2);
    root["deleted"] = serializeMap(elements3);
    auto c = serializeMap(root);

    print_hex(serializeTag(c));
    return 1;
    std::string overridePath = "";
    bool deleteOnTemp = false;
    if (conf.incrementalBackups) {
      nbt::tag& backupsTag = static_cast<nbt::tag&>(world.sequoiaNbt.at("data").at("backups"));
      nbt::tag_list& backupsList = static_cast<nbt::tag_list&>(backupsTag);
      if (backupsList.size() == 0) {
        conf.backupFormat = CompressionFormat::RAW;
        conf.backupNameFormat = "incrementalBase";

      }
      else {
        auto& backupTag = *(backupsList.begin());

        nbt::tag_compound backupTagC = backupTag.as<nbt::tag_compound>();
        std::string relPath = static_cast<std::string>(backupTagC["relativePath"]);
        if (static_cast<int8_t>(backupTagC.at("incrementalBase")) == false) {
          // base backup
          conf.backupFormat = CompressionFormat::RAW;
          conf.backupNameFormat = "incrementalBase";
        }
        else {
          // diff check
          auto rp = world.getBackupConfig()->destinationFolder.string() + relPath;
          auto a = indexDirectory(rp);
          auto b = indexDirectory(world.getWorldPath());
          DiffResult diffs = compareDirectories(a, b);

          // create temporary snapshot

          // temp dirname 1 line
          fs::path s = world.getBackupConfig()->destinationFolder.string() + "/incremental_" + [] { static const char h[] = "0123456789abcdef"; std::mt19937 r(std::random_device{}()); std::string o(8, '0'); for (char& c : o) c = h[r() % 16]; return o; }();

          fs::create_directories(s);
          for (std::string file : diffs.created) {
            std::string worldPath = world.getWorldPath().string();
            fs::path b = worldPath + "/" + file;
            std::filesystem::create_directories((s / file).parent_path());

            fs::copy(b, s / file);
          }
          for (std::string file : diffs.modified) {
            std::string worldPath = world.getWorldPath().string();
            fs::path b = worldPath + "/" + file;
            std::filesystem::create_directories((s / file).parent_path());

            fs::copy(b, s / file);
          }
          overridePath = s.string();
          deleteOnTemp = true;

        }
      }
    }
    if (conf.backupFormat == CompressionFormat::RAW) {
      auto replaceAll = [](std::string& str, const std::string& oldStr, const std::string& newStr) {
        size_t pos = 0;
        while ((pos = str.find(oldStr, pos)) != std::string::npos) {
          str.replace(pos, oldStr.length(), newStr);;
          pos += newStr.length();
        }
        };
      std::filesystem::path path = overridePath.empty() ? world.getWorldPath() : overridePath;
      std::string parsed = conf.backupNameFormat;

      replaceAll(parsed, "${worldName}", world.worldName);
      replaceAll(parsed, "${dateTime}", isoTime());

      std::filesystem::path relPath = std::string("/") + parsed;
      std::filesystem::path dest = conf.destinationFolder.string() + relPath.string();



      if (copyDirectory(path, dest)) {
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
        data["incrementalBase"] = static_cast<int8_t>(parsed == "incrementalBase" ? true : false);
        nbt::value_initializer val{ static_cast<nbt::tag&&>(d) };
        backupsList.push_back(static_cast<nbt::value_initializer&&>(val));
        std::ofstream create(world.worldPath + "/sequoia.dat", std::ios::binary);

        nbt::io::write_tag("root", world.sequoiaNbt, create);
        if (deleteOnTemp) {
          // scary
          fs::remove_all(overridePath);
        }
        return true;
      }
      if (deleteOnTemp) {
        // scary
        fs::remove_all(overridePath);
      }
      return false;
    }
    if (conf.backupFormat == CompressionFormat::ZIP_COMPRESSED) {
      auto replaceAll = [](std::string& str, const std::string& oldStr, const std::string& newStr) {
        size_t pos = 0;
        while ((pos = str.find(oldStr, pos)) != std::string::npos) {
          str.replace(pos, oldStr.length(), newStr);;
          pos += newStr.length();
        }
        };
      std::filesystem::path path = overridePath.empty() ? world.getWorldPath() : overridePath;
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
        data["incrementalBase"] = static_cast<int8_t>(false);
        nbt::value_initializer val{ static_cast<nbt::tag&&>(d) };
        backupsList.push_back(static_cast<nbt::value_initializer&&>(val));
        std::ofstream create(world.worldPath + "/sequoia.dat", std::ios::binary);

        nbt::io::write_tag("root", world.sequoiaNbt, create);
        if (deleteOnTemp) {
          // scary
          fs::remove_all(overridePath);
        }
        return true;
      }
      if (deleteOnTemp) {
        // scary
        fs::remove_all(overridePath);
      }
      return false;
    }


  }
}