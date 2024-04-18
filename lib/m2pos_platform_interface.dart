import 'dart:typed_data';

import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'm2pos_method_channel.dart';

abstract class M2posPlatform extends PlatformInterface {
  /// Constructs a M2posPlatform.
  M2posPlatform() : super(token: _token);

  static final Object _token = Object();

  static M2posPlatform _instance = MethodChannelM2pos();

  /// The default instance of [M2posPlatform] to use.
  ///
  /// Defaults to [MethodChannelM2pos].
  static M2posPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [M2posPlatform] when
  /// they register themselves.
  static set instance(M2posPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }

  Future<String?> writeData(Uint8List data, int pid, int vid) {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
