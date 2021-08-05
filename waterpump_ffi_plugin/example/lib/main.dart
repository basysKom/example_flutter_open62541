import 'package:flutter/material.dart';
import 'dart:async';
import 'dart:io';

import 'package:flutter/services.dart';
import 'package:waterpump_ffi_plugin/waterpump_ffi_plugin.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';

  String _curretServerTime = '';
  double _tank1PercentFilled = 0.0;
  double _tank2PercentFilled = 0.0;
  double _tank2TargetPercent = 0.0;

  late WaterpumpFfiPlugin waterpumpFfiPlugin;

  @override
  void initState() {
    super.initState();
    initPlatformState();

    // TODO: Finding the library needs to be implemented properly
    print('Directory.current.path: ${Directory.current.path}');
    final libName = Platform.isAndroid
        ? 'libwaterpump_ffi_lib.so'
        : 'libwaterpump_ffi_plugin_plugin.so';

    var libPath = (Platform.isAndroid ? '' : 'lib/') + libName;
    final libExists = new File(libPath).existsSync();
    if (!Platform.isAndroid && !libExists) {
      print('$libPath does not exist. Assuming app was started from terminal');
      libPath = 'build/linux/x64/debug/bundle/lib/$libName';
      print('Trying $libPath instead');
    }

    waterpumpFfiPlugin = WaterpumpFfiPlugin();
    waterpumpFfiPlugin.init(libPath);

    waterpumpFfiPlugin.currentTimeStream.listen((currentTime) {
      setState(() {
        _curretServerTime = currentTime;
      });
    });

    waterpumpFfiPlugin.tank1PercentFilledStream.listen((filled) {
      setState(() {
        _tank1PercentFilled = filled;
      });
    });

    waterpumpFfiPlugin.tank2PercentFilledStream.listen((filled) {
      setState(() {
        _tank2PercentFilled = filled;
      });
    });

    waterpumpFfiPlugin.tank2TargetPercentStream.listen((filled) {
      setState(() {
        _tank2TargetPercent = filled;
      });
    });

    waterpumpFfiPlugin.startBackgroundThread();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    // We also handle the message potentially returning null.
    try {
      platformVersion = await WaterpumpFfiPlugin.platformVersion ??
          'Unknown platform version';
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Flutter OPC UA Tank Example'),
        ),
        body: Center(
          child: Column(
            // mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text('Running on: $_platformVersion\n'),
              Text('Current server time is: $_curretServerTime'),
              Text('tank1: $_tank1PercentFilled'),
              Text('tank2: $_tank2PercentFilled'),
              Text('tank2 target: $_tank2TargetPercent'),
              Expanded(
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  crossAxisAlignment: CrossAxisAlignment.end,
                  children: [
                    Container(
                      width: 250,
                      height: _tank1PercentFilled * 3,
                      decoration: BoxDecoration(color: Color(0xff00bfff)),
                    ),
                    SizedBox(width: 10),
                    Container(
                      width: 250,
                      height: _tank2PercentFilled * 3,
                      decoration: BoxDecoration(color: Color(0xff00bfff)),
                    )
                  ],
                ),
              )
            ],
          ),
        ),
      ),
    );
  }
}
