cmake_minimum_required(VERSION 3.5.1)

project(simplefs)

set(CMAKE_CXX_STANDARD 14)

ADD_LIBRARY( simplefs STATIC lib.cpp SimpleFS.cpp INode.cpp FileDescriptor.cpp Lock.cpp ResourceManager.cpp)
ADD_EXECUTABLE( main main.cpp)
target_link_libraries(main simplefs)