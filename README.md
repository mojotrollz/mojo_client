## Installation Instructions

PseuWoW installation uses CMake. Please create a new directory for the build and create Build Files appropriate for your system using cmake or cmake-gui. If you prefer to use the command line, this is an example of the process, starting in the PseuWoW root directory:

    mkdir build
    cd build
    cmake ..
    make
    make install

To build stuffextract, as well as the model viewer, define BUILD_TOOLS in cmake by appending -DBUILD_TOOLS=1 to the command line.
To build a Debug version of PseuWoW, define DEBUG in cmake by appending -DDEBUG=1 to the command line

After installation edit the config files in bin/conf and rename them to PseuWoW.conf, gui.conf, ScriptConfig.conf and users.conf.
If UseMPQ in PseuWoW.conf is set, create a link/copy to your "TheGame" **Data** folder in bin. Otherwise use stuffextract with the appropriate parameters to extract data from "TheGame" archives and copy it to bin/data

To run PseuWoW please execute

    ./pseuwow

The model viewer is strictly a debugging tool. It is not user friendly. The viewer expects to find MPQ files in ./Data and uses only textures from MPQ files. Models have to be extracted to be viewed. The viewer cannot browse MPQ files.