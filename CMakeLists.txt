cmake_minimum_required(VERSION 3.27)
project(game_networking_test
    LANGUAGES CXX
    VERSION 0.0.0.1
)
add_definitions(PROJECT_VERSION=${PROJECT_VERSION})


# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v5.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)
# ------------------------------------------------------------------------------

include_directories(src/)

add_executable(client_test src/client_test.cpp)
add_executable(master_test_server src/master_test_server.cpp)
add_executable(master_test_client src/master_test_client.cpp)
add_executable(lobby_test src/lobby_test.cpp)
add_executable(game_test src/game_test.cpp)

add_subdirectory(src/net)

target_link_libraries(master_test_server
    PRIVATE client
    PRIVATE server_master
    PRIVATE server_lobby
    PRIVATE server_game

    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)
target_link_libraries(master_test_client
    PRIVATE client
    PRIVATE server_master
    PRIVATE server_lobby
    PRIVATE server_game

    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
)

# target_include_directories(client_test PRIVATE src)