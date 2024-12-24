#include <chrono>
#include <utility>

#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_options.h>
#include <opentelemetry/sdk/metrics/push_metric_exporter.h>

#include "configurator_p.h"
#include "helpers.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace wwa::opentelemetry {

/**
 * @see https://opentelemetry.io/docs/specs/otel/configuration/sdk-environment-variables/#periodic-exporting-metricreader
 */
metric_reader_t get_periodic_exporting_metric_reader(metric_exporter_t&& exporter)
{
    static bool initialized = false;
    static ::opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions options;

    if (!initialized) {
        constexpr auto default_interval = 60'000UL;
        constexpr auto default_timeout  = 30'000UL;

        options.export_interval_millis =
            std::chrono::milliseconds(get_env_long("OTEL_METRIC_EXPORT_INTERVAL", default_interval));
        options.export_timeout_millis =
            std::chrono::milliseconds(get_env_long("OTEL_METRIC_EXPORT_TIMEOUT", default_timeout));
        initialized = true;
    }

    return ::opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), options);
}

}  // namespace wwa::opentelemetry
