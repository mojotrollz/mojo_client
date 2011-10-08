include_directories (${PROJECT_SOURCE_DIR}/src/dep/include ${PROJECT_SOURCE_DIR}/src/shared ${PROJECT_SOURCE_DIR}/src/Client)

add_subdirectory (DefScript)
add_subdirectory (GUI)

if(UNIX)
  set(EXECUTABLE_LINK_FLAGS "-pthread")
endif()

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
target_link_libraries (pseuwow PseuGUI DefScript irrklang irrlicht GL Xxf86vm shared zthread StormLib zlib bz2 ssh crypto) 

install(TARGETS pseuwow DESTINATION ${CMAKE_INSTALL_PREFIX})
