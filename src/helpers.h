#ifndef B600D26D_6323_4934_BEEE_AE1EA6704D0D
#define B600D26D_6323_4934_BEEE_AE1EA6704D0D

/**
 * @file
 * @internal
 */

#include <string>
#include <string_view>
#include <vector>

std::string_view trim(std::string_view s);
std::vector<std::string_view> split_and_trim(std::string_view s);
std::string get_env(const char* name);
bool get_env_bool(const char* name);
unsigned long int get_env_long(const char* name, unsigned long int default_value);
double get_env_double(const char* name, double default_value, double min, double max);
std::string get_otlp_protocol(const char* env, const char* backup, const std::string& default_value);

#endif /* B600D26D_6323_4934_BEEE_AE1EA6704D0D */
