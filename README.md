# flutter-waterpump
This is a small demo project showing how to use open62541 with Flutter and asynchronous callbacks.

## Setup
The demo project was developed using Ubuntu 18.04 and tested on Ubuntu 18.04 and Android.

### OPC UA server
- For the example OPC UA server you need to install Qt (see https://doc.qt.io/qt-5/gettingstarted.html) along with the qtopcua plugin (see https://github.com/qt/qtopcua). I used Qt 5.15.2. There is a script `server/clone-and-build-qtopcua.sh` which will clone and build qtopca assuming that Qt 5.15.2 is installed in `$HOME/Qt`.
- qtopcua also contains the waterpump example server this demo is using as a backend. Some lines were changed (see `.patch`-files) in the example to make it start pumping forever right after it was started.
- To run the server you can use the script `server/make-and-run-waterpump-server.sh`
- To check whether everything works you can start the Qt client using `LD_LIBRARY_PATH=$HOME/Qt/5.15.2/gcc_64/lib/:$LD_LIBRARY_PATH build/examples/opcua/waterpump/waterpump-qmlcpp/waterpump-qmlcpp`

### Flutter
- The flutter plugin is located in `waterpump_ffi_plugin`
- Before building/running this you have to prepare the dart sdk and open62541. To do so you can run `waterpump_ffi_plugin/native_code/prepare-dependencies.sh` which will download the sdk and prepare open62541.

## Running
As usual with Flutter plugins there is an example app that shows how to use the plugin located in `exmaple`. To run this change to the directory and run `flutter -d linux run` to test the app for linux

