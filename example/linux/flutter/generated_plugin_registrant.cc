//
//  Generated file. Do not edit.
//

// clang-format off

#include "generated_plugin_registrant.h"

#include <m2pos/m2pos_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) m2pos_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "M2posPlugin");
  m2pos_plugin_register_with_registrar(m2pos_registrar);
}
