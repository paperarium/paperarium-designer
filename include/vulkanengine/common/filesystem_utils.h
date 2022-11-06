#ifndef FILESYSTEM_UTILS_H
#define FILESYSTEM_UTILS_H

#include <string>
#include <iostream>

#include "render_common.h"

#if !defined(USE_STD_FILESYSTEM) && !defined(USE_STD_EXPERIMENTAL_FILESYSTEM)
#define USE_STD_FILESYSTEM
#endif

#if defined(USE_STD_FILESYSTEM)
#if defined(VK_USE_PLATFORM_MACOS_MVK)
#include <filesystem>
using namespace std::__fs::filesystem;
#else
#include <filesystem>
using namespace std::filesystem;
#endif
#elif defined(USE_STD_EXPERIMENTAL_FILESYSTEM)
#include <experimental/filesystem>
using namespace std::experimental::filesystem;
#endif

namespace VulkanEngine {

class FS {
public:
  static std::string getPath(std::string p) {
#if defined(USE_STD_FILESYSTEM) || defined(USE_STD_EXPERIMENTAL_FILESYSTEM)
    path pa = path(std::string("../data/"+p).c_str());
    path pb=path(std::string(PROJECT_ABSOLUTE_PATH + ("/data/"+p)).c_str());
    if (exists(pa)) {
      return std::string("../data/"+p);
    } else if (exists(pb)) {
      return std::string(PROJECT_ABSOLUTE_PATH + ("/data/"+p));
    } else {
      std::cout << "Error! Could not find this file:" << p << std::endl;
      exit(1);
    }
#else
    return p;
#endif
  }
};
  
}

#endif /* FILESYSTEM_UTILS_H */
