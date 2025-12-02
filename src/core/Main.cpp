
#include <iostream>
#include <fstream>
#include "./World.h"
#include "Backup.h"
int main(const int argc, const char* argv[]) {
  std::cout << "Sequoia v0.0.1 alpha" << std::endl;

  std::string worldPath = "";
 
  
  for (int i = 2; i < argc-1; i++) {
    if ((std::string(argv[i]) == "-w" || std::string(argv[i]) == "--world") && worldPath == "") {
      // then our world is here
      worldPath = std::string(argv[i+1]);
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
  std::string option = argv[1];
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
      std::cout << "WARN: backup config does not exist!" << std::endl;
      std::cout << "choose config template (default/high_compression/version_retention/high_compress_retention), or load your own by passing --backup-config <backup-config.json> or the filename in the input:" << std::endl;
      std::cin >> configTemplate;
      
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
