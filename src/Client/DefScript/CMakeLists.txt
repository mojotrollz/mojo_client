include_directories (${PROJECT_SOURCE_DIR}/src/dep/include)
add_library(DefScript
DefScriptBBFunctions.cpp
DefScriptFileFunctions.cpp
DefScriptListFunctions.cpp
DynamicEvent.cpp
DefScript.cpp
DefScriptFunctions.cpp
DefScriptTools.cpp
VarSet.cpp
)