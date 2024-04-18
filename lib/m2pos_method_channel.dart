import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'm2pos_platform_interface.dart';

/// An implementation of [M2posPlatform] that uses method channels.
class MethodChannelM2pos extends M2posPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('m2pos');

  @override
  Future<String?> getPlatformVersion() async {
    final version =
        await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }

  @override
  Future<String?> writeData(Uint8List data, int pid, int vid) async {
    await methodChannel.invokeMethod<String>(
        'writeData', {'data': data, 'pid': pid, 'vid': vid});
    return "Success";
  }
}
