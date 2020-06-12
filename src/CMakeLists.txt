ADD_LIBRARY( simplefs STATIC lib.cpp SimpleFS.cpp INode.cpp FileDescriptor.cpp Lock.cpp ResourceManager.cpp)
ADD_EXECUTABLE( main main.cpp)
target_link_libraries(main simplefs)
