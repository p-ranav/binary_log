if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_INSTALL_INCLUDEDIR include/binary_log CACHE PATH "")
endif()

# Project is configured with no languages, so tell GNUInstallDirs the lib dir
set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package binary_log)

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT binary_log_Development
)

install(
    TARGETS binary_log_binary_log
    EXPORT binary_logTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

# Allow package maintainers to freely override the path for the configs
set(
    binary_log_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(binary_log_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${binary_log_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT binary_log_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${binary_log_INSTALL_CMAKEDIR}"
    COMPONENT binary_log_Development
)

install(
    EXPORT binary_logTargets
    NAMESPACE binary_log::
    DESTINATION "${binary_log_INSTALL_CMAKEDIR}"
    COMPONENT binary_log_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
