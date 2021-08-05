import 'dart:async';
import 'package:flutter/services.dart';
import 'dart:isolate';
import 'dart:ffi'; // ffi libary
import 'package:ffi/ffi.dart'; // ffi package for utility functions like toNativeUtf8()

class Work extends Opaque {}

class WaterpumpFfiPlugin {
  static final WaterpumpFfiPlugin _instance = WaterpumpFfiPlugin._internal();

  factory WaterpumpFfiPlugin() {
    return _instance;
  }

  WaterpumpFfiPlugin._internal();

  late final DynamicLibrary _opcuaLib;
  late final int _nativePort;
  late final _executeCallback;
  late final _registerSetCallbackSendPort;
  late final _startBackgroundThread;
  late final _registerCallbackByNodeId;

  void init(String libPath) {
    _openDynamicLibrary(libPath);
    _initializeApi();
    _initializePort();
    _lookupFunctions();
    _registerCallbacks();
  }

  void startBackgroundThread() => _startBackgroundThread();

  get currentTimeStream => WaterpumpFfiPlugin._currentTimeController.stream;
  get tank1PercentFilledStream =>
      WaterpumpFfiPlugin._tank1PercentFilledController.stream;
  get tank2PercentFilledStream =>
      WaterpumpFfiPlugin._tank2PercentFilledController.stream;
  get tank2TargetPercentStream =>
      WaterpumpFfiPlugin._tank2TargetPercentController.stream;

  static const MethodChannel _channel =
      const MethodChannel('waterpump_ffi_plugin');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  void _openDynamicLibrary(String libPath) {
    print('Trying to open $libPath');
    _opcuaLib = DynamicLibrary.open(libPath);
    print('Opened $libPath');
  }

  void _initializeApi() {
    final initializeApi = _opcuaLib.lookupFunction<
        IntPtr Function(Pointer<Void>),
        int Function(Pointer<Void>)>('InitDartApiDL');

    if (initializeApi(NativeApi.initializeApiDLData) != 0) {
      throw ('InitDartApiDL failed');
    }
  }

  void _requestExecuteCallback(dynamic message) {
    final int workAdress = message;
    final work = Pointer<Work>.fromAddress(workAdress);
    _executeCallback(work);
  }

  void _initializePort() {
    final interactiveCppRequests = ReceivePort()
      ..listen(_requestExecuteCallback);
    _nativePort = interactiveCppRequests.sendPort.nativePort;
  }

  void _lookupFunctions() {
    _registerSetCallbackSendPort = _opcuaLib.lookupFunction<
        Void Function(Int64 sendPort),
        void Function(int sendPort)>('SetCallbackendPort');

    _executeCallback = _opcuaLib.lookupFunction<Void Function(Pointer<Work>),
        void Function(Pointer<Work>)>('ExecuteCallback');

    _startBackgroundThread =
        _opcuaLib.lookupFunction<Void Function(), void Function()>(
            'StartBackgroundThread');

    _registerCallbackByNodeId = _opcuaLib.lookupFunction<
        Void Function(Pointer<Utf8> nodeId,
            Pointer<NativeFunction<Void Function(IntPtr)>> functionPointer),
        void Function(
            Pointer<Utf8> nodeId,
            Pointer<NativeFunction<Void Function(IntPtr)>>
                functionPointer)>('RegisterCallbackByNodeId');

    print('Function lookups done');
  }

  // Dart streams to send data to the app
  static final _currentTimeController = StreamController<String>.broadcast();
  static final _tank1PercentFilledController =
      StreamController<double>.broadcast();
  static final _tank2PercentFilledController =
      StreamController<double>.broadcast();
  static final _tank2TargetPercentController =
      StreamController<double>.broadcast();

  // Register callback functions
  void _registerCallbacks() {
    _registerSetCallbackSendPort(_nativePort);

    final callbacksByNodeIds = {
      'CurrentTime':
          Pointer.fromFunction<Void Function(IntPtr)>(_currentTimeCallback),
      'Machine.Tank1.PercentFilled':
          Pointer.fromFunction<Void Function(IntPtr)>(
              _tank1PercentFilledCallback),
      'Machine.Tank2.PercentFilled':
          Pointer.fromFunction<Void Function(IntPtr)>(
              _tank2PercentFilledCallback),
      'Machine.Tank2.TargetPercent':
          Pointer.fromFunction<Void Function(IntPtr)>(_tank2TargetPercent)
    };

    callbacksByNodeIds.forEach((nodeId, callback) {
      Pointer<Utf8> cstring = nodeId.toNativeUtf8();
      _registerCallbackByNodeId(cstring, callback);
      calloc.free(cstring);
    });

    print('Callbacks registered');
  }

  // Dart callback functions that will be communicated to the C library
  static void _currentTimeCallback(int ptr) {
    Pointer<Utf8> utf8Ptr = Pointer.fromAddress(ptr);
    final dartString = utf8Ptr.toDartString();
    print('_currentTimeCallback($dartString)');
    malloc.free(utf8Ptr);
    _currentTimeController.add(dartString);
  }

  static void _tank1PercentFilledCallback(int ptr) {
    Pointer<Double> doublePtr = Pointer.fromAddress(ptr);

    double value = doublePtr.value;
    malloc.free(doublePtr);

    _tank1PercentFilledController.add(value);
    print('tank1PercentFilled: $value');
  }

  static void _tank2PercentFilledCallback(int ptr) {
    Pointer<Double> doublePtr = Pointer.fromAddress(ptr);

    double value = doublePtr.value;
    malloc.free(doublePtr);

    _tank2PercentFilledController.add(value);
    print('tank2PercentFilled: $value');
  }

  static void _tank2TargetPercent(int ptr) {
    Pointer<Double> doublePtr = Pointer.fromAddress(ptr);

    double value = doublePtr.value;
    malloc.free(doublePtr);
    _tank2TargetPercentController.add(value);
    print('tank2TargetPercent: $value');
  }
}
