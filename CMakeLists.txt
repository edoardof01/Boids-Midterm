cmake_minimum_required(VERSION 3.16)
project(midtermParallel)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#[[set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fopenmp -g -O1")]]

# Trova OpenMP
find_package(OpenMP REQUIRED)

# ---------------------------------------------------------------------------
# 1) Versione headless sequenziale
# ---------------------------------------------------------------------------
add_executable(SeqHeadless
        boids_sequential.cpp
        BoidsCommon.hpp
        BoidsUpdate.hpp
)
target_link_libraries(SeqHeadless PRIVATE OpenMP::OpenMP_CXX)

# ---------------------------------------------------------------------------
# 2) Versione headless parallela
# ---------------------------------------------------------------------------
add_executable(ParHeadless
        boids_parallel.cpp
        BoidsCommon.hpp
        BoidsUpdate.hpp
)
target_link_libraries(ParHeadless PRIVATE OpenMP::OpenMP_CXX)

# ---------------------------------------------------------------------------
# 3) Versione parallela SOA
# ---------------------------------------------------------------------------
add_executable(ParSOAHeadless
        boids_parallel_SOA.cpp
        BoidsSOA.hpp
        BoidsUpdateSOA.hpp
)
target_include_directories(ParSOAHeadless PRIVATE ${CMAKE_SOURCE_DIR})
if(OpenMP_CXX_FOUND)
    target_link_libraries(ParSOAHeadless PRIVATE OpenMP::OpenMP_CXX)
endif()

# ---------------------------------------------------------------------------
# 4) Versione Grid sequenziale
# ---------------------------------------------------------------------------
add_executable(SequentialGrid
        boids_sequential_grid.cpp
        BoidsGrid.hpp
        BoidsCommon.hpp
)
target_link_libraries(SequentialGrid PRIVATE OpenMP::OpenMP_CXX)

# ---------------------------------------------------------------------------
# 5) Versione Grid parallela
# ---------------------------------------------------------------------------
add_executable(ParallelGrid
        boids_parallel_grid.cpp
        BoidsGrid.hpp
        BoidsCommon.hpp
        SpeedUpCalculation_threads.cpp
)
target_include_directories(ParallelGrid PRIVATE ${CMAKE_SOURCE_DIR})
if(OpenMP_CXX_FOUND)
    target_link_libraries(ParallelGrid PRIVATE OpenMP::OpenMP_CXX)
endif()


# ---------------------------------------------------------------------------
# 6) Speedup Calculation
# ---------------------------------------------------------------------------
add_executable(SpeedUpCalculation
        SpeedUpCalculation.cpp
)
target_link_libraries(SpeedUpCalculation PRIVATE OpenMP::OpenMP_CXX)

# ---------------------------------------------------------------------------
# 7) Speedup Calculation threads
# ---------------------------------------------------------------------------

add_executable(SpeedUpCalculation_threads
        SpeedUpCalculation_threads.cpp
)
target_link_libraries(SpeedUpCalculation_threads PRIVATE OpenMP::OpenMP_CXX)

