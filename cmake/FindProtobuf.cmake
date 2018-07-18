find_library(Protobuf_LIBRARY
  NAMES protobuf)
mark_as_advanced(Protobuf_LIBRARY)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads)
if(Threads_FOUND)
  list(APPEND Protobuf_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
  set(Protobuf_LIBRARIES "${Protobuf_LIBRARIES}" PARENT_SCOPE)
endif()
      
find_path(Protobuf_INCLUDE_DIR
  google/protobuf/service.h)
mark_as_advanced(Protobuf_INCLUDE_DIR)

# Find the protoc Executable
find_program(Protobuf_PROTOC_EXECUTABLE
    NAMES protoc
    DOC "The Google Protocol Buffers Compiler")
mark_as_advanced(Protobuf_PROTOC_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Protobuf DEFAULT_MSG
    Protobuf_LIBRARY Protobuf_INCLUDE_DIR)
if(PROTOBUF_FOUND)
  set(Protobuf_INCLUDE_DIRS ${Protobuf_INCLUDE_DIR})
endif()

if(Protobuf_LIBRARY)
  if(NOT TARGET protobuf::libprotobuf)
    add_library(protobuf::libprotobuf UNKNOWN IMPORTED)
    set_target_properties(protobuf::libprotobuf PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${Protobuf_INCLUDE_DIR}")
    if(EXISTS "${Protobuf_LIBRARY}")
      set_target_properties(protobuf::libprotobuf PROPERTIES
        IMPORTED_LOCATION "${Protobuf_LIBRARY}")
    endif()
    if(EXISTS "${Protobuf_LIBRARY_RELEASE}")
      set_property(TARGET protobuf::libprotobuf APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(protobuf::libprotobuf PROPERTIES
        IMPORTED_LOCATION_RELEASE "${Protobuf_LIBRARY_RELEASE}")
    endif()
  endif()
endif()
