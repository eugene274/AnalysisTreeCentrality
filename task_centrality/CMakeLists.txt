
add_executable(CentralityPsd CentralityPsd.cpp CentralityPsd.h)
target_link_libraries(CentralityPsd PUBLIC
        task_commons
        at_task_main
        Centrality)
