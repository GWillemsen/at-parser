if(IDF_TARGET) # -> in ESP-IDF build system
    set(COMPILE_ESP_IDF_VERSION 1)
    message("Value: ${COMPILE_ESP_IDF_VERSION}.")
else()
    set(COMPILE_ESP_IDF_VERSION 0)
    project(at-parser LANGUAGES C)
    cmake_minimum_required(VERSION 3.13.4)
    include(GNUInstallDirs)
    option(ENABLE_ATPARSER_TESTS "Enable building the doctest target exectuable." OFF)
endif()

set(PROJECT_DIR_NAME at-parser)

set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(SRC_FILES
    "${SRC_DIR}/at_parser.c"
)
set(INC_FILES
    "${INC_DIR}/at_parser/at_parser.h"
)

if(${COMPILE_ESP_IDF_VERSION}) # -> In ESP-IDF build system
    idf_component_register(COMPONENT_NAME at_parser
                            SRCS ${SRC_FILES} ${INC_FILES}
                            INCLUDE_DIRS "${INC_DIR}"
                        )
else()
    add_library(${PROJECT_NAME} STATIC ${SRC_FILES} ${INC_FILES})

    target_include_directories(${PROJECT_NAME}
        PUBLIC
        "$<BUILD_INTERFACE:${INC_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLLUDEDIR}>"
    )

    # Configure project to be exported
    # https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets
    install(TARGETS ${PROJECT_NAME}
        EXPORT "${PROJECT_NAME}Targets"
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(FILES ${INC_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_DIR_NAME})

    install(EXPORT "${PROJECT_NAME}Targets"
        FILE "${PROJECT_NAME}Targets.cmake"
        NAMESPACE "${PROJECT_NAME}::"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )

    # Generate the configuration files so that a find_package also works after a install.
    # https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html#exporting-targets
    include(CMakePackageConfigHelpers)
    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )

    if(${ENABLE_ATPARSER_TESTS})
        add_subdirectory(test)
    endif()
endif()