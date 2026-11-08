cmake_minimum_required(VERSION 3.15)
project(DungPoly)

if(MSVC)
        add_compile_options(/W4)
else()
        add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-unused-private-field)
        add_compile_options(-fsanitize=undefined,thread)
        add_link_options(-fsanitize=undefined,thread)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
include_directories(include)

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(lib)

file(GLOB_RECURSE sources "src/*")
file(GLOB_RECURSE includes "include/*")

set(core_sources
        src/Board.cpp
        src/Cards.cpp
        src/Field.cpp
        src/LocalGame.cpp
        src/PlayerRequesrs.cpp
        src/Random.cpp
        src/threads/BoardModel.cpp
)
foreach(core_source IN LISTS core_sources)
        list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/${core_source}")
endforeach()
list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/src/web/GameEngine.cpp")

add_library(dungpoly_core STATIC ${core_sources})
target_include_directories(dungpoly_core PUBLIC include lib/sockpp/include)
target_link_libraries(dungpoly_core PUBLIC Threads::Threads)

add_executable(${PROJECT_NAME} ${sources} ${includes} ${ProtoSources} ${ProtoHeaders})
target_link_libraries(${PROJECT_NAME} PRIVATE dungpoly_core)

add_executable(DungPolyWebEngine src/web/GameEngine.cpp)
target_link_libraries(DungPolyWebEngine PRIVATE dungpoly_core)
set_target_properties(DungPolyWebEngine PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/web-engine/$<CONFIG>"
)

set (resources "${CMAKE_SOURCE_DIR}/resources")
set (resources_target "${CMAKE_CURRENT_BINARY_DIR}/resources")
if(WIN32)
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${resources} ${resources_target}
                COMMENT "copy resources folder from ${resources} => ${resources_target}"
        )
else()
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E create_symlink ${resources} ${resources_target}
                COMMENT "symbolic link resources folder from ${resources} => ${resources_target}"
        )
endif()

if(WIN32)
        set(sockpp_target sockpp-static)
else()
        set(sockpp_target sockpp)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads sfml-graphics ${PROTOBUF_LIBRARIES} ${sockpp_target})