/*
 * render_common.h
 * Created by: Evan Kirkiles
 * Created on: Fri Oct 28 2022
 * for Paperarium Design
 * 
 * Most, if not all code written by wjl:
 * https://github.com/engineer1109/LearnVulkan/blob/master/source/common/render_common.h
 */

#ifndef RENDER_COMMON_H
#define RENDER_COMMON_H

// Base
#include <cstdio>
#include <cmath>
#include <cstring>
#include <vector>
#include <array>
#include <numeric>
#include <string>
#include <memory>
#include <algorithm>
#include <thread>
#include <functional>
#include <ciso646>
#include <random>

// ---------------- PLATFORM-SPECIFIC ----------------

#ifdef VK_USE_PLATFORM_XCB_KHR
#include <xcb/xcb.h>
#endif

// GLM
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

// Unix
#ifdef __unix__
#include <unistd.h>
#endif

// Windows
#ifdef WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <ShellScalingAPI.h>
#endif

// Vulkan
#include <vulkan/vulkan.h>


// -------------------- DEFINES --------------------

// Common C++ inserts
#define interface class
#define DELETE_PTR(x) if (x) { delete x; x = nullptr; }

// Logging
#define LOG(...) printf(__VA_ARGS__)
#define LOGI(...) printf(__VA_ARGS__)

// Vulkan buffer indices
#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1

#endif // RENDER_COMMON_H
