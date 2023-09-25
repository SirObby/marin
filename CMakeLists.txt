
cmake_minimum_required(VERSION 3.15)

project(marin VERSION 1.0 DESCRIPTION "A discord bot for interfacing with the MyAnimeList API")

if(UNIX AND NOT APPLE) 
	set(CMAKE_THREAD_LIBS_INIT "-lpthread")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	set(CMAKE_HAVE_THREADS_LIBRARY 1)
	set(CMAKE_USE_WIN32_THREADS_INIT 0)
	set(CMAKE_USE_PTHREADS_INIT 1)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

add_subdirectory(libs/fmt)
add_subdirectory(libs/DPP)

file(GLOB_RECURSE source_list "src/*.cpp" "src/commands/*.cpp" "src/*.hpp")
add_executable(${PROJECT_NAME} ${source_list})

target_compile_definitions(${PROJECT_NAME} PUBLIC DPP_CORO)
find_package(PostgreSQL REQUIRED)
find_package(fmt)


target_link_libraries(${PROJECT_NAME}
    dpp
	pqxx
	fmt::fmt
)

# Specify includes
target_include_directories(${PROJECT_NAME} PRIVATE
   libs/DPP/include
)


# Set C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20 # or 20 if you want something more recent
    CXX_STANDARD_REQUIRED ON
 )