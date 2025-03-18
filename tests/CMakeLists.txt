file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/**/*.cc)

# 显式添加 main_test
# add_executable(main_test ${CMAKE_SOURCE_DIR}/tests/main_test.cc)
# target_link_libraries(main_test PRIVATE regex_engine_lib gtest gtest_main)
# add_test(NAME main_test COMMAND main_test)

# 为每个测试文件创建可执行文件
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    
    # 假设测试文件命名如 parser_test.cc，对应源文件 parser.cc
    string(REPLACE "_test" "" MODULE_NAME ${TEST_NAME})
    
    # 检查对应的源文件是否存在
    set(SRC_FILE ${CMAKE_SOURCE_DIR}/src/${MODULE_NAME}/${MODULE_NAME}.cc)
    if(EXISTS ${SRC_FILE})
        add_executable(${TEST_NAME} ${TEST_FILE} ${SRC_FILE})
    else()
        add_executable(${TEST_NAME} ${TEST_FILE})
    endif()
    
    # 链接必要的库
    target_link_libraries(${TEST_NAME} PRIVATE http_server_lib gtest gtest_main)
    
    # 添加测试
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()