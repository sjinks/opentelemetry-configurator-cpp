#include "opentelemetry/configurator/wwa/configurator.h"

#include <utility>
#include <variant>

#include <opentelemetry/context/propagation/global_propagator.h>
#include <opentelemetry/logs/logger_provider.h>
#include <opentelemetry/logs/provider.h>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer_provider.h>

#include "helpers.h"

namespace wwa::opentelemetry {

void configure_opentelemetry(opentelemetry_configuration_t&& opts)
{
    if (get_env_bool("OTEL_SDK_DISABLED")) {
        return;
    }

    // 1. Configure internal logging
    configure_internal_logging_from_environment();

    // 2. Configure Resource, as it will be used by all providers
    auto resource = std::holds_alternative<resource_config_t>(opts.resource)
                        ? configure_resource(std::get<resource_config_t>(opts.resource))
                        : std::get<::opentelemetry::sdk::resource::Resource>(opts.resource);

    // 3. Configure TracerProvider
    tracer_provider_config_t tracer_provider_config;
    tracer_provider_config.configure_exporters = true;
    tracer_provider_config.resource            = resource;
    tracer_provider_config.span_exporter_config =
        std::move(opts.span_exporter_config);  // NOLINT(performance-move-const-arg)
    tracer_provider_config.processors      = std::move(opts.span_processors);
    tracer_provider_config.tracing_sampler = std::move(opts.tracing_sampler);
    tracer_provider_config.id_generator    = std::move(opts.id_generator);
    auto tracer_provider                   = configure_tracer_provider(std::move(tracer_provider_config));
    auto api_tracer_provider =
        ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::TracerProvider>(tracer_provider.release());
    ::opentelemetry::trace::Provider::SetTracerProvider(api_tracer_provider);

    // 4. Configure Propagator
    auto propagator = std::holds_alternative<propagator_config_t>(opts.propagator)
                          ? configure_propagators_from_environment(std::get<propagator_config_t>(opts.propagator))
                          : std::get<propagator_t>(opts.propagator);
    ::opentelemetry::context::propagation::GlobalTextMapPropagator::SetGlobalPropagator(propagator);

    // 5. Configure MeterProvider
    meter_provider_config_t meter_provider_config;
    meter_provider_config.configure_exporters = true;
    meter_provider_config.metric_exporter_config =
        std::move(opts.metric_exporter_config);  // NOLINT(performance-move-const-arg)
    meter_provider_config.view_registry = std::move(opts.view_registry);
    meter_provider_config.resource      = resource;
    auto meter_provider                 = configure_meter_provider(std::move(meter_provider_config));
    auto api_meter_provider =
        ::opentelemetry::nostd::shared_ptr<::opentelemetry::metrics::MeterProvider>(meter_provider.release());
    ::opentelemetry::metrics::Provider::SetMeterProvider(api_meter_provider);

    // 6. Configure LoggerProvider
    logger_provider_config_t logger_provider_config;
    logger_provider_config.configure_exporters = true;
    logger_provider_config.log_record_exporter_config =
        std::move(opts.log_record_exporter_config);  // NOLINT(performance-move-const-arg)
    logger_provider_config.processors = std::move(opts.log_processors);
    logger_provider_config.resource   = resource;
    auto logger_provider              = configure_logger_provider(std::move(logger_provider_config));
    auto api_logger_provider =
        ::opentelemetry::nostd::shared_ptr<::opentelemetry::logs::LoggerProvider>(logger_provider.release());
    ::opentelemetry::logs::Provider::SetLoggerProvider(api_logger_provider);
}

}  // namespace wwa::opentelemetry
