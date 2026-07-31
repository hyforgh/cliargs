enable_testing()

aux_source_directory(. test_source_list)
foreach(test_source ${test_source_list})
    get_filename_component(test_name ${test_source} NAME_WE)
    cliarg_add_inner_bin_target(${test_name})
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
