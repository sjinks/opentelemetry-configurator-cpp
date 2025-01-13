#ifndef B9313F7F_096F_47E8_9AAA_B3828659B876
#define B9313F7F_096F_47E8_9AAA_B3828659B876

#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

#include <opentelemetry/logs/logger.h>
#include <opentelemetry/logs/provider.h>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_metadata.h>
#include <opentelemetry/trace/span_startoptions.h>
#include <opentelemetry/trace/tracer.h>

namespace wwa::opentelemetry {

using span_t = ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Span>;

struct span_ender {
    span_ender(span_t span) : m_span(std::move(span)) {}
    ~span_ender() { this->m_span->End(); }

    span_ender(const span_ender&)            = delete;
    span_ender(span_ender&&)                 = delete;
    span_ender& operator=(const span_ender&) = delete;
    span_ender& operator=(span_ender&&)      = delete;

private:
    span_t m_span;
};

void record_exception(const span_t& span, const std::exception* e);

template<typename F, typename... Args>
inline std::invoke_result_t<std::decay_t<F>, span_t, Args...> startActiveSpan(
    const ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer>& tracer,
    ::opentelemetry::nostd::string_view name, const ::opentelemetry::trace::StartSpanOptions& opts, F&& f,
    Args&&... args
)
{
    auto span  = tracer->StartSpan(name, opts);
    auto scope = ::opentelemetry::trace::Tracer::WithActiveSpan(span);
    const span_ender ender(span);
    try {
        return std::invoke(std::forward<F>(f), span, std::forward<Args>(args)...);
    }
    catch (const std::exception& e) {
        record_exception(span, &e);
        span->SetStatus(::opentelemetry::trace::StatusCode::kError, e.what());
        throw;
    }
}

template<typename F, typename... Args>
inline std::invoke_result_t<std::decay_t<F>, span_t, Args...> startActiveSpan(
    const ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer>& tracer,
    ::opentelemetry::nostd::string_view name, F&& f, Args&&... args
)
{
    return startActiveSpan(tracer, name, {}, std::forward<F>(f), std::forward<Args>(args)...);
}

template<typename F, typename... Args>
inline std::invoke_result_t<std::decay_t<F>, span_t, Args...> startSpan(
    const ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer>& tracer,
    ::opentelemetry::nostd::string_view name, const ::opentelemetry::trace::StartSpanOptions& opts, F&& f,
    Args&&... args
)
{
    auto span = tracer->StartSpan(name, opts);
    const span_ender ender(span);
    try {
        return std::invoke(std::forward<F>(f), span, std::forward<Args>(args)...);
    }
    catch (const std::exception& e) {
        record_exception(span, &e);
        span->SetStatus(::opentelemetry::trace::StatusCode::kError, e.what());
        throw;
    }
}

template<typename F, typename... Args>
inline std::invoke_result_t<std::decay_t<F>, span_t, Args...> startSpan(
    ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer> tracer, ::opentelemetry::nostd::string_view name,
    F&& f, Args&&... args
)
{
    return startSpan(tracer, name, {}, std::forward<F>(f), std::forward<Args>(args)...);
}

static inline ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer>
get_tracer(::opentelemetry::nostd::string_view name, ::opentelemetry::nostd::string_view version = "")
{
    return ::opentelemetry::trace::Provider::GetTracerProvider()->GetTracer(name, version);
}

static inline ::opentelemetry::nostd::shared_ptr<::opentelemetry::metrics::Meter> get_meter(
    ::opentelemetry::nostd::string_view name, ::opentelemetry::nostd::string_view version = "",
    ::opentelemetry::nostd::string_view schema_url = ""
)
{
    return ::opentelemetry::metrics::Provider::GetMeterProvider()->GetMeter(name, version, schema_url);
}

static inline ::opentelemetry::nostd::shared_ptr<::opentelemetry::logs::Logger> get_logger(
    ::opentelemetry::nostd::string_view logger_name, ::opentelemetry::nostd::string_view library_name = "",
    ::opentelemetry::nostd::string_view library_version = ""
)
{
    return ::opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger(logger_name, library_name, library_version);
}

}  // namespace wwa::opentelemetry

#endif /* B9313F7F_096F_47E8_9AAA_B3828659B876 */
