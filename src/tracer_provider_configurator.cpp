#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/id_generator.h>
#include <opentelemetry/sdk/trace/sampler.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>

#include "configurator_p.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace wwa::opentelemetry {

tracer_provider_t configure_tracer_provider(tracer_provider_config_t&& opts)
{
    std::vector<span_exporter_t> exporters;
    if (opts.configure_exporters) {
        exporters = configure_span_exporters_from_environment(opts.span_exporter_config);
    }

    std::vector<span_processor_t> processors;
    processors.reserve(exporters.size() + opts.processors.size());
    for (auto&& exporter : exporters) {
        processors.push_back(get_batch_span_processor(std::move(exporter)));
    }

    for (auto&& processor : opts.processors) {
        processors.push_back(std::move(processor));
    }

    auto resource = std::holds_alternative<resource_config_t>(opts.resource)
                        ? configure_resource(std::get<resource_config_t>(opts.resource))
                        : std::get<::opentelemetry::sdk::resource::Resource>(opts.resource);

    auto sampler = std::holds_alternative<tracing_sampler_config_t>(opts.tracing_sampler)
                       ? configure_tracing_sampler_from_environment(
                             // NOLINTNEXTLINE(performance-move-const-arg)
                             std::move(std::get<tracing_sampler_config_t>(opts.tracing_sampler))
                         )
                       : std::move(std::get<tracing_sampler_t>(opts.tracing_sampler));

    auto id_generator = opts.id_generator ? std::move(opts.id_generator) : get_id_generator();

    return ::opentelemetry::sdk::trace::TracerProviderFactory::Create(
        std::move(processors), resource, std::move(sampler), std::move(id_generator)
    );
}

}  // namespace wwa::opentelemetry
