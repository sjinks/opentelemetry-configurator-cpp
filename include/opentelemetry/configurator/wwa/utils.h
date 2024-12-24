#ifndef B9313F7F_096F_47E8_9AAA_B3828659B876
#define B9313F7F_096F_47E8_9AAA_B3828659B876

#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_metadata.h>
#include <opentelemetry/trace/span_startoptions.h>
#include <opentelemetry/trace/tracer.h>

namespace wwa::opentelemetry {

using span_t = ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Span>;

namespace detail {

struct span_ender {
    span_ender(span_t span) : m_span(std::move(span)) {}
    ~span_ender() { m_span->End(); }

    span_ender(const span_ender&)            = delete;
    span_ender(span_ender&&)                 = delete;
    span_ender& operator=(const span_ender&) = delete;
    span_ender& operator=(span_ender&&)      = delete;

private:
    span_t m_span;
};

}  // namespace detail

void record_exception(const span_t& span, const std::exception* e);

template<typename F, typename... Args>
inline std::invoke_result_t<std::decay_t<F>, Args...> startActiveSpan(
    const ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer>& tracer,
    ::opentelemetry::nostd::string_view name, const ::opentelemetry::trace::StartSpanOptions& opts, F&& f,
    Args&&... args
)
{
    auto span  = tracer->StartSpan(name, opts);
    auto scope = ::opentelemetry::trace::Tracer::WithActiveSpan(span);
    const detail::span_ender ender(span);
    try {
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }
    catch (const std::exception& e) {
        record_exception(span, &e);
        span->SetStatus(::opentelemetry::trace::StatusCode::kError, e.what());
        throw;
    }
}

template<typename F, typename... Args>
inline std::invoke_result_t<std::decay_t<F>, Args...> startActiveSpan(
    const ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer>& tracer,
    ::opentelemetry::nostd::string_view name, F&& f, Args&&... args
)
{
    return startActiveSpan(tracer, name, {}, std::forward<F>(f), std::forward<Args>(args)...);
}

template<typename F, typename... Args>
inline std::invoke_result_t<std::decay_t<F>, Args...> startSpan(
    const ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer>& tracer,
    ::opentelemetry::nostd::string_view name, const ::opentelemetry::trace::StartSpanOptions& opts, F&& f,
    Args&&... args
)
{
    auto span = tracer->StartSpan(name, opts);
    const detail::span_ender ender(span);
    try {
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    }
    catch (const std::exception& e) {
        record_exception(span, &e);
        span->SetStatus(::opentelemetry::trace::StatusCode::kError, e.what());
        throw;
    }
}

template<typename F, typename... Args>
inline std::invoke_result_t<std::decay_t<F>, Args...> startSpan(
    ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer> tracer, ::opentelemetry::nostd::string_view name,
    F&& f, Args&&... args
)
{
    return startSpan(tracer, name, {}, std::forward<F>(f), std::forward<Args>(args)...);
}

}  // namespace wwa::opentelemetry

#endif /* B9313F7F_096F_47E8_9AAA_B3828659B876 */
