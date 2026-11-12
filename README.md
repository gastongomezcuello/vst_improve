# VST Improve

VST Improve is a JUCE audio plugin for tracking focused work sessions and breaks. It includes configurable work and break limits, a session work target, a work/rest balance, and budget tracking based on hours worked.

The project builds as a VST3 plugin and a standalone desktop application.

## Requirements

- Windows 10 or later
- Visual Studio 2022 with the Desktop development with C++ workload
- CMake 3.22 or later
- Git and an internet connection for the first configure (CMake fetches JUCE 8.0.13)

## Build

Run these commands from the project directory:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Build products are generated under `build/VST_IMPROVE_artefacts/Release/`. The VST3 is also copied to the default plugin location by JUCE after a successful build.

## Project layout

- `Source/` contains the plugin processor, editor, and standalone app entry point.
- `CMakeLists.txt` defines the plugin targets and fetches the pinned JUCE dependency into the build directory.
- `Assets/`, `Docs/`, and `Tests/` are reserved for project resources, documentation, and tests.

JUCE is not stored in this repository; CMake downloads it during configuration. Review JUCE's licensing terms before distributing builds that use it.
