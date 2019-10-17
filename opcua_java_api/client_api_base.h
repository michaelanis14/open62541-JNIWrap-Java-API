#pragma once
#include "open62541.h"
#include <signal.h>
class ClientAPIBase
{
private:
	static ClientAPIBase *jClientAPIBase_local;
	size_t outputs_length = 1;
	
public:
	UA_Int32 methodInputs[10];
	struct method_output
	{
		UA_UInt32 subId;
		UA_NodeId key;
		UA_Variant *value;
		ClientAPIBase *api_local;
	};

	method_output  *methodOutputs;
	static bool AddOutput(method_output output);
	static int GetSubIdIndex(UA_UInt32 subId);

//	UA_NodeId current_nodeId;
	UA_ClientConfig *clientConfig;
	UA_Boolean running = true;
	char* output;
	static ClientAPIBase * Get();



	static void stopHandler(int sign);


	static void inactivityCallback(UA_Client *client);

	static UA_Client * InitClient();

	static UA_StatusCode ClientConnect(ClientAPIBase * jClientAPIBase, UA_Client * client, char* serverUrl);

	static void handler_TheStatusChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
		UA_UInt32 monId, void *monContext, UA_DataValue *value);

	static UA_NodeId NodeIter(UA_NodeId childId, UA_Client *client, char* nodeName);

	static UA_NodeId GetNodeByName(UA_Client *client, char* nodeName);


	static UA_UInt32 ClientSubtoNode(ClientAPIBase * jClientAPIBase, UA_Client *client, UA_NodeId nodeID);

	static UA_Variant SetGetVariant(UA_Variant * value);

	static void ClientRemoveSub(UA_Client *client, UA_UInt32 subId);

	static UA_Variant * ClientReadValue(UA_Client *client, UA_NodeId nodeID);
	static UA_Int32   ClientReadIntValue(UA_Client *client, UA_NodeId nodeID);
	static UA_StatusCode ClientWriteValue( char* serverUrl, UA_NodeId nodeId, UA_Int32 value);

	static void
		deleteSubscriptionCallback(UA_Client *client, UA_UInt32 subscriptionId, void *subscriptionContext);

	static void
		subscriptionInactivityCallback(UA_Client *client, UA_UInt32 subId, void *subContext);
	static char* GetMethodOutput();
	static UA_String CallMethod(char* serverUrl, const UA_NodeId objectId,
			const UA_NodeId methodId, char* argInputString);

	static UA_String CallArrayMethod( char* serverUrl, const UA_NodeId objectId,
		const UA_NodeId methodId, UA_Int32 methodInputs[], UA_Int32 arraySize, UA_Variant *output);

	virtual void monitored_itemChanged(UA_NodeId nodeId, const UA_Int32 value) {}
	virtual void client_connected(ClientAPIBase * jClientAPIBase, UA_Client *client, char* serverUrl) {}
	virtual void methods_callback(const UA_NodeId objectId,const UA_NodeId methodId, UA_String input, UA_String output, ClientAPIBase *jAPIBase) {}



	virtual ~ClientAPIBase() {}
};


