#pragma once

#include <zip.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <zlib.h>
#include <cstdlib>
#include <cstring>
#include "Compressor.h"
#include <stdlib.h>
namespace fs = std::filesystem;
namespace sequoia {
 
  class ZipCompressor : public Compressor {
  private:
    int numFiles = 0;
    bool progressEnabled = false;
    int numFilesCompressed = 0;
    template <typename T>
    class ThreadSafeQueue {
    private:
      std::queue<T> queue;
      std::mutex mutex;
      
    public:
      std::condition_variable cv;
      void push(T value);
      bool pop(T& value, bool& done);
      bool empty();
    };

    struct FileTask {
      fs::path diskPath;
      fs::path zipPath;
    };

    struct CompressedTask {
      std::vector<uint8_t> data;
      fs::path zipPath;
    };

    std::mutex zipMutex;
    using Compressor::Compressor;
    bool compressFile(const fs::path& filePath, std::vector<uint8_t>& outBuffer, int level = Z_DEFAULT_COMPRESSION);
    void workerCompress(ThreadSafeQueue<FileTask>& fileQueue,
      ThreadSafeQueue<CompressedTask>& compressedQueue,
      bool& done);
    void enqueueFolder(ThreadSafeQueue<FileTask>& queue,
      const fs::path& folder,
      const fs::path& zipBase,
      zip_t* zip);
    void addCompressedFiles(zip_t* zip, ThreadSafeQueue<CompressedTask>& compressedQueue);
  public:
    bool compress();
    bool compress(bool progress);
  };
}