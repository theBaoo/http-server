# Google 建议显式列出源文件, 而不是使用file
# 只有添加了CONFIGURE_DEPENDS参数，才能在源文件发生变化时重新生成项目
file(GLOB_RECURSE LIB_SRC_FILES CONFIGURE_DEPENDS *.cc)
list(FILTER LIB_SRC_FILES EXCLUDE REGEX ".*main.cc$")

# 未来可以考虑parser等模块单独成库
add_library(http_server_lib STATIC ${LIB_SRC_FILES})
# 手动添加include路径, 因为boost找不到openssl
target_include_directories(http_server_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(http_server_lib PUBLIC ${OPENSSL_LIBRARIES})
target_link_libraries(http_server_lib PUBLIC fmt::fmt)
target_link_libraries(http_server_lib PUBLIC Boost::system)
target_link_libraries(http_server_lib PUBLIC Boost::asio)
target_link_libraries(http_server_lib PUBLIC ZLIB::ZLIB)
# target_compile_definitions(http_server_lib PRIVATE ASIO_STANDALONE)

add_executable(http_server main.cc)
target_link_libraries(http_server PRIVATE http_server_lib)