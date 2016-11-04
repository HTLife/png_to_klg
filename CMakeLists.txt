cmake_minimum_required(VERSION 2.6.0)

find_package(ZLIB REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS date_time REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_ALL_DYN_LINK ON) #force dynamic linking for all libraries


add_executable(pngtoklg
               main.cpp)

target_link_libraries(pngtoklg
                      ${ZLIB_LIBRARY}
                      ${OpenCV_LIBS})
