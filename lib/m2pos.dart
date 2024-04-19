import 'dart:typed_data';

import 'package:flutter/services.dart';

import 'm2pos_platform_interface.dart';

class M2pos {
  Future<String?> getPlatformVersion() {
    return M2posPlatform.instance.getPlatformVersion();
  }

//pid 774 vid 2245
  Future<String?> writeData(
      {required Uint8List data, required int pid, required int vid}) {
    return M2posPlatform.instance.writeData(
      data,
      pid,
      vid,
    );
  }

  Future<List?> readUsbPrinterList() {
    return M2posPlatform.instance.readUsbPrinterList();
  }

  void setMethodCallHandler(
      Future<dynamic> Function(MethodCall call)? handler) {
    M2posPlatform.instance.setMethodCallHandler(handler);
  }
}
