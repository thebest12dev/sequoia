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
#include "ZipCompressor.h"
#include <stdlib.h>
namespace fs = std::filesystem;
namespace {

  std::mutex logMutex;

}
namespace sequoia {
  
  
  template<typename T>
  void ZipCompressor::ThreadSafeQueue<T>::push(T value) {
    {
      std::lock_guard<std::mutex> lock(mutex);
      queue.push(std::move(value));
    }
    cv.notify_one();
  };
  template<typename T>
  bool ZipCompressor::ThreadSafeQueue<T>::pop(T& value, bool& done) {
      std::unique_lock<std::mutex> lock(mutex);
      cv.wait(lock, [this, &done]() { return !queue.empty() || done; }); // capture by reference!
      if (queue.empty()) return false;
      value = std::move(queue.front());
      queue.pop();
      return true;
  }

  template<typename T>
  bool ZipCompressor::ThreadSafeQueue<T>::empty() {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty();
  }
  bool ZipCompressor::compress(bool progress) {
    progressEnabled = progress;
    return this->compress();
  }
  bool ZipCompressor::compress() {

    
    int err = 0;
    zip_t* zip = zip_open(Compressor::dest.string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!zip) { 
      return false;
    }

    ThreadSafeQueue<FileTask> fileQueue;
    ThreadSafeQueue<CompressedTask> compressedQueue;
    bool done = false;

    
    std::vector<std::thread> workers;
    for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) {

      workers.emplace_back(&ZipCompressor::workerCompress, this, std::ref(fileQueue), std::ref(compressedQueue), std::ref(done));
    }

    enqueueFolder(fileQueue, Compressor::target, "", zip);

    done = true;
    fileQueue.cv.notify_all(); // wake all workers waiting
    for (auto& t : workers) t.join();


    addCompressedFiles(zip, compressedQueue);

    if (zip_close(zip) < 0) {
      return false;
    }

    return true;
  }
  bool ZipCompressor::compressFile(const fs::path& filePath, std::vector<uint8_t>& outBuffer, int level) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) return false;

    auto size = file.tellg();
    file.seekg(0);

    std::vector<uint8_t> in(size);
    file.read(reinterpret_cast<char*>(in.data()), size);

    uLongf maxCompressedSize = compressBound(size);
    outBuffer.resize(maxCompressedSize);

    int ret = compress2(outBuffer.data(), &maxCompressedSize, in.data(), size, level);
    if (ret != Z_OK) return false;

    outBuffer.resize(maxCompressedSize);
    numFilesCompressed++;
    if (progressEnabled) {
      std::lock_guard<std::mutex> lock(logMutex);
      std::cout << "compressed "<< numFilesCompressed << "/" << numFiles << "\r";
    }
    
    return true;
  }

  void ZipCompressor::workerCompress(ThreadSafeQueue<FileTask>& fileQueue,
    ThreadSafeQueue<CompressedTask>& compressedQueue,
    bool& done) {
    FileTask task;
    while (fileQueue.pop(task, done)) {
      std::vector<uint8_t> compressed;
      if (!compressFile(task.diskPath, compressed)) {
        continue;
      }
      compressedQueue.push({ std::move(compressed), task.zipPath });
    }
  }


  void ZipCompressor::enqueueFolder(ThreadSafeQueue<FileTask>& queue,
    const fs::path& folder,
    const fs::path& zipBase,
    zip_t* zip) {
    if (fs::is_empty(folder)) {
      std::string dirName = zipBase.generic_string() + "/";
      std::lock_guard<std::mutex> lock(zipMutex);
      if (zip_dir_add(zip, dirName.c_str(), ZIP_FL_ENC_UTF_8) < 0) {
        std::cerr << "failed to add empty folder: " << dirName << "\n";
      }
    }

    for (const auto& entry : fs::directory_iterator(folder)) {
      fs::path relativePath = zipBase / entry.path().filename();
      if (entry.is_directory()) {
        enqueueFolder(queue, entry.path(), relativePath, zip);
      }
      else if (entry.is_regular_file()) {
        numFiles++;
        queue.push({ entry.path(), relativePath });
      }
    }
  }


  void ZipCompressor::addCompressedFiles(zip_t* zip, ThreadSafeQueue<CompressedTask>& compressedQueue) {
    CompressedTask task;
    bool _ = true;
    while (compressedQueue.pop(task, _)) {

      size_t dataSize = task.data.size();
      void* buf = nullptr;
      if (dataSize > 0) {
        buf = std::malloc(dataSize);
        if (!buf) {
          std::cerr << "out of memory while preparing zip source for: " << task.zipPath << "\n";
          continue;
        }
        std::memcpy(buf, task.data.data(), dataSize);
      }

      zip_source_t* source = zip_source_buffer(zip, buf, dataSize, 1); 
      if (!source) {
        if (buf) std::free(buf);
        std::cerr << "failed to create zip source for: " << task.zipPath << "\n";
        continue;
      }

      zip_int64_t idx;
      {
        std::lock_guard<std::mutex> lock(zipMutex);
        idx = zip_file_add(zip, task.zipPath.generic_string().c_str(), source,
          ZIP_FL_ENC_UTF_8 | ZIP_FL_OVERWRITE);
        if (idx < 0) {
          zip_source_free(source);
          std::cerr << "failed to add file to zip: " << task.zipPath << "\n";
        }
        else {

          zip_set_file_compression(zip, idx, ZIP_CM_DEFLATE, 6);
        }
      }
    }
  }
}









