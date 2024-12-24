#ifndef D5C70C0F_B39A_4F49_8994_A8DF90B94923
#define D5C70C0F_B39A_4F49_8994_A8DF90B94923

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/sdk/logs/exporter.h>
#include <opentelemetry/sdk/logs/logger_provider.h>
#include <opentelemetry/sdk/logs/processor.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/push_metric_exporter.h>
#include <opentelemetry/sdk/metrics/view/view_registry.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/resource/resource_detector.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/sdk/trace/id_generator.h>
#include <opentelemetry/sdk/trace/processor.h>
#include <opentelemetry/sdk/trace/sampler.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>

#include "export.h"

namespace wwa::opentelemetry {

using id_generator_t = std::unique_ptr<::opentelemetry::sdk::trace::IdGenerator>;

using log_record_exporter_t         = std::unique_ptr<::opentelemetry::sdk::logs::LogRecordExporter>;
using log_record_exporter_factory_t = log_record_exporter_t (*)(std::string_view);

using log_record_processor_t = std::unique_ptr<::opentelemetry::sdk::logs::LogRecordProcessor>;
using logger_provider_t      = std::unique_ptr<::opentelemetry::sdk::logs::LoggerProvider>;

using meter_provider_t = std::unique_ptr<::opentelemetry::sdk::metrics::MeterProvider>;

using metric_exporter_t         = std::unique_ptr<::opentelemetry::sdk::metrics::PushMetricExporter>;
using metric_exporter_factory_t = metric_exporter_t (*)(std::string_view);

using tracing_sampler_t         = std::unique_ptr<::opentelemetry::sdk::trace::Sampler>;
using tracing_sampler_factory_t = tracing_sampler_t (*)(std::string_view);

using propagator_t = ::opentelemetry::nostd::shared_ptr<::opentelemetry::context::propagation::TextMapPropagator>;
using propagator_factory_t =
    std::unique_ptr<::opentelemetry::context::propagation::TextMapPropagator> (*)(std::string_view);

using span_exporter_t         = std::unique_ptr<::opentelemetry::sdk::trace::SpanExporter>;
using span_exporter_factory_t = span_exporter_t (*)(std::string_view);

using span_processor_t  = std::unique_ptr<::opentelemetry::sdk::trace::SpanProcessor>;
using tracer_provider_t = std::unique_ptr<::opentelemetry::sdk::trace::TracerProvider>;

using view_registry_t = std::unique_ptr<::opentelemetry::sdk::metrics::ViewRegistry>;

struct resource_config_t {
    std::string_view service_name;
    ::opentelemetry::sdk::resource::ResourceAttributes attrs;
    std::string schema_url;
    std::vector<std::shared_ptr<::opentelemetry::sdk::resource::ResourceDetector>> detectors;
};

struct log_record_exporter_config_t {
    log_record_exporter_factory_t factory;
};

struct logger_provider_config_t {
    log_record_exporter_config_t log_record_exporter_config = {};
    std::vector<log_record_processor_t> processors;
    std::variant<resource_config_t, ::opentelemetry::sdk::resource::Resource> resource;
    bool configure_exporters = true;
};

struct metric_exporter_config_t {
    metric_exporter_factory_t factory;
};

struct meter_provider_config_t {
    metric_exporter_config_t metric_exporter_config = {};
    view_registry_t view_registry;
    std::variant<resource_config_t, ::opentelemetry::sdk::resource::Resource> resource;
    bool configure_exporters = true;
};

struct tracing_sampler_config_t {
    tracing_sampler_factory_t factory;
};

struct propagator_config_t {
    propagator_factory_t factory;
};

struct span_exporter_config_t {
    span_exporter_factory_t factory;
};

struct tracer_provider_config_t {
    span_exporter_config_t span_exporter_config = {};
    std::vector<span_processor_t> processors;
    std::variant<resource_config_t, ::opentelemetry::sdk::resource::Resource> resource;
    std::variant<tracing_sampler_config_t, tracing_sampler_t> tracing_sampler;
    id_generator_t id_generator;
    bool configure_exporters = true;
};

struct opentelemetry_configuration_t {
    // Global
    std::variant<resource_config_t, ::opentelemetry::sdk::resource::Resource> resource;

    // TracerProvider
    span_exporter_config_t span_exporter_config;
    std::vector<span_processor_t> span_processors;
    std::variant<tracing_sampler_config_t, tracing_sampler_t> tracing_sampler;
    id_generator_t id_generator;

    // Propagator
    std::variant<propagator_config_t, propagator_t> propagator;

    // MeterProvider
    metric_exporter_config_t metric_exporter_config;
    view_registry_t view_registry;

    // LoggerProvider
    log_record_exporter_config_t log_record_exporter_config;
    std::vector<log_record_processor_t> log_processors;
};

WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT void configure_internal_logging_from_environment();
WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT std::vector<log_record_exporter_t>
configure_log_record_exporters_from_environment(const log_record_exporter_config_t& opts);
WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT std::vector<metric_exporter_t>
configure_metric_exporters_from_environment(const metric_exporter_config_t& opts);
WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT std::vector<span_exporter_t>
configure_span_exporters_from_environment(const span_exporter_config_t& opts);
WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT propagator_t
configure_propagators_from_environment(const propagator_config_t& opts);
WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT tracing_sampler_t
configure_tracing_sampler_from_environment(tracing_sampler_config_t&& opts);

WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT logger_provider_t configure_logger_provider(logger_provider_config_t&& opts);
WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT meter_provider_t configure_meter_provider(meter_provider_config_t&& opts);
WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT ::opentelemetry::sdk::resource::Resource
configure_resource(const resource_config_t& opts);
WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT tracer_provider_t configure_tracer_provider(tracer_provider_config_t&& opts);

WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT void configure_opentelemetry(opentelemetry_configuration_t&& opts);

}  // namespace wwa::opentelemetry

#endif /* D5C70C0F_B39A_4F49_8994_A8DF90B94923 */
