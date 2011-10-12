include_directories (${PROJECT_SOURCE_DIR}/src/dep/include)

set(STORMLIB_SOURCES huffman/huff.cpp
 wave/wave.cpp
 SAttrFile.cpp
 SCommon.cpp
 SCompression.cpp
 SFileCompactArchive.cpp
 SFileCreateArchiveEx.cpp
 SFileExtractFile.cpp
 SFileFindFile.cpp
 SFileOpenArchive.cpp
 SFileOpenFileEx.cpp
 SFileReadFile.cpp
 SListFile.cpp
 pklib/crc32_pk.c
 pklib/explode.c
 pklib/implode.c
 misc/crc32.cpp
 misc/md5.cpp)

if(UNIX)
      set(STORMLIB_SOURCES ${STORMLIB_SOURCES}  StormPortLinux.cpp)
endif()

if(WIN32)
      set(STORMLIB_SOURCES ${STORMLIB_SOURCES}
      bzip2/blocksort.c
      bzip2/bzip2.c
      bzip2/bzip2recover.c
      bzip2/bzlib.c
      bzip2/compress.c
      bzip2/crctable.c
      bzip2/decompress.c
      bzip2/dlltest.c
      bzip2/huffman.c
      bzip2/mk251.c
      bzip2/randtable.c
      bzip2/spewG.c
      bzip2/unzcrash.c
)
endif()

add_library(StormLib ${STORMLIB_SOURCES})