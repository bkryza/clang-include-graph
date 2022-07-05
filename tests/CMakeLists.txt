add_executable(
        test_topological_sort_printer
        $<TARGET_OBJECTS:lib-clang-include-graph>
        test_topological_sort_printer.cc)

target_include_directories(test_topological_sort_printer PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(test_topological_sort_printer
        Boost::filesystem Boost::graph Boost::program_options Boost::unit_test_framework)

add_test(test_topological_sort_printer test_topological_sort_printer)