cmake_minimum_required(VERSION 3.23)

project(bp2p)

add_executable(a.out)

target_sources(a.out
  PRIVATE
    test.cpp
)
