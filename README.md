## Installation Instructions

To install PseuWoW on Linux please run the following commands

    autoreconf -if
    ./configure --prefix=/path/to/pseuwow
    make
    make install

configure accepts the following parameters:

    --with-irrklang    Builds a stub implementation of Irrklang (necessary for x64 systems)


After installation edit the config files in bin/conf and rename them to PseuWoW.conf, gui.conf, ScriptConfig.conf and users.conf.
If UseMPQ in PseuWoW.conf is set, create a link/copy to your "TheGame" **Data** folder in bin. Otherwise use stuffextract with the appropriate parameters to extract data from "TheGame" archives and copy it to bin/data

To run PseuWoW please execute

    ./pseuwow