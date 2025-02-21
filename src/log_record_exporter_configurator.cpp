#include <algorithm>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#if !defined(OTEL_LOG_EXPORTER_OTLP_GRPC_DISABLED)
#    include <opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_factory.h>
#endif

#if !defined(OTEL_LOG_EXPORTER_OTLP_HTTP_DISABLED)
#    include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter_factory.h>
#endif

#include "configurator_p.h"
#include "helpers.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace {

wwa::opentelemetry::log_record_exporter_t configure_otlp()
{
    using wwa::opentelemetry::helpers::get_otlp_protocol;
    const auto protocol =
        get_otlp_protocol("OTEL_EXPORTER_OTLP_LOGS_PROTOCOL", "OTEL_EXPORTER_OTLP_PROTOCOL", "http/protobuf");

#if !defined(OTEL_LOG_EXPORTER_OTLP_GRPC_DISABLED)
    if (protocol == "grpc") {
        return opentelemetry::exporter::otlp::OtlpGrpcLogRecordExporterFactory::Create();
    }
#endif

#if !defined(OTEL_LOG_EXPORTER_OTLP_HTTP_DISABLED)
    if (protocol.starts_with("http/")) {
        return opentelemetry::exporter::otlp::OtlpHttpLogRecordExporterFactory::Create();
    }

    INTERNAL_LOG_WARN(std::format("Unsupported OTLP logs protocol: <{}>, using http/protobuf", protocol));
    return opentelemetry::exporter::otlp::OtlpHttpLogRecordExporterFactory::Create();
#else
    INTERNAL_LOG_WARN(std::format("Unsupported OTLP logs protocol: <{}>", protocol));
    return nullptr;
#endif
}

wwa::opentelemetry::log_record_exporter_t
get_log_record_exporter(std::string_view name, wwa::opentelemetry::log_record_exporter_factory_t factory)
{
    if (name == "otlp") {
        return configure_otlp();
    }

    return factory != nullptr ? factory(name) : nullptr;
}

}  // namespace

namespace wwa::opentelemetry {

std::vector<log_record_exporter_t>
configure_log_record_exporters_from_environment(const log_record_exporter_config_t& opts)
{
    const auto exporters_env = helpers::get_env("OTEL_LOGS_EXPORTER");
    auto names               = helpers::split_and_trim(exporters_env);

    if (names.size() == 1 && names[0] == "none") {
        INTERNAL_LOG_WARN("OTEL_LOGS_EXPORTER contains \"none\". Logging will not be initialized.");
        return {};
    }

    if (names.empty()) {
        INTERNAL_LOG_WARN("OTEL_LOGS_EXPORTER is empty. Using default otlp exporter.");
        names = {"otlp"};
    }
    else if (std::ranges::find(names, "none") != names.end()) {
        INTERNAL_LOG_WARN(
            "OTEL_LOGS_EXPORTER contains \"none\" along with other exporters. Using default otlp exporter."
        );

        names = {"otlp"};
    }

    std::vector<wwa::opentelemetry::log_record_exporter_t> exporters;
    exporters.reserve(names.size());
    for (const auto& name : names) {
        if (auto exporter = get_log_record_exporter(name, opts.factory); exporter) {
            exporters.push_back(std::move(exporter));
        }
        else {
            INTERNAL_LOG_WARN(std::format("Unrecognized OTEL_LOGS_EXPORTER value: <{}>", name));
        }
    }

    return exporters;
}

}  // namespace wwa::opentelemetry
