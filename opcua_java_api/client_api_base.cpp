#include "client_api_base.h"


ClientAPIBase * ClientAPIBase::jClientAPIBase_local = 0;
ClientAPIBase * ClientAPIBase::Get()
{
	if (jClientAPIBase_local == 0) {
		jClientAPIBase_local = new ClientAPIBase();
	}

	return jClientAPIBase_local;
}



static void handler_TheStatusChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
	UA_UInt32 monId, void *monContext, UA_DataValue *value) {
	ClientAPIBase::Get()->monitored_itemChanged((ClientAPIBase::Get()->current_nodeId), *(UA_Int32*)value->value.data);

	printf("The Status has changed from the client!\n");

}


UA_UInt32 ClientAPIBase::clientSubtoNode(ClientAPIBase * jClientAPIBase, UA_Client *client, UA_NodeId nodeID)
{
	/* Create a subscription */
	UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
	UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request,
		NULL, NULL, deleteSubscriptionCallback);

	UA_UInt32 subId = response.subscriptionId;
	if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
		printf("Create subscription succeeded, id %u\n", subId);

	UA_MonitoredItemCreateRequest monRequest =
		UA_MonitoredItemCreateRequest_default(nodeID);
	jClientAPIBase_local = jClientAPIBase;
	monRequest.requestedParameters.samplingInterval = 500; /* 100 ms interval */
	UA_MonitoredItemCreateResult monResponse =
		UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
			UA_TIMESTAMPSTORETURN_BOTH,
			monRequest, NULL, handler_TheStatusChanged, NULL);
	
	if (monResponse.statusCode == UA_STATUSCODE_GOOD)
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
			"Monitoring UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME', id %u",
			monResponse.monitoredItemId);

	if (monResponse.statusCode == UA_STATUSCODE_GOOD)
		printf("Monitoring 'Status', id %u\n", monResponse.monitoredItemId);

	return subId;
	/* The first publish request should return the initial value of the variable */
	//UA_Client_run_iterate(client, 10000);
}



