# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2016 Blender Foundation. All rights reserved.
# Modified by Evan Kirkiles

# Libraries configuration for Apple.

# Enable MoltenVK
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK -DVK_EXAMPLE_XCODE_GENERATED")