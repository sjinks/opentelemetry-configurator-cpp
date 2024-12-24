#include <array>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include <opentelemetry/sdk/trace/sampler.h>
#include <opentelemetry/sdk/trace/samplers/always_off_factory.h>
#include <opentelemetry/sdk/trace/samplers/always_on_factory.h>
#include <opentelemetry/sdk/trace/samplers/parent_factory.h>
#include <opentelemetry/sdk/trace/samplers/trace_id_ratio_factory.h>

#include "configurator_p.h"
#include "helpers.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace {

using namespace std::literals;
using namespace opentelemetry::sdk::trace;
using sampler_creator_t = wwa::opentelemetry::tracing_sampler_t (*)();

auto create_traceidratio_sampler()
{
    const auto ratio = get_env_double("OTEL_TRACES_SAMPLER_ARG", 1.0, 0.0, 1.0);
    return opentelemetry::sdk::trace::TraceIdRatioBasedSamplerFactory::Create(ratio);
}

auto create_parentbased_alwayson_sampler()
{
    return ParentBasedSamplerFactory::Create(AlwaysOnSamplerFactory::Create());
}

auto create_parentbased_alwaysoff_sampler()
{
    return ParentBasedSamplerFactory::Create(AlwaysOffSamplerFactory::Create());
}

auto create_parentbased_traceidratio_sampler()
{
    return ParentBasedSamplerFactory::Create(create_traceidratio_sampler());
}

constexpr std::array<std::pair<std::string_view, sampler_creator_t>, 6> samplers{
    {{"always_on"sv, &AlwaysOnSamplerFactory::Create},
     {"always_off"sv, &AlwaysOffSamplerFactory::Create},
     {"traceidratio"sv, &create_traceidratio_sampler},
     {"parentbased_always_on"sv, &create_parentbased_alwayson_sampler},
     {"parentbased_always_off"sv, &create_parentbased_alwaysoff_sampler},
     {"parentbased_traceidratio"sv, &create_parentbased_traceidratio_sampler}}
};

wwa::opentelemetry::tracing_sampler_t default_factory_impl(std::string_view)
{
    return {};
}

}  // namespace

namespace wwa::opentelemetry {

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
tracing_sampler_t configure_tracing_sampler_from_environment(tracing_sampler_config_t&& opts)
{
    auto factory = opts.factory != nullptr ? opts.factory : default_factory_impl;

    if (const auto name = get_env("OTEL_TRACES_SAMPLER"); !name.empty()) {
        for (const auto& [key, value] : samplers) {
            if (name == key) {
                return value();
            }

            if (auto sampler = factory(name); sampler) {
                return sampler;
            }
        }

        INTERNAL_LOG_WARN(std::format("Unrecognized OTEL_TRACES_SAMPLER value: <{}>", name));
    }

    return ParentBasedSamplerFactory::Create(AlwaysOnSamplerFactory::Create());
}

}  // namespace wwa::opentelemetry
