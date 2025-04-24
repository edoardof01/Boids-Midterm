cmake_minimum_required(VERSION 3.16)
project(midtermParallel)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


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

# Flags utili per VTune
foreach(target SeqHeadless ParHeadless ParSOAHeadless SequentialGrid ParallelGrid SpeedUpCalculation SpeedUpCalculation_threads)
    target_compile_options(${target} PRIVATE -g -fno-omit-frame-pointer -fopenmp)
endforeach()

# Target custom per avviare VTune e generare report automatico
add_custom_target(run_vtune
        COMMAND ${CMAKE_COMMAND} -E remove_directory vtune_hotspots
        COMMAND env OMP_DYNAMIC=FALSE OMP_PLACES=cores OMP_PROC_BIND=close vtune -collect hotspots -result-dir vtune_hotspots -- ./SpeedUpCalculation
        COMMAND vtune -report hotspots -result-dir vtune_hotspots -format text -report-output vtune_hotspots/functions.txt
        DEPENDS SpeedUpCalculation
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Profiling con VTune e OMP_* settings + report dettagliato"
)





