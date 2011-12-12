include_directories (${PROJECT_SOURCE_DIR}/src/dep/include ${PROJECT_SOURCE_DIR}/src/shared)

add_executable (stuffextract
StuffExtract.cpp
)

# Link the executable to the libraries.
set(STUFFEXTRACT_LIBS shared StormLib_static zlib)
if(UNIX)
  list(APPEND STUFFEXTRACT_LIBS bz2)
endif()
if(WIN32)
  list(APPEND STUFFEXTRACT_LIBS Winmm)
endif()

target_link_libraries (stuffextract ${STUFFEXTRACT_LIBS} )

install(TARGETS stuffextract DESTINATION ${CMAKE_INSTALL_PREFIX})
