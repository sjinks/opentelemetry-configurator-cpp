#include <array>
#include <format>
#include <string_view>
#include <type_traits>
#include <utility>

#include <opentelemetry/sdk/common/global_log_handler.h>

#include "configurator_p.h"
#include "helpers.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace {

using namespace std::literals;
constexpr std::array<std::pair<std::string_view, opentelemetry::sdk::common::internal_log::LogLevel>, 5> log_levels{
    {{"none"sv, opentelemetry::sdk::common::internal_log::LogLevel::None},
     {"error"sv, opentelemetry::sdk::common::internal_log::LogLevel::Error},
     {"warning"sv, opentelemetry::sdk::common::internal_log::LogLevel::Warning},
     {"info"sv, opentelemetry::sdk::common::internal_log::LogLevel::Info},
     {"debug"sv, opentelemetry::sdk::common::internal_log::LogLevel::Debug}}
};

}  // namespace

namespace wwa::opentelemetry {

void internal_log(
    ::opentelemetry::sdk::common::internal_log::LogLevel level, const std::string& message,
    const ::opentelemetry::sdk::common::AttributeMap& attributes, const char* file, int line
)
{
    if (std::is_constant_evaluated() && OTEL_INTERNAL_LOG_LEVEL < static_cast<int>(level)) {
        return;
    }

    using ::opentelemetry::sdk::common::internal_log::GlobalLogHandler;
    using ::opentelemetry::sdk::common::internal_log::LogHandler;

    if (level > GlobalLogHandler::GetLogLevel()) {
        return;
    }

    if (const auto& handler = GlobalLogHandler::GetLogHandler(); handler) {
        handler->Handle(level, file, line, message.c_str(), attributes);
    }
}

void configure_internal_logging_from_environment()
{
    auto level = ::opentelemetry::sdk::common::internal_log::LogLevel::Info;

    auto log_level = get_env("OTEL_LOG_LEVEL");
    if (!log_level.empty()) {
        bool found = false;
        for (const auto& [name, value] : log_levels) {
            if (log_level == name) {
                level = value;
                found = true;
                break;
            }
        }

        if (!found) {
            INTERNAL_LOG_WARN(
                std::format("Environment variable <OTEL_LOG_LEVEL> has an unknown value <{}>, ignoring", log_level)
            );
        }
    }

    ::opentelemetry::sdk::common::internal_log::GlobalLogHandler::SetLogLevel(level);
}

}  // namespace wwa::opentelemetry
