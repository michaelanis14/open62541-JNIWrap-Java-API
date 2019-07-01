cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME Opcua-Java-API)
# Output Variables
set(OUTPUT_DEBUG Debug/bin)
set(OUTPUT_RELEASE Release/bin)
# Folders files
set(CPP_DIR_1 ./opcua_java_api/)
set(HEADER_DIR_1 ./opcua_java_api/)
set(INCLUDEDIR_1 ../OpcUa/OpcUa_include)
set(INCLUDEDIR_2	././OpcUa/OpcUa_lib)
set(INCLUDEDIR_3 ././OpcUa/OpcUa_bin)
set(MBED_DIR ../OpcUa/OpcUa_include/mbedtls)
set(MBCRYPTO_LIBRARY mbedcrypto)
set(IPHLPAPI_LIBRARY iphlpapi)
set(MBEDTLS_LIBRARY mbedtls)
set(MBEDX_LIBRARY mbedx509)



############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} C CXX)
find_package (Java REQUIRED)
find_package (JNI REQUIRED)
include (UseJava)


if(EXISTS "${INCLUDEDIR_2}/${MBCRYPTO_LIBRARY}.*" )
message("-- Found The Libraries.")
	add_library(mbedcrypto INTERFACE)
	add_library(iphlpapi INTERFACE)
	add_library(mbedtls INTERFACE)
	add_library(mbedx509 INTERFACE)
	target_include_directories(mbedcrypto INTERFACE ${INCLUDEDIR_2})
else()
    message("-- Unable to find The Libraries.")
	execute_process(COMMAND git init )
	execute_process(COMMAND git submodule add -b mbedtls-2.16 https://github.com/ARMmbed/mbedtls.git )
	#execute_process(COMMAND git submodule update  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	option(DUSE_SHARED_MBEDTLS_LIBRARY "" ON)
	add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/mbedtls)

	#	target_link_libraries(mbedcrypto  ${MBCRYPTO_LIBRARY})
	#	target_link_libraries(mbedtls  ${MBEDTLS_LIBRARY})
	#	target_link_libraries(mbedx509  ${MBEDX_LIBRARY})
endif()


# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

############## Artefacts Output #################
# Defines outputs , depending Debug or Release. #
#################################################

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
endif()

# Include directories 
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${INCLUDEDIR_1}
	${JNI_INCLUDE_DIRS}
)
link_directories(${INCLUDEDIR_2})
################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

if(MSVC)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /MD /Od /EHsc")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /GL /Od /Oi /Gy /EHsc")
endif(MSVC)
if(NOT MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
   endif()
endif(NOT MSVC)

################ Files ################
#   --   Add files to project.   --   #
#######################################

file(GLOB SRC_FILES
    ${CPP_DIR_1}/*.c
    ${CPP_DIR_1}/*.cpp
    ${CPP_DIR_1}/*.cxx
    ${HEADER_DIR_1}/*.h
)

# Add library to build.
add_library(${PROJECT_NAME} SHARED
   ${SRC_FILES}
)

if(MSVC)
   target_link_libraries(${PROJECT_NAME} ws2_32.lib iphlpapi.lib)
endif(MSVC)
#if(NOT MSVC)
#target_link_libraries(${PROJECT_NAME} mbedcrypto.so	mbedtls.so	mbedx509.so)
#endif(NOT MSVC)
