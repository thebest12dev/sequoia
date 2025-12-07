#pragma once
#include <filesystem>
#include "Compressor.h"
namespace fs = std::filesystem;
namespace sequoia {
  void Compressor::setTarget(fs::path& folder) {
    this->target = folder;
  };
  void Compressor::setDestination(fs::path& dest) {
    this->dest = dest;

  };
  Compressor::Compressor(std::filesystem::path& folder, std::filesystem::path& dest) : target(folder), dest(dest) {};
}