list(APPEND TESTCASES
        test_topological_sort_printer
        test_cycles_printer
        test_tree_printer
        test_dependants_printer
        test_graphviz_printer
        test_graphml_printer
        test_plantuml_printer
        test_util)

if(WITH_JSON)
    list(APPEND TESTCASES test_json_printer)
endif(WITH_JSON)

foreach(TESTCASE IN LISTS TESTCASES)
    add_executable(
            ${TESTCASE}
            $<TARGET_OBJECTS:lib-clang-include-graph>
            ${TESTCASE}.cc)

    target_include_directories(${TESTCASE} PUBLIC ${Boost_INCLUDE_DIRS})

    target_compile_options(
            ${TESTCASE}
            PRIVATE $<$<COMPILE_LANGUAGE:CXX>:
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
            -Wno-unused-parameter
            -Wno-unused-variable
            -Wno-attributes
            -Wno-nonnull
            -Wno-deprecated-enum-enum-conversion
            ${CUSTOM_COMPILE_OPTIONS}>
            $<$<CXX_COMPILER_ID:MSVC>:/MP
            /MD
            /W1
            /bigobj
            /wd4624>>)

    target_link_libraries(${TESTCASE}
            ${LIBCLANG_LIBRARIES}
            Boost::filesystem
            Boost::graph
            Boost::program_options
            Boost::unit_test_framework
            Boost::log)

    if(WITH_JSON)
        target_link_libraries(${TESTCASE} Boost::json)
    endif(WITH_JSON)

    add_test(${TESTCASE} ${TESTCASE})
endforeach()