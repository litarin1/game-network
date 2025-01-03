cmake_minimum_required(VERSION 3.27)
project(game_networking
    LANGUAGES CXX
)

add_library(server_master STATIC master.cpp)
add_library(server_lobby STATIC lobby.cpp)
### controls godot server instances
add_library(server_game STATIC game.cpp)

add_library(client STATIC client.cpp)
