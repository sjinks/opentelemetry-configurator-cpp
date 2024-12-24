#include <utility>
#include <variant>
#include <vector>

#include <opentelemetry/sdk/logs/logger_provider_factory.h>
#include <opentelemetry/sdk/resource/resource.h>

#include "configurator_p.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace wwa::opentelemetry {

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
logger_provider_t configure_logger_provider(logger_provider_config_t&& opts)
{
    std::vector<log_record_exporter_t> exporters;
    if (opts.configure_exporters) {
        exporters = configure_log_record_exporters_from_environment(opts.log_record_exporter_config);
    }

    std::vector<log_record_processor_t> processors;
    processors.reserve(exporters.size() + opts.processors.size());
    for (auto&& exporter : exporters) {
        processors.push_back(get_batch_log_record_processor(std::move(exporter)));
    }

    for (auto&& processor : opts.processors) {
        processors.push_back(std::move(processor));
    }

    auto resource = std::holds_alternative<resource_config_t>(opts.resource)
                        ? configure_resource(std::get<resource_config_t>(opts.resource))
                        : std::get<::opentelemetry::sdk::resource::Resource>(opts.resource);

    return ::opentelemetry::sdk::logs::LoggerProviderFactory::Create(std::move(processors), resource);
}

}  // namespace wwa::opentelemetry
