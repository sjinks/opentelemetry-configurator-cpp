#include <string>
#include <unordered_map>

#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/resource/resource_detector.h>

#if OPENTELEMETRY_VERSION_MAJOR == 1 && OPENTELEMETRY_VERSION_MINOR < 18
#    include <opentelemetry/sdk/resource/semantic_conventions.h>
#else
#    include <opentelemetry/semconv/service_attributes.h>
#endif

#include "opentelemetry/configurator/wwa/configurator.h"

namespace {

/**
 * Unlike Node.JS SDK, C++ SDK does not overwrite the attributes of the left-hand side with the right-hand side.
 */
void merge_attributes(
    opentelemetry::sdk::resource::ResourceAttributes& lhs, const opentelemetry::sdk::resource::ResourceAttributes& rhs
)
{
    for (const auto& [key, value] : rhs) {
        lhs[key] = value;
    }
}

}  // namespace

namespace wwa::opentelemetry {

::opentelemetry::sdk::resource::Resource configure_resource(const resource_config_t& opts)
{
#if OPENTELEMETRY_VERSION_MAJOR == 1 && OPENTELEMETRY_VERSION_MINOR < 18
    using ::opentelemetry::sdk::resource::SemanticConventions::kServiceName;
#else
    using ::opentelemetry::semconv::service::kServiceName;
#endif
    ::opentelemetry::sdk::resource::ResourceAttributes attributes;
    std::string schema_url = opts.schema_url;
    for (const auto& detector : opts.detectors) {
        const auto res = detector->Detect();
        merge_attributes(attributes, res.GetAttributes());
        if (!res.GetSchemaURL().empty()) {
            schema_url = res.GetSchemaURL();
        }
    }

    merge_attributes(attributes, opts.attrs);

    if (!opts.service_name.empty()) {
        attributes[kServiceName] = std::string(opts.service_name);
    }

    return ::opentelemetry::sdk::resource::Resource::Create(attributes, schema_url);
}

}  // namespace wwa::opentelemetry
