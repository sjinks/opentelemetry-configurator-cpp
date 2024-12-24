#ifndef BFF437AD_B2D8_4090_B82F_84107BA5ED65
#define BFF437AD_B2D8_4090_B82F_84107BA5ED65

#include "opentelemetry/configurator/wwa/configurator.h"

#include <memory>
#include <string>

#include <opentelemetry/sdk/common/attribute_utils.h>
#include <opentelemetry/sdk/metrics/metric_reader.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/sdk/trace/processor.h>

namespace wwa::opentelemetry {

using metric_reader_t  = std::unique_ptr<::opentelemetry::sdk::metrics::MetricReader>;
using span_processor_t = std::unique_ptr<::opentelemetry::sdk::trace::SpanProcessor>;

log_record_processor_t get_batch_log_record_processor(log_record_exporter_t&& exporter);
span_processor_t get_batch_span_processor(span_exporter_t&& exporter);
id_generator_t get_id_generator();
metric_reader_t get_periodic_exporting_metric_reader(metric_exporter_t&& exporter);

void internal_log(
    ::opentelemetry::sdk::common::internal_log::LogLevel level, const std::string& message,
    const ::opentelemetry::sdk::common::AttributeMap& attributes, const char* file, int line
);

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTERNAL_LOG_WARN(message)                                                                     \
    ::wwa::opentelemetry::internal_log(                                                                \
        ::opentelemetry::sdk::common::internal_log::LogLevel::Warning, message, {}, __FILE__, __LINE__ \
    )

}  // namespace wwa::opentelemetry

#endif /* BFF437AD_B2D8_4090_B82F_84107BA5ED65 */
