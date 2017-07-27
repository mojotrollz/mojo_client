include_directories (${PROJECT_SOURCE_DIR}/src/dep/include ${PROJECT_SOURCE_DIR}/src/shared ${PROJECT_SOURCE_DIR}/src/Client)

add_executable (viewer
main.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CM2MeshFileLoader.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CWMOMeshFileLoader.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/MemoryInterface.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CMDHMemoryReadFile.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CBoneSceneNode.cpp
${PROJECT_SOURCE_DIR}/src/Client/GUI/CM2Mesh.cpp
)

# Link the executable to the libraries.
set(VIEWER_LIBS shared irrlicht StormLib_static zthread zlib)
if(UNIX)
  list(APPEND VIEWER_LIBS GL Xxf86vm bz2)
  set(EXECUTABLE_LINK_FLAGS "-pthread" ${EXECUTABLE_LINK_FLAGS})
endif()
if(WIN32)
  list(APPEND VIEWER_LIBS Winmm)
endif()

target_link_libraries (viewer ${VIEWER_LIBS} )
set_target_properties (viewer PROPERTIES LINK_FLAGS
  "${EXECUTABLE_LINK_FLAGS}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_INSTALL_PREFIX}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_INSTALL_PREFIX}/bin"
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin"
)
install(TARGETS viewer DESTINATION ${CMAKE_INSTALL_PREFIX})
