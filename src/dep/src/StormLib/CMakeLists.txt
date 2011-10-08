include_directories (${PROJECT_SOURCE_DIR}/src/dep/include)
add_library(StormLib huffman/huff.cpp
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
 StormPortLinux.cpp
 pklib/crc32_pk.c
 pklib/explode.c
 pklib/implode.c
 misc/crc32.cpp
 misc/md5.cpp)