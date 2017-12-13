project(GloVe DESCRIPTION "GloVe: Global Vectors for Word Representation")

cmake_minimum_required(VERSION 3.1.0)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Ofast)

include_directories(src)

add_library(vocabulary OBJECT src/vocabulary.cpp)
add_library(util OBJECT src/util.cpp)
add_library(cooccur OBJECT src/cooccur.cpp)
add_library(glove OBJECT src/glove.cpp)
add_library(glove_all
  $<TARGET_OBJECTS:vocabulary>
  $<TARGET_OBJECTS:cooccur>
  $<TARGET_OBJECTS:glove>
  $<TARGET_OBJECTS:util>)

add_executable(train train.cpp)
target_link_libraries(train armadillo glove_all)

add_executable(analogy analogy.cpp)
target_link_libraries(analogy armadillo glove_all)

add_executable(test_util test/util.cpp)
target_link_libraries(test_util gtest gtest_main glove_all)
