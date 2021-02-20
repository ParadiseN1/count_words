cmake_minimum_required(VERSION 3.17)
project(untitled4)


set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS filesystem locale system)

#find_library(LIBARCHIVE libarchive libArchive)
find_package(LibArchive REQUIRED)

add_executable(untitled4 main.cpp
        src/lib_archive_archive.cpp
        src/lib_archive.h
        libArchive/archive.h
        libArchive/archive_entry.h
        src/word_count.cpp
        src/word_count.h src/queue_t.h src/time.h)

target_include_directories(untitled4 PRIVATE ${Boost_INCLUDE_DIRS} ${LibArchive_INCLUDE_DIRS})

target_link_libraries(untitled4 ${Boost_LIBRARIES}  ${LibArchive_LIBRARIES} )


