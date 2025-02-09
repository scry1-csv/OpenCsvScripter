# OpenCsvScripter
Can be used to create `.csv` script files for Vorze A10CycloneSA / UFOSA / UFOTW. (NSFW) 
Forked from [OpenFunscripter](https://github.com/OpenFunscripter/OFS).
The project is based on OpenGL, SDL2, ImGui, libmpv, & all these other great [libraries](https://github.com/OpenFunscripter/OpenFunscripter/tree/master/lib).

## Changes from OpenFunscripter  
- The script timeline graph display has been modified to a rectangular wave suitable for rotational scripts  
- CSV files for Vorze A10 Cyclone SA / UFOSA / UFOTW can be exported

## Exporting CSV for UFOTW  
If two or more scripts are loaded into the project, you can export them via the menu:  
"File" → "Export..." → "Export as UFOTW script".  

The first script from the top will be saved as the left side of the UFOTW, and the second script will be saved as the right side.  

Currently, there is no functionality to select from more than three loaded scripts or swap left and right.  

## How to build ( for people who want to contribute or fork )
1. Clone the repository
2. `cd "OpenCsvScripter"`
3. `git submodule update --init`
4. Run CMake and compile

Known linux dependencies to just compile are `build-essential libmpv-dev libglvnd-dev`.  

## Windows libmpv binaries used
Currently using: [mpv-dev-x86_64-v3-20220925-git-56e24d5.7z (it's part of the repository)](https://sourceforge.net/projects/mpv-player-windows/files/libmpv/)

## Platforms
I'm providing windows binaries.
In theory linux or OSX should work as well but I lack the environment to set any of that up.

## License
GPLv3