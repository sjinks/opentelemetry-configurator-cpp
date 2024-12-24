#include <chrono>
#include <utility>

#include <opentelemetry/sdk/logs/batch_log_record_processor_factory.h>
#include <opentelemetry/sdk/logs/batch_log_record_processor_options.h>
#include <opentelemetry/sdk/logs/exporter.h>
#include <opentelemetry/sdk/logs/processor.h>

#include "configurator_p.h"
#include "helpers.h"

namespace {

opentelemetry::sdk::logs::BatchLogRecordProcessorOptions get_batch_log_record_processor_options()
{
    opentelemetry::sdk::logs::BatchLogRecordProcessorOptions options;

    // OTEL_BLRP_EXPORT_TIMEOUT: Maximum allowed time (in milliseconds) to export data (30000): does not seem to be supported

    constexpr auto default_delay                 = 5000UL;
    constexpr auto default_max_queue_size        = 2048UL;
    constexpr auto default_max_export_batch_size = 512UL;

    options.schedule_delay_millis = std::chrono::milliseconds(get_env_long("OTEL_BLRP_SCHEDULE_DELAY", default_delay));
    options.max_queue_size        = get_env_long("OTEL_BLRP_MAX_QUEUE_SIZE", default_max_queue_size);
    options.max_export_batch_size = get_env_long("OTEL_BLRP_MAX_EXPORT_BATCH_SIZE", default_max_export_batch_size);

    if (options.max_export_batch_size > options.max_queue_size) {
        INTERNAL_LOG_WARN(
            "OTEL_BLRP_MAX_EXPORT_BATCH_SIZE is greater than OTEL_BLRP_MAX_QUEUE_SIZE. Using OTEL_BLRP_MAX_QUEUE_SIZE "
            "as the batch size."
        );

        options.max_export_batch_size = options.max_queue_size;
    }

    return options;
}

}  // namespace

namespace wwa::opentelemetry {

log_record_processor_t get_batch_log_record_processor(log_record_exporter_t&& exporter)
{
    static bool initialized = false;
    static ::opentelemetry::sdk::logs::BatchLogRecordProcessorOptions options;

    if (!initialized) {
        options     = get_batch_log_record_processor_options();
        initialized = true;
    }

    return ::opentelemetry::sdk::logs::BatchLogRecordProcessorFactory::Create(std::move(exporter), options);
}

}  // namespace wwa::opentelemetry
