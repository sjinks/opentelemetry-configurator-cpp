#include <algorithm>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#if !defined(OTEL_SPAN_EXPORTER_OTLP_GRPC_DISABLED)
#    include <opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h>
#endif

#if !defined(OTEL_SPAN_EXPORTER_OTLP_HTTP_DISABLED)
#    include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#endif

#include "configurator_p.h"
#include "helpers.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace {

wwa::opentelemetry::span_exporter_t configure_otlp()
{
    using wwa::opentelemetry::helpers::get_otlp_protocol;

    const auto protocol =
        get_otlp_protocol("OTEL_EXPORTER_OTLP_TRACES_PROTOCOL", "OTEL_EXPORTER_OTLP_PROTOCOL", "http/protobuf");

#if !defined(OTEL_SPAN_EXPORTER_OTLP_GRPC_DISABLED)
    if (protocol == "grpc") {
        return opentelemetry::exporter::otlp::OtlpGrpcExporterFactory::Create();
    }
#endif

#if !defined(OTEL_SPAN_EXPORTER_OTLP_HTTP_DISABLED)
    if (protocol.starts_with("http/")) {
        return opentelemetry::exporter::otlp::OtlpHttpExporterFactory::Create();
    }

    INTERNAL_LOG_WARN(std::format("Unsupported OTLP traces protocol: <{}>, using http/protobuf", protocol));
    return opentelemetry::exporter::otlp::OtlpHttpExporterFactory::Create();
#else
    INTERNAL_LOG_WARN(std::format("Unsupported OTLP traces protocol: <{}>", protocol));
    return nullptr;
#endif
}

wwa::opentelemetry::span_exporter_t
get_span_exporter(std::string_view name, wwa::opentelemetry::span_exporter_factory_t factory)
{
    if (name == "otlp") {
        return configure_otlp();
    }

    return factory != nullptr ? factory(name) : nullptr;
}

}  // namespace

namespace wwa::opentelemetry {

std::vector<span_exporter_t> configure_span_exporters_from_environment(const span_exporter_config_t& opts)
{
    const auto exporters_env = helpers::get_env("OTEL_TRACES_EXPORTER");
    auto names               = helpers::split_and_trim(exporters_env);

    if (names.size() == 1 && names[0] == "none") {
        INTERNAL_LOG_WARN("OTEL_TRACES_EXPORTER contains \"none\". Tracing will not be initialized.");
        return {};
    }

    if (names.empty()) {
        INTERNAL_LOG_WARN("OTEL_TRACES_EXPORTER is empty. Using default otlp exporter.");
        names = {"otlp"};
    }
    else if (std::ranges::find(names, "none") != names.end()) {
        INTERNAL_LOG_WARN(
            "OTEL_TRACES_EXPORTER contains \"none\" along with other exporters. Using default otlp exporter."
        );

        names = {"otlp"};
    }

    std::vector<wwa::opentelemetry::span_exporter_t> exporters;
    exporters.reserve(names.size());
    for (const auto& name : names) {
        if (auto exporter = get_span_exporter(name, opts.factory); exporter) {
            exporters.push_back(std::move(exporter));
        }
        else {
            INTERNAL_LOG_WARN(std::format("Unrecognized OTEL_TRACES_EXPORTER value: <{}>", name));
        }
    }

    return exporters;
}

}  // namespace wwa::opentelemetry
