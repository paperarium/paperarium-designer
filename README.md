<h1 align="center">
  <br>
  <a href="https://paperarium.place"><img src="https://user-images.githubusercontent.com/30581915/198012667-d384e1c4-33c3-4960-8f72-00f10803fc39.png" alt="Paperarium Designer" width="200"></a>
  <br>
  Paperarium Designer
  <br>
</h1>

<h4 align="center">An open source, cross-platform desktop app for unfolding 3D models into <a href="https://paperarium.place" target="_blank">papercrafts</a>.</h4>

<p align="center">
  <a href="#key-features">Key Features</a> •
  <a href="#how-to-develop">How To Develop</a> •
  <a href="#download">Download</a> •
  <a href="#credits">Credits</a> •
  <a href="#related">Related</a> •
  <a href="#license">License</a>
</p>

> **Note**
> This is a work in progress. Below is a screenshot of the latest state of development.

<img width="1440" alt="Screenshot 2022-11-19 at 3 53 15 AM" src="https://user-images.githubusercontent.com/30581915/202843195-2e1e8fb3-8a35-4ba5-af18-0a576bee6746.png">

## Key Features

* Open source and free to use
* Cross platform
  - Windows, macOS and Linux ready.
* Vulkan-backed rendering engine

## How To Develop

To be able to build this application from source, you'll need [Git](https://git-scm.com), [Qt](https://www.qt.io), [Qt Creator](https://www.qt.io/product/development-tools), and the [Vulkan SDK](https://vulkan.lunarg.com). Also make sure you have `pkg-config` for VCPKG, installable from Homebrew. Then, from your command line:

```bash
# Clone this repository and the VCPKG submodule
$ git clone --recurse-submodules https://github.com/paperarium/paperarium-designer

# Go into the repository
$ cd paperarium-designer

# Set up VCPKG and install the ASSIMP, GLM, OpenMP, and Vulkan dependencies
$ ./lib/vcpkg/bootstrap-vcpkg.sh
$ ./lib/vcpkg/vcpkg install assimp glm openmp vulkan
```

> **Note**
> I develop on MacOS 10.15, so YMMV on other platforms. Please contribute!

Now start QT Creator and open up the project by going to `File` > `Open File or Project` and selecting the `CMakeLists.txt` file. You'll need to make the VCPKG toolchain available to your CMake to be able to automatically link ASSIMP.

This can be done by changing your Kit configuration in Qt Creator to add the `-DCMAKE_TOOLCHAIN_FILE` cmake flag. Go to `Projects` and add the following key-value pair to your kit's Initial Configuration:

```bash
# Key, Value
CMAKE_TOOLCHAIN_FILE, <path-to-your-clone>/lib/vcpkg/scripts/buildsystems/vcpkg.cmake
```

This should enable you to now build and run Paperarium Designer from with Qt Creator. I often do code work in VSCode as well, which necessitates installing the Qt Tools VSCode extension. Happy developing!

## Download

Paperarium does not yet have a release package, or even a beta version. Hopefully I will be able to put something out soon.

## Credits

The early code for this software would not have been possible without the fantastic work of [Sascha Willems](https://github.com/SaschaWillems) and [engineer1109](https://github.com/engineer1109) in setting up their own basic cross-platform Vulkan applications. Other packages / frameworks used:

- [Qt](https://www.qt.io)
- [Vulkan](https://www.vulkan.org)

## Related

[Paperarium Place](https://paperarium.place) - An online compendium for papercrafts

## License

LGPL

---

> [evankirkiles.com](https://evankirkiles.com) &nbsp;&middot;&nbsp;
> GitHub [@evankirkiles](https://github.com/evankirkiles)
