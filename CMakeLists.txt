cmake_minimum_required(VERSION 3.10)
project(sd_sockets)

add_library(${PROJECT_NAME} INTERFACE
  include/${PROJECT_NAME}/${PROJECT_NAME}.hpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
