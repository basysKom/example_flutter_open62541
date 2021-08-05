#ifndef WATERPUMP_FFI_LIB_H
#define WATERPUMP_FFI_LIB_H

#include <functional>
#include <thread>

// Includes from the dart sdk required for asynchronous callbacks
#include "include/dart_api.h"
#include "include/dart_native_api.h"
#include "include/dart_api_dl.h"

// Open62541
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>

typedef std::function<void()> Work;

// Private functions
void threadLoop();
void notifyDart(Dart_Port send_port, const Work *work);
void nonBlockingCallback(intptr_t data, UA_UInt32 monId);

// Public API functions to be used from dart
// Handle c++ name mangling to make the functions usable from Dart
#define EXTERN_C extern "C" __attribute__((visibility("default"))) __attribute__((used))
EXTERN_C void StartBackgroundThread();
EXTERN_C intptr_t InitDartApiDL(void *data);
EXTERN_C void ExecuteCallback(Work* work_ptr);
EXTERN_C void SetCallbackendPort(Dart_Port send_port);
EXTERN_C void RegisterCallbackByNodeId(char* nodeId, void (*callback)(intptr_t));

#endif
