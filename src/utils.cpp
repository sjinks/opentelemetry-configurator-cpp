#include "opentelemetry/configurator/wwa/utils.h"

#include <cassert>
#include <string>
#include <typeinfo>

#include <opentelemetry/common/attribute_value.h>
#include <opentelemetry/nostd/string_view.h>

#if OPENTELEMETRY_VERSION_MAJOR == 1 && OPENTELEMETRY_VERSION_MINOR < 18
#    include <opentelemetry/sdk/resource/semantic_conventions.h>
#else
#    include <opentelemetry/semconv/exception_attributes.h>
#endif

namespace {

std::string get_exception_type(const std::exception* e)
{
    return typeid(*e).name();
}

}  // namespace

namespace wwa::opentelemetry {

void record_exception(const span_t& span, const std::exception* e)
{
#if OPENTELEMETRY_VERSION_MAJOR == 1 && OPENTELEMETRY_VERSION_MINOR < 18
    using ::opentelemetry::sdk::resource::SemanticConventions::kExceptionMessage;
    using ::opentelemetry::sdk::resource::SemanticConventions::kExceptionType;
#else
    using ::opentelemetry::semconv::exception::kExceptionMessage;
    using ::opentelemetry::semconv::exception::kExceptionType;
#endif

    assert(e != nullptr);
    assert(span.get() != nullptr);

    const auto type = get_exception_type(e);
    const std::initializer_list<std::pair<::opentelemetry::nostd::string_view, ::opentelemetry::common::AttributeValue>>
        attrs{{kExceptionType, type.c_str()}, {kExceptionMessage, e->what()}};

    span->AddEvent("exception", attrs);
}

}  // namespace wwa::opentelemetry
