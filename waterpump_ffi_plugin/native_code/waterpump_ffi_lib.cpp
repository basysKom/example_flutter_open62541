#include "waterpump_ffi_lib.h"
#include <iostream>
#include <map>

// ----------------------- Open 62541 Example code --------------------------------------
// Open-62541-Example adapted from: https://github.com/open62541/open62541/blob/master/examples/client_subscription_loop.c
// --------------------------------------------------------------------------------------

#define SERVER_URL "opc.tcp://192.168.0.114:43344"

UA_Boolean running = true;
Dart_Port callback_send_port;
typedef void (*DART_CALLBACK)(intptr_t);
std::map<std::string, DART_CALLBACK> dartCallbackByNodeId;
std::map<UA_UInt32, std::string> nodeIdByMonitoringId;

// This is certainly not an approach that is suitable for production. But at least it enables some generic code
std::map<std::string, std::string> nodeTypeByNodeId {
    {"CurrentTime", "datetime"},
    {"Machine.Tank1.PercentFilled", "double"},
    {"Machine.Tank2.PercentFilled", "double"},
    {"Machine.Tank2.TargetPercent", "double"}
};

static void
handler_currentTimeChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
                           UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    if(UA_Variant_hasScalarType(&value->value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTime raw_date = *(UA_DateTime *) value->value.data;
        UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                    "date is: %02u-%02u-%04u %02u:%02u:%02u.%03u",
                    dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec);

        // Create string to be sent to dart
        char *date_string = (char *)malloc(1000 * sizeof(char));
        char format_string[] = "%04d-%02d-%02d %02d:%02d:%02d";
        sprintf(date_string, format_string, dts.year, dts.month, dts.day, dts.hour, dts.min, dts.sec);
        nonBlockingCallback((intptr_t)date_string, monId);
    }
}

static void
handler_doubleChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
                      UA_UInt32 monId, void *monContext, UA_DataValue *value)
{
    if (UA_Variant_hasScalarType(&value->value, &UA_TYPES[UA_TYPES_DOUBLE]))
    {
        UA_Double result = *(UA_Double *)(value->value.data);
        double *value = (double*)malloc(sizeof(double));
        *value = result;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "handler_doubleChanged: %f", *value);
        nonBlockingCallback((intptr_t)value, monId);
    }
    else
    {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Error processing double callback");
    }
}

static void
deleteSubscriptionCallback(UA_Client *client, UA_UInt32 subscriptionId, void *subscriptionContext) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Subscription Id %u was deleted", subscriptionId);
}

static void
subscriptionInactivityCallback (UA_Client *client, UA_UInt32 subId, void *subContext) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Inactivity for subscription %u", subId);
}

static void
stateCallback(UA_Client *client, UA_SecureChannelState channelState,
              UA_SessionState sessionState, UA_StatusCode recoveryStatus) {
    switch(channelState) {
    case UA_SECURECHANNELSTATE_FRESH:
    case UA_SECURECHANNELSTATE_CLOSED:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "The client is disconnected");
        break;
    case UA_SECURECHANNELSTATE_HEL_SENT:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Waiting for ack");
        break;
    case UA_SECURECHANNELSTATE_OPN_SENT:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Waiting for OPN Response");
        break;
    case UA_SECURECHANNELSTATE_OPEN:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "A SecureChannel to the server is open");
        break;
    default:
        break;
    }

    switch(sessionState) {
    case UA_SESSIONSTATE_ACTIVATED: {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "A session with the server is activated");
        /* A new session was created. We need to create the subscription. */
        /* Create a subscription */
        UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
        UA_CreateSubscriptionResponse response =
            UA_Client_Subscriptions_create(client, request, NULL, NULL, deleteSubscriptionCallback);
            if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Create subscription succeeded, id %u",
                            response.subscriptionId);
            else
                return;

            for (const auto& kv : nodeTypeByNodeId)
            {
                // FIXME: This is not the correct way to handle this
                char* nodeIdCStr = (char*)kv.first.c_str();
                UA_NodeId nodeId = UA_NODEID_STRING(2, nodeIdCStr);
                UA_MonitoredItemCreateRequest monRequest = UA_MonitoredItemCreateRequest_default(nodeId);

                UA_Client_DataChangeNotificationCallback callbackHandler = NULL;
                if (kv.second == "datetime" ) {
                    // TODO: This is more of a hack in order to get the node id for current time
                    nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME);
                    monRequest = UA_MonitoredItemCreateRequest_default(nodeId);
                    callbackHandler = handler_currentTimeChanged;
                } else if (kv.second == "double") {
                    callbackHandler = handler_doubleChanged;
                } else {
                    std::cout << "Unhandled type: " << kv.second << std::endl;
                    exit(EXIT_FAILURE);
                }

                UA_MonitoredItemCreateResult monResponse = UA_Client_MonitoredItems_createDataChange(client,
                                                                                                     response.subscriptionId,
                                                                                                     UA_TIMESTAMPSTORETURN_BOTH,
                                                                                                     monRequest,
                                                                                                     NULL,
                                                                                                     callbackHandler,
                                                                                                     NULL);
                nodeIdByMonitoringId[monResponse.monitoredItemId] = kv.first;

                assert(monResponse.statusCode == UA_STATUSCODE_GOOD);
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Monitoring %s', id %u", nodeIdCStr, monResponse.monitoredItemId);
            }
        }
        break;
    case UA_SESSIONSTATE_CLOSED:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Session disconnected");
        break;
    default:
        break;
    }
}

void threadLoop()
{
    UA_Client *client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(cc);

    /* Set stateCallback */
    cc->stateCallback = stateCallback;
    cc->subscriptionInactivityCallback = subscriptionInactivityCallback;

    /* Endless loop runAsync */
    while(running) {
        /* if already connected, this will return GOOD and do nothing */
        /* if the connection is closed/errored, the connection will be reset and then reconnected */
        /* Alternatively you can also use UA_Client_getState to get the current state */
        UA_StatusCode retval = UA_Client_connect(client, SERVER_URL);
        if(retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                         "Not connected. Retrying to connect in 1 second");
            /* The connect may timeout after 1 second (see above) or it may fail immediately on network errors */
            /* E.g. name resolution errors or unreachable network. Thus there should be a small sleep here */
            UA_sleep_ms(1000);
            continue;
        }

        UA_Client_run_iterate(client, 1000);
    };

    /* Clean up */
    UA_Client_delete(client); /* Disconnects the client internally */
}




intptr_t InitDartApiDL(void *data)
{
    return Dart_InitializeApiDL(data);
}

void nonBlockingCallback(intptr_t data, UA_UInt32 monId)
{
    assert(nodeIdByMonitoringId.count(monId) == 1);
    std::string nodeId = nodeIdByMonitoringId[monId];

    if (dartCallbackByNodeId.count(nodeId) != 1) {
        // Node was not registered yet
        // TODO: This probably introduces a memory leak...
        return;
    }

    auto callback = dartCallbackByNodeId[nodeId];

    // auto callback = machine_station1_status_dart_callback;
    // Create a lambda which calls the dart callback function with the new data
    const Work work = [data, callback]() { callback(data); };
    // Move it to the heap
    const Work *work_ptr = new Work(work);
    // Notify dart that a new callback is waiting
    notifyDart(callback_send_port, work_ptr);
}

// Notify Dart through a port that the C lib has pending async callbacks.
//
// Expects heap allocated `work` so delete can be called on it.
//
// The `send_port` should be from the isolate which registered the callback.
void notifyDart(Dart_Port send_port, const Work *work)
{
    // Notice that this runs in the background thread (compare to ExecuteCalbback())
    // printf("C: notifyDart() from thread %ld\n", std::this_thread::get_id());

    const intptr_t work_addr = reinterpret_cast<intptr_t>(work);

    Dart_CObject dart_object;
    dart_object.type = Dart_CObject_kInt64;
    dart_object.value.as_int64 = work_addr;

    const bool result = Dart_PostCObject_DL(send_port, &dart_object);
    if (!result) {
        printf("FATAL: Posting message to port failed.");
    }
}

void ExecuteCallback(Work *work_ptr)
{
    // This runs in Dart's thread (compare to notifyDart())
    // printf("C: ExecuteCallback() from thread %ld\n", std::this_thread::get_id());

    // Call the lambda which calls the dart callback with the new data
    const Work work = *work_ptr;
    work();

    delete work_ptr;
}

void SetCallbackendPort(Dart_Port send_port)
{
    callback_send_port = send_port;
}

void RegisterCallbackByNodeId(char* nodeId, void (*callback)(intptr_t))
{
    std::string nodeIdCppString(nodeId);
    assert(dartCallbackByNodeId.count(nodeIdCppString) == 0);
    dartCallbackByNodeId[nodeIdCppString] = callback;
}

void StartBackgroundThread()
{
    std::cout << "StartBackgroundThread()" << std::endl;

    std::thread t(threadLoop);
    t.detach();
}
