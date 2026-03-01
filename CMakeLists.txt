cmake_minimum_required(VERSION 3.20)

project(limit_order_book
    VERSION 0.1.0
    DESCRIPTION "High-performance C++20 limit order book"
    LANGUAGES CXX
)

# ----------------------------
# C++ Standard
# ----------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------
# Default Build Type
# ----------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ----------------------------
# Compiler Warnings
# ----------------------------
add_library(project_warnings INTERFACE)

target_compile_options(project_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
)

# ----------------------------
# Core Library
# ----------------------------
add_library(lob_core
    src/order_book.cpp
    src/matching_engine.cpp
    src/paper_trader.cpp
)

target_include_directories(lob_core
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(lob_core
    PUBLIC
    project_warnings
)

# ----------------------------
# Tests
# ----------------------------
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.3
)

FetchContent_MakeAvailable(Catch2)

enable_testing()

# Order Book test
add_executable(test_order_book_container tests/test_order_book_container.cpp)
target_link_libraries(test_order_book_container PRIVATE lob_core Catch2::Catch2WithMain)
add_test(NAME order_book_container COMMAND test_order_book_container)

# Matching Engine test
add_executable(test_matching_engine tests/test_matching_engine.cpp)
target_link_libraries(test_matching_engine PRIVATE lob_core Catch2::Catch2WithMain)
add_test(NAME matching_engine COMMAND test_matching_engine)

# Paper Trader test
add_executable(test_paper_trader tests/test_paper_trader.cpp)
target_link_libraries(test_paper_trader PRIVATE lob_core Catch2::Catch2WithMain)
add_test(NAME paper_trader COMMAND test_paper_trader)

include(CTest)
include(Catch)

# ----------------------------
# Optional: Install Rules (professional touch)
# ----------------------------
install(TARGETS lob_core
    EXPORT lobTargets
    DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)