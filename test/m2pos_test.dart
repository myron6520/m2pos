import 'package:flutter_test/flutter_test.dart';
import 'package:m2pos/m2pos.dart';
import 'package:m2pos/m2pos_platform_interface.dart';
import 'package:m2pos/m2pos_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockM2posPlatform
    with MockPlatformInterfaceMixin
    implements M2posPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final M2posPlatform initialPlatform = M2posPlatform.instance;

  test('$MethodChannelM2pos is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelM2pos>());
  });

  test('getPlatformVersion', () async {
    M2pos m2posPlugin = M2pos();
    MockM2posPlatform fakePlatform = MockM2posPlatform();
    M2posPlatform.instance = fakePlatform;

    expect(await m2posPlugin.getPlatformVersion(), '42');
  });
}
