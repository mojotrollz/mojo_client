include_directories (${PROJECT_SOURCE_DIR}/src/dep/include ${PROJECT_SOURCE_DIR}/src/shared ${PROJECT_SOURCE_DIR}/src/Client)

add_executable (viewer 
main.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CM2MeshFileLoader.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CWMOMeshFileLoader.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CImageLoaderBLP.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/SImage.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/MemoryInterface.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CMDHMemoryReadFile.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CBoneSceneNode.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/SSkinnedMesh.cpp

)

# Link the executable to the libraries.
set(VIEWER_LIBS shared irrlicht StormLib zthread zlib)
if(UNIX)
  list(APPEND VIEWER_LIBS GL Xxf86vm bz2)
endif()
if(WIN32)
  list(APPEND VIEWER_LIBS Winmm)
endif()

target_link_libraries (viewer ${VIEWER_LIBS} ) 

install(TARGETS viewer DESTINATION ${CMAKE_INSTALL_PREFIX})
