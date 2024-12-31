#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>
#include <opentelemetry/sdk/metrics/metric_reader.h>
#include <opentelemetry/sdk/metrics/view/view_registry.h>
#include <opentelemetry/sdk/metrics/view/view_registry_factory.h>
#include <opentelemetry/sdk/resource/resource.h>

#include "configurator_p.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace wwa::opentelemetry {

meter_provider_t configure_meter_provider(meter_provider_config_t&& opts)
{
    std::vector<metric_exporter_t> exporters;
    if (opts.configure_exporters) {
        exporters = configure_metric_exporters_from_environment(opts.metric_exporter_config);
    }

    auto resource = std::holds_alternative<resource_config_t>(opts.resource)
                        ? configure_resource(std::get<resource_config_t>(opts.resource))
                        : std::get<::opentelemetry::sdk::resource::Resource>(opts.resource);

    auto view_registry = opts.view_registry != nullptr ? std::move(opts.view_registry)
                                                       : ::opentelemetry::sdk::metrics::ViewRegistryFactory::Create();

    auto provider = ::opentelemetry::sdk::metrics::MeterProviderFactory::Create(std::move(view_registry), resource);

    for (auto&& exporter : exporters) {
        provider->AddMetricReader(get_periodic_exporting_metric_reader(std::move(exporter)));
    }

    return provider;
}

}  // namespace wwa::opentelemetry
