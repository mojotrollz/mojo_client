include_directories (${PROJECT_SOURCE_DIR}/src/dep/include ${PROJECT_SOURCE_DIR}/src/shared)

add_executable (stuffextract 
StuffExtract.cpp
)

# Link the executable to the libraries.
target_link_libraries (stuffextract shared StormLib zlib bz2) 

install(TARGETS stuffextract DESTINATION ${CMAKE_INSTALL_PREFIX})
