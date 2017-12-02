PROJECT(GloVe)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

ADD_COMPILE_OPTIONS(-std=c++11 -Ofast)

INCLUDE_DIRECTORIES(src)
FILE(GLOB src src/*.cpp)
ADD_EXECUTABLE(train train.cpp ${src})
ADD_EXECUTABLE(analogy analogy.cpp ${src})

TARGET_LINK_LIBRARIES(train armadillo)
TARGET_LINK_LIBRARIES(analogy armadillo)
