#pragma once
#include "SequoiaExport.h"
#include "Backup.h"
#include "ZipCompressor.h"
#include "World.h"
#include <unordered_set>
#include <unordered_map>
namespace {
  
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

    std::unordered_map<std::string, std::pair<uintmax_t, uintmax_t>> created;
    std::unordered_map<std::string, std::pair<uintmax_t, uintmax_t>> modified;
    std::vector<std::string> deleted;

    bool empty() const {
      return created.empty() && modified.empty() && deleted.empty();
    }

    size_t totalChanges() const {
      return created.size() + modified.size() + deleted.size();
    }
  };

  static constexpr uintmax_t DIR_SENTINEL_SIZE = 0;
  static fs::file_time_type toFileTime(std::time_t t)
  {
    using namespace std::chrono;

    auto sysTime = system_clock::from_time_t(t);
    auto fileTime = fs::file_time_type::clock::now()
      + (sysTime - system_clock::now());

    return fileTime;
  }
  inline bool isDirectorySentinel(const FileInfo& info) {
    return info.lastWrite == fs::file_time_type::min();
  }
  std::unordered_map<std::string, FileInfo>
    indexDirectory(const fs::path& root)
  {
    std::unordered_map<std::string, FileInfo> result;

    for (const auto& entry : fs::recursive_directory_iterator(root))
    {
      auto relative = fs::relative(entry.path(), root).generic_string();

      if (relative.empty())
        continue;

      if (entry.is_regular_file())
      {
        result.emplace(relative, FileInfo{
            entry.file_size(),
            entry.last_write_time()
          });
      }
      else if (entry.is_directory())
      {
        result.emplace(relative, FileInfo{
            DIR_SENTINEL_SIZE,
            toFileTime(0)
          });
      }
    }

    return result;
  }
  //std::unordered_map<std::string, FileInfo>
  //  indexDirectory(const fs::path& root) {
  //  std::unordered_map<std::string, FileInfo> result;
  //  std::unordered_set<std::string> dirsWithFiles;

  //  for (const auto& entry : fs::recursive_directory_iterator(root)) {
  //    auto relative = fs::relative(entry.path(), root).string();

  //    if (entry.is_regular_file()) {
  //      result.emplace(relative, FileInfo{
  //        entry.file_size(),
  //        entry.last_write_time()
  //        });


  //      auto parent = fs::path(relative).parent_path();
  //      while (!parent.empty()) {
  //        dirsWithFiles.insert(parent.string() + "/");
  //        parent = parent.parent_path();
  //      }
  //    }
  //    else if (entry.is_directory()) {

  //      result.emplace(relative + "/", FileInfo{
  //        DIR_SENTINEL_SIZE,
  //        fs::file_time_type::min()
  //        });
  //    }
  //  }


  //  for (const auto& dir : dirsWithFiles) {
  //    result.erase(dir);
  //  }

  //  return result;
  //}
  
  uintmax_t unixWriteTime(const std::filesystem::directory_entry& entry)
  {
    using namespace std::chrono;

    auto ftime = entry.last_write_time();
    auto sctp = clock_cast<system_clock>(ftime);

    auto seconds = duration_cast<std::chrono::seconds>(
      sctp.time_since_epoch()
    ).count();

    return static_cast<uintmax_t>(seconds);
  }
  std::unordered_map<std::string, FileInfo>
    indexZip(const fs::path& root)
  {
    std::unordered_map<std::string, FileInfo> result;

    int err = 0;
    zip_t* archive = zip_open(root.string().c_str(), ZIP_RDONLY, &err);
    if (!archive) {
      throw std::runtime_error("failed to open zip archive!");
    }

    zip_int64_t count = zip_get_num_entries(archive, 0);

    for (zip_uint64_t i = 0; i < static_cast<zip_uint64_t>(count); ++i) {
      zip_stat_t sb;
      if (zip_stat_index(archive, i, 0, &sb) == 0) {

        std::string name = sb.name ? sb.name : "";

        FileInfo info{};
        info.size = sb.size;

        if (sb.valid & ZIP_STAT_MTIME) {
          info.lastWrite = toFileTime(sb.mtime);
        }
        else {
          info.lastWrite = fs::file_time_type::min();
        }

        result.emplace(std::move(name), std::move(info));
      }
    }

    zip_close(archive);
    return result;
  }
  uintmax_t convertToUnix(std::filesystem::file_time_type ftime) {
    auto stpoint = std::chrono::clock_cast<std::chrono::system_clock>(ftime);
    std::time_t t = std::chrono::system_clock::to_time_t(stpoint);
    uintmax_t time = static_cast<uintmax_t>(t);

    return time;
  }
  bool timeEqual(fs::file_time_type a,
    fs::file_time_type b)
  {
    using namespace std::chrono;

    auto diff = a > b ? a - b : b - a;

    return diff <= 1000ms; 
  }
  DiffResult compareDirectories(
    const std::unordered_map<std::string, FileInfo>& oldDir,
    const std::unordered_map<std::string, FileInfo>& newDir
  ) {
    DiffResult diff;

    for (const auto& [path, newInfo] : newDir) {
      auto it = oldDir.find(path);

      if (it == oldDir.end()) {
        diff.created.emplace(path, std::pair{ convertToUnix(newInfo.lastWrite), newInfo.size });
        continue;
      }

      const auto& oldInfo = it->second;


      if (isDirectorySentinel(newInfo) || isDirectorySentinel(oldInfo))
        continue;

      if (oldInfo.size != newInfo.size ||
        !timeEqual(oldInfo.lastWrite, newInfo.lastWrite)) {
        diff.modified.emplace(path, std::pair{ convertToUnix(newInfo.lastWrite), newInfo.size });
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
  using LocatorMap = std::unordered_map<std::string, std::pair<std::string, std::pair<uintmax_t, uintmax_t>>>;
  struct FileLocator {
  private:

    LocatorMap netFiles;
    std::vector<DiffResult*> diffResults;

  public:
    void apply(DiffResult& result, std::string id);
    void set(LocatorMap netFiles);
    void loadNBT(std::filesystem::path path);
    void saveNBT(std::filesystem::path path);
    LocatorMap get();
  };
  void FileLocator::apply(DiffResult& result, std::string id) {
    diffResults.push_back(&result);
 
    for (auto& [i, val] : result.created) {
      netFiles[i] = { id, {val.first, val.second} };
    }
    for (auto& [i, val]: result.modified) {
      netFiles[i] = { id, {val.first, val.second} };
    }
    for (std::string i : result.deleted) {
      netFiles.erase(i);
    }
    
  }
  void FileLocator::set(LocatorMap netFiles) {
    FileLocator::netFiles = netFiles;
  }
  void FileLocator::saveNBT(std::filesystem::path path) {
    nbt::tag_compound locator{};
    for (auto& [key, val] : netFiles) {
      nbt::tag_compound item_{};
      nbt::tag_compound& item = item_;
      item["id"] = static_cast<std::string>(val.first);
      item["time"] = static_cast<int64_t>(val.second.first);
      item["size"] = static_cast<int64_t>(val.second.second);
      locator[key] = static_cast<nbt::tag&&>(item);

    }
    std::ofstream create(path, std::ios::binary);

    nbt::io::write_tag("root", locator, create);
  }
  void FileLocator::loadNBT(std::filesystem::path path) {
    
    std::ifstream locatorNbtStream{ path, std::ios::binary };
    auto [_, locNbtPtr] = nbt::io::read_tag(locatorNbtStream);

    auto locatorNbtVal = nbt::value{ std::move(locNbtPtr) };
    nbt::tag_compound& locatorNbt = static_cast<nbt::tag_compound&>(locatorNbtVal.get());
    netFiles.clear();
    for (auto& [key, value] : locatorNbt) {
      nbt::tag_compound item = value.get().as<nbt::tag_compound>();
      std::pair<std::string, std::pair<uintmax_t, uintmax_t>> pair = { item["id"].as<nbt::tag_string>().get(), std::pair{item["time"].as<nbt::tag_long>().get(),item["size"].as<nbt::tag_long>().get()} };

      netFiles.emplace(key, pair);

    }
  }
  LocatorMap FileLocator::get() {
    return netFiles;
  }

  
}

//random lib
#include <random>
#include <iomanip>


namespace sequoia {
  Backup::Backup(World& world, BackupConfig conf) : world(world), conf(conf) {};
  World& Backup::getWorld() {
    return world;
  };
  bool Backup::backup() {
    std::string overridePath = "";
    bool deleteOnTemp = false;
    DiffResult diffs;
    if (conf.incrementalBackups) {
      nbt::tag& backupsTag = static_cast<nbt::tag&>(world.sequoiaNbt.at("data").at("backups"));
      nbt::tag_list& backupsList = static_cast<nbt::tag_list&>(backupsTag);
      
      if (backupsList.size() == 0) {
        conf.backupFormat = CompressionFormat::RAW;
        conf.backupNameFormat = "incrementalBase";
      }
      else {
        auto& backupTag = *(backupsList.end() - 1);
        nbt::tag_compound backupTagC = backupTag.as<nbt::tag_compound>();
        std::string relPath = static_cast<std::string>(backupTagC["relativePath"]);
        //if (static_cast<int8_t>(backupTagC.at("incrementalBase")) == false && backupsList) {
        //  // base backup
        //  conf.backupFormat = CompressionFormat::RAW;
        //  conf.backupNameFormat = "incrementalBase";
        //}
        //else {
          // diff check
          auto rp = world.getBackupConfig()->destinationFolder.string() + relPath;
          FileLocator fl;
          fl.loadNBT(conf.destinationFolder.string() + "/incrementalLocator.dat");
          auto map = fl.get();
          auto indexFileLocator = [](LocatorMap fileLocator) {
            std::unordered_map<std::string, FileInfo> index;
            for (auto& [key, val] : fileLocator) {
              index.emplace(key, FileInfo{ .size = val.second.second, .lastWrite = toFileTime(val.second.first) });
            }
            return index;
          };
          //auto a = relPath == "/incrementalBase" ? indexDirectory(rp) : indexZip(rp);
          auto a = indexFileLocator(fl.get());
          auto b = indexDirectory(world.getWorldPath());
          
          diffs = compareDirectories(a,b);

          



          // create temporary snapshot

          // temp dirname 1 line
          fs::path s = world.getBackupConfig()->destinationFolder.string() + "/incremental_" + [] { static const char h[] = "0123456789abcdef"; std::mt19937 r(std::random_device{}()); std::string o(8, '0'); for (char& c : o) c = h[r() % 16]; return o; }();

          fs::create_directories(s);
          for (auto [file, _] : diffs.created) {
            std::string worldPath = world.getWorldPath().string();
            fs::path b = worldPath + "/" + file;
            std::filesystem::create_directories((s / file).parent_path());

            fs::copy(b, s / file);
          }
          for (auto [file, _] : diffs.modified) {
            std::string worldPath = world.getWorldPath().string();
            fs::path b = worldPath + "/" + file;
            std::filesystem::create_directories((s / file).parent_path());

            fs::copy(b, s / file);
          }
          overridePath = s.string();
          deleteOnTemp = true;

        //}
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
        LocatorMap map;
        FileLocator fl;
        for (auto b : fs::recursive_directory_iterator(dest)) {
           auto path = fs::relative(b.path(), dest);
          
           map.emplace(path.generic_string(), std::pair{relPath.string(), std::pair{b.is_regular_file() ? unixWriteTime(b) : 0, b.file_size()}});
           
        }
        fl.set(map);
        fl.saveNBT(conf.destinationFolder.string() + "/incrementalLocator.dat");
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
        FileLocator fl;
        fl.loadNBT(conf.destinationFolder.string() + "/incrementalLocator.dat");
        std::ofstream create(world.worldPath + "/sequoia.dat", std::ios::binary);
        fl.apply(diffs, relPath.generic_string());
        fl.saveNBT(conf.destinationFolder.string() + "/incrementalLocator.dat");
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