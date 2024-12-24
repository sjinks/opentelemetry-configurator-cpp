#include <opentelemetry/sdk/trace/random_id_generator_factory.h>

#include "configurator_p.h"
#include "opentelemetry/configurator/wwa/configurator.h"

namespace wwa::opentelemetry {

id_generator_t get_id_generator()
{
    return ::opentelemetry::sdk::trace::RandomIdGeneratorFactory::Create();
}

}  // namespace wwa::opentelemetry
