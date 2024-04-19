#ifndef FLUTTER_PLUGIN_M2POS_PLUGIN_H_
#define FLUTTER_PLUGIN_M2POS_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <map>
#include <vector>

#include <memory>

namespace m2pos {

class M2posPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  M2posPlugin();

  virtual ~M2posPlugin();

  // Disallow copy and assign.
  M2posPlugin(const M2posPlugin&) = delete;
  M2posPlugin& operator=(const M2posPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
    std::vector<std::map<std::string, std::string>> ReadUsbPrinterList();
};

}  // namespace m2pos

#endif  // FLUTTER_PLUGIN_M2POS_PLUGIN_H_
