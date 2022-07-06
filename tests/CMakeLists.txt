
list(APPEND TESTCASES
        test_topological_sort_printer
        test_cycles_printer
        test_tree_printer)

foreach(TESTCASE IN LISTS TESTCASES)
    add_executable(
            ${TESTCASE}
            $<TARGET_OBJECTS:lib-clang-include-graph>
            ${TESTCASE}.cc)

    target_include_directories(${TESTCASE} PUBLIC ${Boost_INCLUDE_DIRS})

    target_link_libraries(${TESTCASE}
            Boost::filesystem Boost::graph Boost::program_options Boost::unit_test_framework)

    add_test(${TESTCASE} ${TESTCASE})
endforeach()