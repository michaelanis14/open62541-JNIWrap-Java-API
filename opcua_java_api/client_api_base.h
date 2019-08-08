#pragma once
#include "open62541.h"
#include <signal.h>
class ClientAPIBase
{
private:
	static ClientAPIBase *jClientAPIBase_local;

public:
	UA_NodeId current_nodeId;
	UA_ClientConfig *clientConfig;
	UA_Boolean running = true;
	char* output;
	static ClientAPIBase * Get();



	static void stopHandler(int sign);


	static void inactivityCallback(UA_Client *client);

	static UA_Client * initClient();

	static UA_StatusCode clientConnect(ClientAPIBase * jClientAPIBase, UA_Client * client, char* serverUrl);

	static void handler_TheStatusChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
		UA_UInt32 monId, void *monContext, UA_DataValue *value);

	static UA_NodeId nodeIter(UA_NodeId childId, UA_Client *client, char* nodeName);

	static UA_NodeId getNodeByName(UA_Client *client, char* nodeName);


	static UA_UInt32 clientSubtoNode(ClientAPIBase * jClientAPIBase, UA_Client *client, UA_NodeId nodeID);



	static void clientRemoveSub(UA_Client *client, UA_UInt32 subId);

	static UA_Int32 clientReadValue(UA_Client *client, UA_NodeId theStatusNodeID);

	static UA_StatusCode clientWriteValue(UA_Client *client, UA_NodeId theStatusNodeID, UA_Int32 value);

	static void
		deleteSubscriptionCallback(UA_Client *client, UA_UInt32 subscriptionId, void *subscriptionContext);

	static void
		subscriptionInactivityCallback(UA_Client *client, UA_UInt32 subId, void *subContext);
	static char* getMethodOutput();
	static UA_String call_method(UA_Client *client, const UA_NodeId objectId,
			const UA_NodeId methodId, char* argInputString);

	virtual void monitored_itemChanged(UA_NodeId nodeId, const UA_Int32 value) {}
	virtual void client_connected(ClientAPIBase * jClientAPIBase, UA_Client *client, char* serverUrl) {}
	virtual void methods_callback(const UA_NodeId objectId,const UA_NodeId methodId, UA_String input, UA_String output, ClientAPIBase *jAPIBase) {}

	virtual ~ClientAPIBase() {}
};


