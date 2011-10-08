include_directories (${PROJECT_SOURCE_DIR}/src/dep/include ${PROJECT_SOURCE_DIR}/src/shared ${PROJECT_SOURCE_DIR}/src/Client)

add_executable (viewer 
main.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CM2MeshFileLoader.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CWMOMeshFileLoader.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CImageLoaderBLP.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/SImage.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/MemoryInterface.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CMDHMemoryReadFile.cpp
)

# Link the executable to the libraries.
target_link_libraries (viewer shared irrlicht StormLib zthread GL Xxf86vm zlib bz2 ) 

install(TARGETS viewer DESTINATION ${CMAKE_INSTALL_PREFIX})
