#include "include/m2pos/m2pos_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "m2pos_plugin.h"

void M2posPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  m2pos::M2posPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
