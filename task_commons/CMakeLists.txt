


add_library(task_commons STATIC TrackCuts.cpp)
target_link_libraries(task_commons PUBLIC at_task)
target_include_directories(task_commons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})