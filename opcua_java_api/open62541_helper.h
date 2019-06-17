#ifndef OPEN62541_HELPER_
#define OPEN62541_HELPER_


#include "open62541.h"
#include <signal.h>
#include <stdlib.h>




	static volatile UA_Boolean running = true;
	static void stopHandler(int sig) {
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
		running = false;
	}

	UA_Server *  createServerDefaultConfig(void) {
		UA_Server *server = UA_Server_new();
		UA_ServerConfig_setDefault(UA_Server_getConfig(server));
		return server;
	}
	UA_Server *  runServer(UA_Server * server) {
		signal(SIGINT, stopHandler);
		signal(SIGTERM, stopHandler);
		
		UA_Server_run(server, &running);
		UA_StatusCode retval = UA_Server_run(server, &running);
		UA_Server_delete(server);
		
		return server;
	}

	static void
		dataChangeNotificationCallback(UA_Server *server, UA_UInt32 monitoredItemId,
			void *monitoredItemContext, const UA_NodeId *nodeId,
			void *nodeContext, UA_UInt32 attributeId,
			const UA_DataValue *value) {
	

		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server Received Notification on Monitored Item "  );
	}

	static void
		addMonitoredItemToCurrentTimeVariable(UA_Server *server, UA_NodeId immId) {
		UA_MonitoredItemCreateRequest monRequest =
			UA_MonitoredItemCreateRequest_default(immId);

		monRequest.requestedParameters.samplingInterval = 100.0; /* 100 ms interval */
		UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
			monRequest, NULL, dataChangeNotificationCallback);
	}
	UA_NodeId statusNodeId;
	static UA_NodeId
		manuallyDefineIMM(UA_Server *server) {
		UA_NodeId immId; /* get the nodeid assigned by the server */
		UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
		oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "IMM");
		UA_Server_addObjectNode(server, UA_NODEID_NULL,
			UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
			UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
			UA_QUALIFIEDNAME(1, "IMM"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
			oAttr, NULL, &immId);

		UA_VariableAttributes mnAttr = UA_VariableAttributes_default;
		UA_String manufacturerName = UA_STRING("IMM Ltd.");
		UA_Variant_setScalar(&mnAttr.value, &manufacturerName, &UA_TYPES[UA_TYPES_STRING]);
		mnAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ManufacturerName");
		UA_Server_addVariableNode(server, UA_NODEID_NULL, immId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
			UA_QUALIFIEDNAME(1, "ManufacturerName"),
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), mnAttr, NULL, NULL);

		UA_VariableAttributes modelAttr = UA_VariableAttributes_default;
		UA_String modelName = UA_STRING("IMM 3000");
		UA_Variant_setScalar(&modelAttr.value, &modelName, &UA_TYPES[UA_TYPES_STRING]);
		modelAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ModelName");
		UA_Server_addVariableNode(server, UA_NODEID_NULL, immId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
			UA_QUALIFIEDNAME(1, "ModelName"),
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), modelAttr, NULL, NULL);

		UA_VariableAttributes statusAttr = UA_VariableAttributes_default;
		UA_Boolean status = true;
		UA_Variant_setScalar(&statusAttr.value, &status, &UA_TYPES[UA_TYPES_BOOLEAN]);
		statusAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Status");
		statusAttr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
		UA_Server_addVariableNode(server, UA_NODEID_NULL, immId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
			UA_QUALIFIEDNAME(1, "Status"),
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), statusAttr, NULL, &statusNodeId);

		UA_VariableAttributes rpmAttr = UA_VariableAttributes_default;
		UA_Double rpm = 50.0;
		UA_Variant_setScalar(&rpmAttr.value, &rpm, &UA_TYPES[UA_TYPES_DOUBLE]);
		rpmAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MouldSize");
		UA_Server_addVariableNode(server, UA_NODEID_NULL, immId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
			UA_QUALIFIEDNAME(1, "MotorRPMs"),
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), rpmAttr, NULL, NULL);

		return statusNodeId;
	}


	static void writeVariable(UA_Server *server, UA_NodeId* nodeId, UA_Int32 intValue) {

		//	UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");

		/* Write a different integer value */
		UA_Int32 myInteger = intValue;
		UA_Variant myVar;
		UA_Variant_init(&myVar);
		UA_Variant_setScalar(&myVar, &intValue, &UA_TYPES[UA_TYPES_INT32]);
		UA_Server_writeValue(server, (*nodeId), myVar);

		/* Set the status code of the value to an error code. The function
		* UA_Server_write provides access to the raw service. The above
		* UA_Server_writeValue is syntactic sugar for writing a specific node
		* attribute with the write service. */
		UA_WriteValue wv;
		UA_WriteValue_init(&wv);
		wv.nodeId = *nodeId;
		wv.attributeId = UA_ATTRIBUTEID_VALUE;
		wv.value.status = UA_STATUSCODE_BADNOTCONNECTED;
		wv.value.hasStatus = true;
		UA_Server_write(server, &wv);

		/* Reset the variable to a good statuscode with a value */
		wv.value.hasStatus = false;
		wv.value.value = myVar;
		wv.value.hasValue = true;
		UA_Server_write(server, &wv);
	}




	static void
		handler_TheAnswerChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
			UA_UInt32 monId, void *monContext, UA_DataValue *value) {
		printf("The Answer has changed!\n");
	}


	static UA_StatusCode
		nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
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
#endif // OPEN62541_HELPER_