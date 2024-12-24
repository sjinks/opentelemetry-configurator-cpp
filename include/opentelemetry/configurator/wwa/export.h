#ifndef CBD9FDD4_ACA6_452E_A9A5_5017962C3980
#define CBD9FDD4_ACA6_452E_A9A5_5017962C3980

#ifdef WWA_OPENTELEMETRY_CONFIGURATOR_STATIC_DEFINE
#    define WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT
#    define WWA_OPENTELEMETRY_CONFIGURATOR_NO_EXPORT
#else
#    ifdef wwa_opentelemetry_configurator_EXPORTS
/* We are building this library; export */
#        if defined _WIN32 || defined __CYGWIN__
#            define WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT __declspec(dllexport)
#            define WWA_OPENTELEMETRY_CONFIGURATOR_NO_EXPORT
#        else
#            define WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT    [[gnu::visibility("default")]]
#            define WWA_OPENTELEMETRY_CONFIGURATOR_NO_EXPORT [[gnu::visibility("hidden")]]
#        endif
#    else
/* We are using this library; import */
#        if defined _WIN32 || defined __CYGWIN__
#            define WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT __declspec(dllimport)
#            define WWA_OPENTELEMETRY_CONFIGURATOR_NO_EXPORT
#        else
#            define WWA_OPENTELEMETRY_CONFIGURATOR_EXPORT    [[gnu::visibility("default")]]
#            define WWA_OPENTELEMETRY_CONFIGURATOR_NO_EXPORT [[gnu::visibility("hidden")]]
#        endif
#    endif
#endif

#endif /* CBD9FDD4_ACA6_452E_A9A5_5017962C3980 */
