cmake_minimum_required(VERSION 3.17)
project(ATCentralityTask)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# AT Task skeleton
FetchContent_Declare(
        ATTaskSkeleton
        GIT_REPOSITORY https://github.com/eugene274/AnalysisTreeTaskSkeleton.git
        GIT_TAG        0.9.1
)
FetchContent_MakeAvailable(ATTaskSkeleton)

# AT Task skeleton
FetchContent_Declare(
        Centrality
        GIT_REPOSITORY git@git.cbm.gsi.de:pwg-c2f/analysis/centrality.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(Centrality)

add_subdirectory(task_commons)
add_subdirectory(task_centrality)
add_subdirectory(task_psd_modules_90)
add_subdirectory(qa_centrality)
