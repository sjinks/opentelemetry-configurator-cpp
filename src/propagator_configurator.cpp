#include <array>
#include <format>
#include <memory>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <opentelemetry/baggage/propagation/baggage_propagator.h>
#include <opentelemetry/context/propagation/composite_propagator.h>
#include <opentelemetry/context/propagation/text_map_propagator.h>
#include <opentelemetry/trace/propagation/b3_propagator.h>
#include <opentelemetry/trace/propagation/http_trace_context.h>

#include "configurator_p.h"
#include "helpers.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace {

using propagator_t = std::unique_ptr<opentelemetry::context::propagation::TextMapPropagator>;
using creator_t    = propagator_t (*)();

using namespace opentelemetry::baggage::propagation;
using namespace opentelemetry::trace::propagation;
using namespace std::literals;

constexpr std::array<std::pair<std::string_view, creator_t>, 4> creators{{
    {"tracecontext"sv, []() -> propagator_t { return std::make_unique<HttpTraceContext>(); }},
    {"baggage"sv, []() -> propagator_t { return std::make_unique<BaggagePropagator>(); }},
    {"b3"sv, []() -> propagator_t { return std::make_unique<B3Propagator>(); }},
    {"b3multi"sv, []() -> propagator_t { return std::make_unique<B3PropagatorMultiHeader>(); }},
}};

propagator_t default_factory_impl(std::string_view)
{
    return {};
}

propagator_t find_propagator(std::string_view name, wwa::opentelemetry::propagator_factory_t factory)
{
    for (const auto& [key, value] : creators) {
        if (name == key) {
            return value();
        }

        if (auto propagator = factory(name); propagator) {
            return propagator;
        }
    }

    return {};
}

std::vector<std::unique_ptr<opentelemetry::context::propagation::TextMapPropagator>>
get_propagators(const std::unordered_set<std::string_view>&& names, wwa::opentelemetry::propagator_factory_t factory)
{
    if (factory == nullptr) {
        factory = default_factory_impl;
    }

    std::vector<propagator_t> propagators;
    propagators.reserve(names.size());
    for (const auto& name : names) {
        if (auto propagator = find_propagator(name, factory); propagator) {
            propagators.push_back(std::move(propagator));
        }
        else {
            INTERNAL_LOG_WARN(std::format("Unrecognized OTEL_PROPAGATORS value: <{}>", name));
        }
    }

    return propagators;
}

std::unordered_set<std::string_view> get_propagator_names()
{
    auto env  = get_env("OTEL_PROPAGATORS");
    auto list = split_and_trim(env);
    if (list.empty()) {
        return {"tracecontext", "baggage"};
    }

    std::unordered_set<std::string_view> names(list.begin(), list.end());
    if (names.size() == 1 && names.count("none") == 1) {
        return {};
    }

    if (names.count("none") == 1) {
        INTERNAL_LOG_WARN("OTEL_PROPAGATORS contains <none> along with other propagators. Using default propagators.");
        return {"tracecontext", "baggage"};
    }

    return names;
}

}  // namespace

namespace wwa::opentelemetry {

propagator_t configure_propagators_from_environment(const propagator_config_t& opts)
{
    auto propagators = get_propagators(get_propagator_names(), opts.factory);
    return propagator_t(new ::opentelemetry::context::propagation::CompositePropagator(std::move(propagators)));
}

}  // namespace wwa::opentelemetry
