prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
includedir=${prefix}/@CMAKE_INSTALL_INCLUDEDIR@
libdir=${prefix}/@CMAKE_INSTALL_LIBDIR@

Name: Seastar
Url: http://seastar-project.org
Description: Advanced C++ framework for high-performance server applications on modern hardware.
Version: @PROJECT_VERSION@

# Platform dependencies.
dl_libs=${CMAKE_DL_LIBS}
rt_libs=-l$<TARGET_PROPERTY:rt::rt,INTERFACE_LINK_LIBRARIES>

# Dependencies of dependencies.
boost_system_libs=$<TARGET_LINKER_FILE:Boost::system>

# Dependencies.
boost_cflags=-I$<JOIN:$<TARGET_PROPERTY:Boost::boost,INTERFACE_INCLUDE_DIRECTORIES>, -I>
boost_filesystem_libs=$<TARGET_LINKER_FILE:Boost::filesystem>
boost_program_options_libs=$<TARGET_LINKER_FILE:Boost::program_options>
boost_thread_libs=${boost_system_libs} $<TARGET_LINKER_FILE:Boost::thread>
c_ares_cflags=-I$<JOIN:$<TARGET_PROPERTY:c-ares::c-ares,INTERFACE_INCLUDE_DIRECTORIES>, -I>
c_ares_libs=$<TARGET_LINKER_FILE:c-ares::c-ares>
cryptopp_cflags=-I$<JOIN:$<TARGET_PROPERTY:cryptopp::cryptopp,INTERFACE_INCLUDE_DIRECTORIES>, -I>
cryptopp_libs=$<TARGET_LINKER_FILE:cryptopp::cryptopp>
fmt_cflags=-I$<JOIN:$<TARGET_PROPERTY:fmt::fmt,INTERFACE_INCLUDE_DIRECTORIES>, -I>
fmt_libs=$<TARGET_LINKER_FILE:fmt::fmt>
lksctp_tools_cflags=-I$<JOIN:$<TARGET_PROPERTY:lksctp-tools::lksctp-tools,INTERFACE_INCLUDE_DIRECTORIES>, -I>
lksctp_tools_libs=$<TARGET_LINKER_FILE:lksctp-tools::lksctp-tools>
sanitizers_cflags=$<JOIN:@Sanitizers_COMPILER_OPTIONS@, >
stdfilesystem_libs=-l$<TARGET_PROPERTY:StdFilesystem::filesystem,INTERFACE_LINK_LIBRARIES>

# Us.
seastar_cflags=-I{includedir} $<JOIN:$<TARGET_PROPERTY:seastar,INTERFACE_COMPILE_OPTIONS>, > -D$<JOIN:$<TARGET_PROPERTY:seastar,INTERFACE_COMPILE_DEFINITIONS>, -D>
seastar_libs=$<TARGET_FILE:seastar>

Requires: liblz4 >= 1.8.0
Requires.private: gnutls >= 3.5.18, protobuf >= 3.3.1, hwloc >= 1.11.5, yaml-cpp >= 0.5.3
Conflicts:
Cflags: ${boost_cflags}  ${c_ares_cflags} ${cryptopp_cflags} ${fmt_cflags} ${lksctp_tools_cflags} ${sanitizers_cflags} ${seastar_cflags}
Libs: ${boost_program_options_libs} ${boost_thread_libs} ${c_ares_libs} ${cryptopp_libs} ${fmt_libs}  ${seastar_libs}
Libs.private: ${dl_libs} ${rt_libs} ${boost_filesystem_libs} ${boost_thread_libs} ${lksctp_tools_libs} ${stdfilesystem_libs}
