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
	static ClientAPIBase * Get();
	
	

	static void stopHandler(int sign) {
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Received Ctrl-C");
		ClientAPIBase::Get()->running = 0;
	}

	static void
		inactivityCallback(UA_Client *client) {
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server Inactivity");
	}

	static UA_Client * initClient() {
		UA_Client *client = UA_Client_new();
		ClientAPIBase::Get()->clientConfig = UA_Client_getConfig(client);
		UA_ClientConfig_setDefault(ClientAPIBase::Get()->clientConfig);
		return client;
	}

	static UA_StatusCode clientConnect(ClientAPIBase * jClientAPIBase, UA_Client * client, char* serverUrl) {
		signal(SIGINT, stopHandler); /* catches ctrl-c */
		ClientAPIBase::Get()->clientConfig->inactivityCallback = inactivityCallback; /* Set stateCallback */
		ClientAPIBase::Get()->clientConfig->connectivityCheckInterval = 2000; /* Perform a connectivity check every 2 seconds */

											  /* Endless loop runAsync */
		while (ClientAPIBase::Get()->running) {
			/* if already connected, this will return GOOD and do nothing */
			/* if the connection is closed/errored, the connection will be reset and then reconnected */
			/* Alternatively you can also use UA_Client_getState to get the current state */
			UA_StatusCode retval = UA_Client_connect(client, serverUrl);
			if (retval != UA_STATUSCODE_GOOD) {
				UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
					"Not connected. Retrying to connect in 1 second");
				/* The connect may timeout after 1 second (see above) or it may fail immediately on network errors */
				/* E.g. name resolution errors or unreachable network. Thus there should be a small sleep here */
				UA_sleep_ms(1000);
				continue;
			}
			jClientAPIBase->clientConnected(jClientAPIBase,client,serverUrl);
			UA_Client_run_iterate(client, 1000);
		};

		/* Clean up */
		UA_Client_delete(client); /* Disconnects the client internally */
		return EXIT_SUCCESS;
	}

	static UA_NodeId nodeIter(UA_NodeId childId, UA_Client *client , char* nodeName) {
		//printf("Browsing nodes in this object r:\n");
		UA_NodeId theStatusNodeID;

		UA_BrowseRequest bReq;
		UA_BrowseRequest_init(&bReq);
		bReq.requestedMaxReferencesPerNode = 0;
		bReq.nodesToBrowse = UA_BrowseDescription_new();
		bReq.nodesToBrowseSize = 1;
		bReq.nodesToBrowse[0].nodeId = childId; /* browse objects folder */
		bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
		UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
		//printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
		for (size_t i = 0; i < bResp.resultsSize; ++i) {
			for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
				UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
				if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
				//	printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
				//		ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
				//		ref->browseName.name.data, (int)ref->displayName.text.length,
				//		ref->displayName.text.data);

					UA_String s3 = UA_STRING(nodeName);

					UA_String s4;
					s4.length = (int)ref->displayName.text.length;
					s4.data = (UA_Byte*)ref->browseName.name.data;
					UA_Boolean eq = UA_String_equal(&s3, &s4);
					//printf(ref->displayName.text.data);
					if (eq) {
						theStatusNodeID = ref->nodeId.nodeId;
						break;
						//printf("THE RESULT STATEE \n");
					}

				}
			}
		}
		return theStatusNodeID;
	}

	static UA_NodeId getNodeByName(UA_Client *client, char* nodeName) {
		UA_NodeId theStatusNodeID;
		/* Browse some objects */
	//	printf("Browsing nodes in objects folder:\n");
		UA_BrowseRequest bReq;
		UA_BrowseRequest_init(&bReq);
		bReq.requestedMaxReferencesPerNode = 0;
		bReq.nodesToBrowse = UA_BrowseDescription_new();
		bReq.nodesToBrowseSize = 1;
		bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
		bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
		UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
	//	printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
		for (size_t i = 0; i < bResp.resultsSize; ++i) {
			for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
				UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
				if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
			//		printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
				//		ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
				//		ref->browseName.name.data, (int)ref->displayName.text.length,
				//		ref->displayName.text.data);
					UA_NodeId *parent = UA_NodeId_new();
					parent = &ref->nodeId.nodeId;
					theStatusNodeID = nodeIter(ref->nodeId.nodeId, client, nodeName);
					//	UA_Client_forEachChildNodeCall(client, ref->nodeId.nodeId,
					//		nodeIter, (void *)parent);

				}
				/*	else if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
				printf("%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
				(int)ref->nodeId.nodeId.identifier.string.length,
				ref->nodeId.nodeId.identifier.string.data,
				(int)ref->browseName.name.length, ref->browseName.name.data,
				(int)ref->displayName.text.length, ref->displayName.text.data);
				}
				TODO: distinguish further types */
			}
		}
		UA_BrowseRequest_clear(&bReq);
		UA_BrowseResponse_clear(&bResp);

		return theStatusNodeID;
	}

	
	static void clientSubtoNode(ClientAPIBase * jClientAPIBase, UA_Client *client, UA_NodeId nodeID);



	static void clientRemoveSub(UA_Client *client, UA_UInt32 subId) {
		/* Take another look at the.answer */
		UA_Client_run_iterate(client, 100);
		/* Delete the subscription */
		if (UA_Client_Subscriptions_deleteSingle(client, subId) == UA_STATUSCODE_GOOD)
			printf("NOW WILL Subscription removed\n");
	}

	static UA_Int32 clientReadValue(UA_Client *client, UA_NodeId theStatusNodeID) {
		UA_Int32 value = 0;
		printf("\nReading from Server the value of node");
		UA_Variant *val = UA_Variant_new();
		//    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, "the.answer"), val);
		UA_StatusCode retval = UA_Client_readValueAttribute(client, theStatusNodeID, val);


		if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
			val->type == &UA_TYPES[UA_TYPES_INT32]) {
			value = *(UA_Int32*)val->data;
			printf("the value is: %i\n", value);
		}
		UA_Variant_delete(val);
		return value;
	}

	static UA_StatusCode clientWriteValue(UA_Client *client, UA_NodeId theStatusNodeID, UA_Int32 value) {
		/* Write node attribute */
		UA_StatusCode write_state = UA_STATUSCODE_GOOD;
		printf("\nWriting a value to a node\n");
		UA_WriteRequest wReq;
		UA_WriteRequest_init(&wReq);
		wReq.nodesToWrite = UA_WriteValue_new();
		wReq.nodesToWriteSize = 1;
		//  wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING_ALLOC(1, "the.answer");
		wReq.nodesToWrite[0].nodeId = theStatusNodeID;

		wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
		wReq.nodesToWrite[0].value.hasValue = true;
		wReq.nodesToWrite[0].value.value.type = &UA_TYPES[UA_TYPES_INT32];
		wReq.nodesToWrite[0].value.value.storageType = UA_VARIANT_DATA_NODELETE; /* do not free the integer on deletion */
		wReq.nodesToWrite[0].value.value.data = &value;

		UA_WriteResponse wResp = UA_Client_Service_write(client, wReq);

		write_state = wResp.responseHeader.serviceResult;
		if (wResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
			printf("the new value is: %i\n", value);
	
		UA_WriteRequest_clear(&wReq);
		UA_WriteResponse_clear(&wResp);

		return write_state;

	}

	virtual void monitored_itemChanged(UA_NodeId nodeId, const UA_Int32 value) {}
	virtual void clientConnected(ClientAPIBase * jClientAPIBase, UA_Client *client,char* serverUrl) {}

	virtual ~ClientAPIBase() {}
};

