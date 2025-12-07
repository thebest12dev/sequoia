#pragma once
#include <filesystem>

namespace sequoia {
  class Compressor {
  protected:
    std::filesystem::path& target;
    std::filesystem::path& dest;

    
  public:
    virtual void setTarget(std::filesystem::path& folder);
    virtual void setDestination(std::filesystem::path& dest);
    Compressor(std::filesystem::path& folder, std::filesystem::path& dest);
    virtual bool compress() = 0;
  };
}