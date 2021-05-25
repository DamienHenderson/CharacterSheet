cmake_minimum_required(VERSION 3.10)

project (CharacterSheet VERSION 1.0)


find_package(cereal CONFIG REQUIRED)
find_package(SFML COMPONENTS graphics window system  CONFIG REQUIRED )
find_package(unofficial-nativefiledialog CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)

add_executable(CharacterSheet Source/main.cpp)


target_link_libraries(CharacterSheet PRIVATE sfml-system sfml-network sfml-graphics sfml-window ImGui-SFML::ImGui-SFML unofficial::nativefiledialog::nfd cereal)