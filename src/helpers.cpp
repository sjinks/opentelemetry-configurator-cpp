#include "helpers.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <format>
#include <stdexcept>

#include "configurator_p.h"

namespace {

const char* const spaces = " \t\n\r\f\v";

bool case_insensitive_compare(std::string_view str1, std::string_view str2)
{
    return str1.length() == str2.length() &&
           std::ranges::equal(str1, str2, [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

}  // namespace

namespace wwa::opentelemetry::helpers {

std::string_view trim(std::string_view s)
{
    const auto start = s.find_first_not_of(spaces);
    if (start == std::string_view::npos) {
        return {};
    }

    const auto end = s.find_last_not_of(spaces);
    return s.substr(start, end - start + 1);
}

std::vector<std::string_view> split_and_trim(std::string_view s)
{
    std::vector<std::string_view> result;
    std::string_view::size_type start = 0;
    std::string_view::size_type end   = 0;

    while ((end = s.find(',', start)) != std::string_view::npos) {
        auto token = s.substr(start, end - start);
        if (auto trimmed_token = trim(token); !trimmed_token.empty()) {
            result.push_back(trimmed_token);
        }

        start = end + 1;
    }

    auto token = s.substr(start);
    if (auto trimmed_token = trim(token); !trimmed_token.empty()) {
        result.push_back(trimmed_token);
    }

    return result;
}

std::string get_env(const char* name)
{
    const char* env = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    return env != nullptr ? env : std::string();
}

/**
 * @see https://opentelemetry.io/docs/specs/otel/configuration/sdk-environment-variables/#boolean-value
 *
 * > Any value that represents a Boolean MUST be set to true only by the case-insensitive string "true",
 * > meaning "True" or "TRUE" are also accepted, as true. An implementation MUST NOT extend this definition
 * > and define additional values that are interpreted as true. Any value not explicitly defined here as a true value,
 * > including unset and empty values, MUST be interpreted as false. If any value other than a true value,
 * > case-insensitive string "false", empty, or unset is used, a warning SHOULD be logged to inform users
 * > about the fallback to false being applied.
 */
bool get_env_bool(const char* name)
{
    const char* env = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (env == nullptr || *env == '\0') {
        return false;
    }

    if (case_insensitive_compare(env, "true")) {
        return true;
    }

    if (case_insensitive_compare(env, "false")) {
        return false;
    }

    INTERNAL_LOG_WARN(std::format("Environment variable <{}> has an invalid value <{}>, ignoring", name, env));

    return false;
}

/**
 * > If an implementation chooses to support an integer-valued environment variable,
 * > it SHOULD support nonnegative values between 0 and 2³¹ − 1 (inclusive).
 * > Individual SDKs MAY choose to support a larger range of values.
 * >
 * > For variables accepting a numeric value, if the user provides a value the implementation cannot parse,
 * > or which is outside the valid range for the configuration item, the implementation SHOULD generate
 * > a warning and gracefully ignore the setting, i.e., treat them as not set.
 *
 * @see https://opentelemetry.io/docs/specs/otel/configuration/sdk-environment-variables/#numeric-value
 */
unsigned long int get_env_long(const char* name, unsigned long int default_value)
{
    const char* env = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (env == nullptr || *env == '\0') {
        return default_value;
    }

    try {
        auto value = std::stol(env);
        if (value >= 0) {
            return static_cast<unsigned long int>(value);
        }

        INTERNAL_LOG_WARN(
            std::format("Environment variable <{}> has a value <{}>, outside the valid range, ignoring", name, env)
        );
    }
    catch (const std::invalid_argument&) {
        INTERNAL_LOG_WARN(std::format("Environment variable <{}> has an invalid value <{}>, ignoring", name, env));
    }
    catch (const std::out_of_range&) {
        INTERNAL_LOG_WARN(std::format("Environment variable <{}> has an invalid value <{}>, ignoring", name, env));
    }

    return default_value;
}

double get_env_double(const char* name, double default_value, double min, double max)
{
    const char* env = std::getenv(name);  // NOLINT(concurrency-mt-unsafe)
    if (env == nullptr || *env == '\0') {
        return default_value;
    }

    try {
        auto value = std::stod(env);
        if (value >= min && value <= max) {
            return value;
        }

        INTERNAL_LOG_WARN(
            std::format("Environment variable <{}> has a value <{}>, outside the valid range, ignoring", name, env)
        );
    }
    catch (const std::invalid_argument&) {
        INTERNAL_LOG_WARN(std::format("Environment variable <{}> has an invalid value <{}>, ignoring", name, env));
    }
    catch (const std::out_of_range&) {
        INTERNAL_LOG_WARN(std::format("Environment variable <{}> has an invalid value <{}>, ignoring", name, env));
    }

    return default_value;
}

std::string get_otlp_protocol(const char* env, const char* backup, const std::string& default_value)
{
    auto value = get_env(env);
    if (value.empty()) {
        value = get_env(backup);
    }

    return value.empty() ? default_value : value;
}

}  // namespace wwa::opentelemetry::helpers
