include_directories (${PROJECT_SOURCE_DIR}/src/dep/include ${PROJECT_SOURCE_DIR}/src/shared ${PROJECT_SOURCE_DIR}/src/Client)

set(PSEUWOW_LIBS
PseuGUI DefScript IrrKlang irrlicht shared zthread StormLib_static zlib ${OPENSSL_LIBRARIES} ${OPENSSL_EXTRA_LIBRARIES}
)

link_directories (${CMAKE_INSTALL_PREFIX})
if(WIN32)
  link_directories (${PROJECT_SOURCE_DIR}/src/dep/lib/static)
  set(PSEUWOW_LIBS ${PSEUWOW_LIBS} Winmm)
endif()
if(UNIX)
  set(EXECUTABLE_LINK_FLAGS "-pthread" ${EXECUTABLE_LINK_FLAGS})
  set(PSEUWOW_LIBS ${PSEUWOW_LIBS} GL Xxf86vm bz2)
endif()

add_subdirectory (DefScript)
add_subdirectory (GUI)



add_executable (pseuwow
Realm/RealmSession.cpp
Realm/RealmSocket.cpp

World/Bag.cpp
World/CacheHandler.cpp
World/Channel.cpp
World/CMSGConstructor.cpp
World/Corpse.cpp
World/DynamicObject.cpp
World/GameObject.cpp
World/Item.cpp
World/MapMgr.cpp
World/MovementMgr.cpp
World/Object.cpp
World/ObjMgr.cpp
World/Opcodes.cpp
World/Player.cpp
World/Unit.cpp
World/UpdateData.cpp
World/UpdateFields.cpp
World/World.cpp
World/WorldPacket.cpp
World/WorldSession.cpp
World/WorldSocket.cpp

Cli.cpp
ControlSocket.cpp
DefScriptInterface.cpp
main.cpp
PseuWoW.cpp
RemoteController.cpp
SCPDatabase.cpp
)



# Link the executable to the libraries.
target_link_libraries (pseuwow ${PSEUWOW_LIBS})

set_target_properties(pseuwow PROPERTIES LINK_FLAGS
  "${EXECUTABLE_LINK_FLAGS}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_INSTALL_PREFIX}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_INSTALL_PREFIX}/bin"
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin"
)
install(TARGETS pseuwow DESTINATION ${CMAKE_INSTALL_PREFIX})
