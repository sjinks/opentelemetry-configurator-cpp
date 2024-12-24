get_filename_component(WWA_OTEL_CONFIGURATOR_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

list(APPEND CMAKE_MODULE_PATH ${WWA_OTEL_CONFIGURATOR_CMAKE_DIR})

include(CMakeFindDependencyMacro)
find_dependency(opentelemetry-cpp)

if(NOT TARGET wwa_opentelemetry_configurator)
    include("${WWA_OTEL_CONFIGURATOR_CMAKE_DIR}/wwa_opentelemetry_configurator-target.cmake")
    add_library(wwa::opentelemetry::configurator ALIAS wwa_opentelemetry_configurator)
endif()
