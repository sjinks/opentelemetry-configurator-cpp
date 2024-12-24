cmake_minimum_required(VERSION 3.25)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_MAINTAINER_MODE "Enable maintainer mode" OFF)
option(INSTALL_OTEL_CONFIGURATOR "Whether to install the OpenTelemetry Configurator" ON)

if(DEFINED VCPKG_TOOLCHAIN)
    option(WITH_OTLP_GRPC "Build with OTLP gRPC support" OFF)
    option(WITH_OTLP_HTTP "Build with OTLP HTTP support" ON)

    if (WITH_OTLP_GRPC)
        list(APPEND VCPKG_MANIFEST_FEATURES "grpc")
    endif()

    if (WITH_OTLP_HTTP)
        list(APPEND VCPKG_MANIFEST_FEATURES "http")
    endif()
endif()

set(EXPORT_COMPILE_COMMANDS ON)
project(wwa_opentelemetry_configurator VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(opentelemetry-cpp CONFIG REQUIRED)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-Wall" COMPILER_SUPPORTS_WALL)
check_cxx_compiler_flag("-Wextra" COMPILER_SUPPORTS_WEXTRA)
check_cxx_compiler_flag("-pedantic" COMPILER_SUPPORTS_PEDANTIC)

if(CMAKE_CONFIGURATION_TYPES)
    list(APPEND CMAKE_CONFIGURATION_TYPES "Coverage" "ASAN" "LSAN" "TSAN" "UBSAN")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
string(TOLOWER "${CMAKE_CONFIGURATION_TYPES}" CMAKE_CONFIGURATION_TYPES_LOWER)

string(REGEX MATCH "Clang" CMAKE_COMPILER_IS_CLANG "${CMAKE_CXX_COMPILER_ID}")
string(REGEX MATCH "GNU" CMAKE_COMPILER_IS_GNU "${CMAKE_CXX_COMPILER_ID}")

if(CMAKE_COMPILER_IS_GNU OR CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_CXX_FLAGS_ASAN "-O1 -g -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    set(CMAKE_CXX_FLAGS_TSAN "-O1 -g -fsanitize=thread -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_LSAN "-O1 -g -fsanitize=leak -fno-omit-frame-pointer -fno-optimize-sibling-calls")

    if(CMAKE_COMPILER_IS_GNU)
        set(CMAKE_CXX_FLAGS_COVERAGE "-Og -g --coverage -fprofile-abs-path")
        set(CMAKE_CXX_FLAGS_UBSAN "-O1 -g -fsanitize=undefined -fsanitize=float-divide-by-zero -fno-omit-frame-pointer")
    elseif(CMAKE_COMPILER_IS_CLANG)
        set(CMAKE_CXX_FLAGS_COVERAGE "-O1 -g --coverage")
        set(CMAKE_CXX_FLAGS_UBSAN "-O1 -g -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=integer -fsanitize=implicit-conversion -fsanitize=local-bounds -fsanitize=nullability -fno-omit-frame-pointer")
    endif()
endif()

if(ENABLE_MAINTAINER_MODE)
    if(CMAKE_COMPILER_IS_CLANG)
        set(CMAKE_CXX_FLAGS_MM -Weverything -Werror -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-pre-c++20-compat -Wno-padded)
    elseif(CMAKE_COMPILER_IS_GNU)
        set(CMAKE_CXX_FLAGS_MM -Wall -Wextra -Werror -pedantic)
    endif()
endif()

add_library(${PROJECT_NAME})
target_sources(
    ${PROJECT_NAME}
    PRIVATE
        src/batch_log_record_processor_configurator.cpp
        src/batch_span_processor_configurator.cpp
        src/configurator.cpp
        src/helpers.cpp
        src/id_generator_configurator.cpp
        src/internal_logging.cpp
        src/log_record_exporter_configurator.cpp
        src/logger_provider_configurator.cpp
        src/meter_provider_configurator.cpp
        src/metric_exporter_configurator.cpp
        src/periodic_exporting_metric_reader_configurator.cpp
        src/propagator_configurator.cpp
        src/resource_configurator.cpp
        src/span_exporter_configurator.cpp
        src/tracer_provider_configurator.cpp
        src/tracing_sampler_configurator.cpp
        src/utils.cpp
)

set(
    headers
        include/opentelemetry/configurator/wwa/export.h
        include/opentelemetry/configurator/wwa/configurator.h
        include/opentelemetry/configurator/wwa/utils.h
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        INTERFACE_COMPILE_FEATURES cxx_std_20
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
        PUBLIC_HEADER "${headers}"
)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WWA_OPENTELEMETRY_CONFIGURATOR_STATIC_DEFINE)
endif()

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC opentelemetry-cpp::sdk)

function(maybe_link target_name lib_name def_name)
    if (TARGET ${lib_name})
        target_link_libraries(${target_name} PUBLIC ${lib_name})
    else()
        target_compile_definitions(${target_name} PUBLIC ${def_name})
    endif()
endfunction()

maybe_link(${PROJECT_NAME} opentelemetry-cpp::otlp_grpc_exporter OTEL_SPAN_EXPORTER_OTLP_GRPC_DISABLED)
maybe_link(${PROJECT_NAME} opentelemetry-cpp::otlp_http_exporter OTEL_SPAN_EXPORTER_OTLP_HTTP_DISABLED)

maybe_link(${PROJECT_NAME} opentelemetry-cpp::otlp_grpc_log_record_exporter OTEL_LOG_EXPORTER_OTLP_GRPC_DISABLED)
maybe_link(${PROJECT_NAME} opentelemetry-cpp::otlp_http_log_record_exporter OTEL_LOG_EXPORTER_OTLP_HTTP_DISABLED)

maybe_link(${PROJECT_NAME} opentelemetry-cpp::otlp_grpc_metrics_exporter OTEL_METRICS_EXPORTER_OTLP_GRPC_DISABLED)
maybe_link(${PROJECT_NAME} opentelemetry-cpp::otlp_http_metric_exporter OTEL_METRICS_EXPORTER_OTLP_HTTP_DISABLED)

if(TARGET opentelemetry-cpp::otlp_http_exporter OR TARGET opentelemetry-cpp::otlp_http_log_record_exporter OR TARGET opentelemetry-cpp::otlp_http_metric_exporter)
    find_package(CURL REQUIRED)
    find_package(Protobuf REQUIRED)
    find_package(ZLIB REQUIRED)
    find_package(nlohmann_json REQUIRED)
endif()

if(TARGET opentelemetry-cpp::otlp_grpc_exporter OR TARGET opentelemetry-cpp::otlp_grpc_log_record_exporter OR TARGET opentelemetry-cpp::otlp_grpc_metrics_exporter)
    find_package(grpc REQUIRED)
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE
        $<$<BOOL:${COMPILER_SUPPORTS_WALL}>:-Wall>
        $<$<BOOL:${COMPILER_SUPPORTS_WEXTRA}>:-Wextra>
        $<$<BOOL:${COMPILER_SUPPORTS_PEDANTIC}>:-pedantic>
)

if(ENABLE_MAINTAINER_MODE)
    target_compile_options(${PROJECT_NAME} PRIVATE ${CMAKE_CXX_FLAGS_MM})
endif()

find_program(CLANG_FORMAT NAMES clang-format)
find_program(CLANG_TIDY NAMES clang-tidy)

if(CLANG_FORMAT OR CLANG_TIDY)
    file(GLOB_RECURSE ALL_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} LIST_DIRECTORIES OFF src/*.cpp)
    file(GLOB_RECURSE ALL_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} LIST_DIRECTORIES OFF src/*.h include/*.h)

    if(CLANG_FORMAT)
        add_custom_target(
            format
            COMMAND ${CLANG_FORMAT} --Wno-error=unknown -i -style=file ${ALL_SOURCE_FILES} ${ALL_HEADER_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()

    if(CLANG_TIDY)
        add_custom_target(
            tidy
            COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${ALL_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
endif()

if(INSTALL_OTEL_CONFIGURATOR)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS ${PROJECT_NAME}
        EXPORT "${PROJECT_NAME}-target"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/opentelemetry/configurator/wwa"
    )

    install(
        EXPORT "${PROJECT_NAME}-target"
        FILE "${PROJECT_NAME}-target.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        "${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )
endif()
