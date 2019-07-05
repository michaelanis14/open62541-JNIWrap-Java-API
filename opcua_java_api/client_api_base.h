#pragma once
#include "open62541.h"

class ClientAPIBase
{
public:
	ClientAPIBase();
	static UA_StatusCode nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
		if (isInverse)
			return UA_STATUSCODE_GOOD;
		UA_NodeId *parent = (UA_NodeId *)handle;
		printf("%d, %d --- %d ---> NodeId %d, %d\n",
			parent->namespaceIndex, parent->identifier.numeric,
			referenceTypeId.identifier.numeric, childId.namespaceIndex,
			childId.identifier.numeric);
		return UA_STATUSCODE_GOOD;
	}

	static UA_Client * initClient() {
		UA_Client *client = UA_Client_new();
		UA_ClientConfig_setDefault(UA_Client_getConfig(client));


		return client;
	}

	static UA_StatusCode clientConnect(UA_Client * client) {

		/* Listing endpoints */
		UA_EndpointDescription* endpointArray = NULL;
		size_t endpointArraySize = 0;
		UA_StatusCode retval = UA_Client_getEndpoints(client, "opc.tcp://localhost:4840",
			&endpointArraySize, &endpointArray);
		if (retval != UA_STATUSCODE_GOOD) {
			UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
			UA_Client_delete(client);
			return EXIT_FAILURE;
		}
		printf("%i endpoints found\n", (int)endpointArraySize);
		for (size_t i = 0; i<endpointArraySize; i++) {
			printf("URL of endpoint %i is %.*s\n", (int)i,
				(int)endpointArray[i].endpointUrl.length,
				endpointArray[i].endpointUrl.data);
		}
		UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);

		/* Connect to a server */
		/* anonymous connect would be: retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); */
		retval = UA_Client_connect_username(client, "opc.tcp://localhost:4840", "user1", "password");
		if (retval != UA_STATUSCODE_GOOD) {
			UA_Client_delete(client);
			return EXIT_FAILURE;
		}
	}

	static UA_NodeId nodeIterC(UA_NodeId childId, UA_Client *client) {
		printf("Browsing nodes in this object r:\n");
		UA_NodeId theStatusNodeID;

		UA_BrowseRequest bReq;
		UA_BrowseRequest_init(&bReq);
		bReq.requestedMaxReferencesPerNode = 0;
		bReq.nodesToBrowse = UA_BrowseDescription_new();
		bReq.nodesToBrowseSize = 1;
		bReq.nodesToBrowse[0].nodeId = childId; /* browse objects folder */
		bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
		UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
		printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
		for (size_t i = 0; i < bResp.resultsSize; ++i) {
			for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
				UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
				if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
					printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
						ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
						ref->browseName.name.data, (int)ref->displayName.text.length,
						ref->displayName.text.data);

					UA_String s3 = UA_STRING("Status");

					UA_String s4;
					s4.length = (int)ref->displayName.text.length;
					s4.data = (UA_Byte*)ref->browseName.name.data;
					UA_Boolean eq = UA_String_equal(&s3, &s4);
					//printf(ref->displayName.text.data);
					if (eq) {
						theStatusNodeID = ref->nodeId.nodeId;
						printf("THE RESULT STATEE \n");
					}

				}
			}
		}
		return theStatusNodeID;
	}

	static UA_NodeId getStatusNode(UA_Client *client) {
		UA_NodeId theStatusNodeID;
		/* Browse some objects */
		printf("Browsing nodes in objects folder:\n");
		UA_BrowseRequest bReq;
		UA_BrowseRequest_init(&bReq);
		bReq.requestedMaxReferencesPerNode = 0;
		bReq.nodesToBrowse = UA_BrowseDescription_new();
		bReq.nodesToBrowseSize = 1;
		bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); /* browse objects folder */
		bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */
		UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
		printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
		for (size_t i = 0; i < bResp.resultsSize; ++i) {
			for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
				UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
				if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
					printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
						ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
						ref->browseName.name.data, (int)ref->displayName.text.length,
						ref->displayName.text.data);
					UA_NodeId *parent = UA_NodeId_new();
					parent = &ref->nodeId.nodeId;
					theStatusNodeID = nodeIterC(ref->nodeId.nodeId, client);
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

	static void handler_TheStatusChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
		UA_UInt32 monId, void *monContext, UA_DataValue *value) {
		printf("The Status has changed!\n");
	}
	static void clientSubtoNode(UA_Client *client, UA_NodeId theStatusNodeID) {
		/* Create a subscription */
		UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
		UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request,
			NULL, NULL, NULL);

		UA_UInt32 subId = response.subscriptionId;
		if (response.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
			printf("Create subscription succeeded, id %u\n", subId);

		UA_MonitoredItemCreateRequest monRequest =
			// UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(1, "the.answer"));
			UA_MonitoredItemCreateRequest_default(theStatusNodeID);

		UA_MonitoredItemCreateResult monResponse =
			UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
				UA_TIMESTAMPSTORETURN_BOTH,
				monRequest, NULL, handler_TheStatusChanged, NULL);
		if (monResponse.statusCode == UA_STATUSCODE_GOOD)
			printf("Monitoring 'Status', id %u\n", monResponse.monitoredItemId);


		/* The first publish request should return the initial value of the variable */
		UA_Client_run_iterate(client, 1000);
	}

	static void clientRemoveSub(UA_Client *client, UA_UInt32 subId) {
		/* Take another look at the.answer */
		UA_Client_run_iterate(client, 100);
		/* Delete the subscription */
		if (UA_Client_Subscriptions_deleteSingle(client, subId) == UA_STATUSCODE_GOOD)
			printf("Subscription removed\n");
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
		if (wResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
			printf("the new value is: %i\n", value);
		UA_WriteRequest_clear(&wReq);
		UA_WriteResponse_clear(&wResp);

		return UA_STATUSCODE_GOOD;

	}

	virtual ~ClientAPIBase() {}
};

