#include "opentelemetry/configurator/wwa/utils.h"

#include <cassert>
#include <initializer_list>
#include <string>
#include <typeinfo>
#include <utility>

#include <opentelemetry/common/attribute_value.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/sdk/resource/semantic_conventions.h>

namespace {

std::string get_exception_type(const std::exception* e)
{
    return typeid(*e).name();
}

}  // namespace

namespace wwa::opentelemetry {

void record_exception(const span_t& span, const std::exception* e)
{
    assert(e != nullptr);
    assert(span.get() != nullptr);

    const auto type = get_exception_type(e);
    const std::initializer_list<std::pair<::opentelemetry::nostd::string_view, ::opentelemetry::common::AttributeValue>>
        attrs{
            {::opentelemetry::sdk::resource::SemanticConventions::kExceptionType, type.c_str()},
            {::opentelemetry::sdk::resource::SemanticConventions::kExceptionMessage, e->what()}
        };

    span->AddEvent("exception", attrs);
}

}  // namespace wwa::opentelemetry
