#include <iostream>
#include <fstream>
#include "./World.h"
#include "./CoreGui.h"
#include "LogHelpers.h"
#include "./Backup.h"
#include "Version.h"
#include "../core/ZipCompressor.h"
int main(const int argc, const char* argv[]) {
  std::cout << "Sequoia v" << SEQUOIA_VERSION_MAJOR << "." << SEQUOIA_VERSION_MINOR << "." << SEQUOIA_VERSION_PATCH << " " << SEQUOIA_VERSION_CHANNEL << std::endl;
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
    std::cout << "launching background process..." << std::endl;
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);


    char cmdLine[] = "internal -Dtask=sequoia_background_process -Dnamed_pipe=true -Dpipe_name=sequoia \
      -Dprotocol=sequoia -Dconsole=false -Dinvoked_by=sequoia_console";

    BOOL ok = CreateProcessA(
        path,
        cmdLine,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

  }
  if (option == "") {
    sequoia::CoreGui::launchGui(); // pass to GUI
    return 0;
  }
  std::string worldPath = "";
  
  
  for (int i = 2; i < argc-1; i++) {
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
    std::string command = argv[2];
    
    if (command == "view") {
      std::vector<sequoia::Backup> backups = world.getBackups();
      if (backups.size() == 0) {
        std::cout << "backups:" << std::endl;
        std::cout << "  no backups, backup one and it will appear here!" << std::endl;

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
      std::cout << "ERROR: please select command. possible options are: view, create, delete, config" << std::endl;
    }
  }
  else {
    std::cout << "ERROR: please select option. possible options are: backups[view|create|delete|config], init, deinit" << std::endl;
  }
  
  return 0;
}
