#include <iostream>
#include <fstream>
#include "./World.h"
#include "./CoreGui.h"
#include "LogHelpers.h"
#include "./Backup.h"
#include "Version.h"
#include "../core/ZipCompressor.h"
#include <windows.h>
namespace {
  void attachWin32Console()
  {
   
    if (!AllocConsole())
      return;

    
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    
    std::ios::sync_with_stdio(true);

    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();
  }
}

int main(const int argc, const char* argv[]) {
  std::cout << "Sequoia v" << SEQUOIA_VERSION_MAJOR << "." << SEQUOIA_VERSION_MINOR << "." << SEQUOIA_VERSION_PATCH << " " << SEQUOIA_VERSION_CHANNEL << std::endl;
  std::cin.get();
  return 0;
  if (argc <= 1) {
    std::cout << "no arguments provided, launching GUI..." << std::endl;
    sequoia::CoreGui::launchGui();
    return 0;
  }
  std::string option = argv[1];
  if (option == "gui") {
    
    std::cout << "launching GUI..." << std::endl;
    sequoia::CoreGui::launchGui();
  }
  if (option == "background") {
    std::cout << "background process started!" << std::endl;


  }
  if (option == "") {
    sequoia::CoreGui::launchGui(); // pass to GUI
    return 0;
  }
  std::string worldPath = "";
  
  
  for (int i = 2; i < argc; i++) {
    if ((std::string(argv[i]) == "-w" || std::string(argv[i]) == "--world") && worldPath == "") {
      // then our world is here
      worldPath = std::string(argv[i+1]);
      continue;
    }
    if ((std::string(argv[i]) == "-d" || std::string(argv[i]) == "--debug")) {
        sequoia::setDebug(true);
        continue;
    }
  }
  if (worldPath == "") {
    std::cout << "error: world cannot be found! use -w or --world <world> to locate world." << std::endl;
    return 1;
  }
  // world to open
  

  sequoia::World world(worldPath);
 
  std::cout << "INFO: opened world " << worldPath << std::endl;
  
  if (option == "backups") {
    std::vector<sequoia::Backup> backups = world.getBackups();
    std::string command = argv[2];
    if (command == "restore") {

      if (backups.size() == 0) {
        std::cout << "no backups to restore! create a backup, then return to here to restore." << std::endl;
      } else {
        std::cout << "select backup to restore: " << std::endl;
        int i = 1;
        
        for (sequoia::Backup bc : backups) {
          std::chrono::system_clock::time_point tp =
            std::chrono::system_clock::from_time_t(bc.getBackupTime());
          auto tpS = std::chrono::floor<std::chrono::seconds>(tp);

          std::string formatted = std::format(
            "{:%b %d %Y %I:%M:%S %p}", tpS);
          
          
          std::cout << "  [" << i++ << "] " << formatted << " (" << bc.getRelativeLocation().substr(1) << ")" << std::endl;
        }
        std::string option;
        std::cin >> option;

        std::cout << "create backup before restoring? it is HIGHLY advised to backup your data before restoring to avoid data loss. (y/n)";

        std::string backupConfirmation;
        std::cin >> backupConfirmation;
        std::transform(backupConfirmation.begin(), backupConfirmation.end(), backupConfirmation.begin(), [](unsigned char c) {
          return std::tolower(c);
        });
        if (backupConfirmation == "y") {
          auto conf = world.getBackupConfig();
          if (!conf.has_value()) {
            std::cerr << "error: no backup config! rerun with backups create for backup config templates!" << std::endl;
            return 0;
          }
          sequoia::Backup bc(world, conf.value());
          std::cout << "starting backup!" << std::endl;
          bc.backup();
          std::cout << "backed up world!" << std::endl;
        }
        sequoia::Backup bc = backups[std::stoi(option) - 1];
        bc.restore();
        std::cout << "restored world!" << std::endl;
        return 0;
      }
      
    }
    if (command == "view") {
      
      if (backups.size() == 0) {

        std::cout << "backups:" << std::endl;
        std::cout << "  no backups, backup one and it will appear here!" << std::endl;
        return 0;
      }
      else {
        std::cout << "backups:" << std::endl;
        for (sequoia::Backup bc : backups) {
          std::chrono::system_clock::time_point tp =
            std::chrono::system_clock::from_time_t(bc.getBackupTime());
          auto tpS = std::chrono::floor<std::chrono::seconds>(tp);

          std::string formatted = std::format(
            "{:%b %d %Y %I:%M:%S %p}", tpS);


          std::cout << "  "<< formatted << " (" << bc.getRelativeLocation().substr(1) << ")" << std::endl;
          
        }
        return 0;
      }
    }
    if (command == "create") {
      std::string configTemplate = "";
      std::string folder = "";
      auto conf = world.getBackupConfig();
      if (!conf.has_value()) {
        std::cout << "WARN: backup config does not exist!" << std::endl;
        std::cout << "choose config template (default/high_compression/version_retention/high_compress_retention), or load your own by passing --backup-config <backup-config.json> or the filename in the input:" << std::endl;
        std::cin >> configTemplate;

        if (configTemplate == "default") {
          std::cout << "using default config... (features: deflate, multithreading, zip)" << std::endl;
          std::cout << "select backup folder for this world (default is .minecraft/backups/sequoia/<world name>, type default for that)" << std::endl;
          std::cin >> folder;
          if (folder == "default") {
            char path[MAX_PATH];
            GetEnvironmentVariable("APPDATA", path, MAX_PATH);
            folder = path + std::string("/.minecraft/backups/sequoia/") + world.getWorldName();
            std::cout << "creating directory in " << folder << std::endl;
          }
          std::filesystem::create_directories(folder);
          sequoia::BackupConfig c = {};
          c.backupFormat = sequoia::CompressionFormat::ZIP_COMPRESSED;
          c.backupNameFormat = "${worldName}_${dateTime}";
          c.destinationFolder = folder;
          world.setBackupConfig(c);
          conf = c;
        }
      }
      
      
      sequoia::Backup bc(world, conf.value());
      std::cout << "starting backup!" << std::endl;
      bc.backup();
      std::cout << "backed up world!" << std::endl;
      return 0;
    }
    else {
      std::cout << "ERROR: please select command. possible options are: view, create, delete, config, restore" << std::endl;
    }
  }
  else {
    std::cout << "ERROR: please select option. possible options are: backups[view|create|delete|config|restore], init, deinit" << std::endl;
  }
  
  return 0;
}
