#ifndef OPEN62541_HELPER_
#define OPEN62541_HELPER_


#include "open62541.h"
#include <signal.h>
#include <stdlib.h>

	
	void set_UABoolean(UA_Boolean* flag) {
		flag = true;
	}

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

		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Received Notification");
	}

	static void
		addMonitoredItemToCurrentTimeVariable(UA_Server *server, UA_NodeId immId) {
		UA_MonitoredItemCreateRequest monRequest =
			UA_MonitoredItemCreateRequest_default(immId);

		monRequest.requestedParameters.samplingInterval = 100.0; /* 100 ms interval */
		UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
			monRequest, NULL, dataChangeNotificationCallback);
	}

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
		UA_Server_addVariableNode(server, UA_NODEID_NULL, immId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
			UA_QUALIFIEDNAME(1, "Status"),
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), statusAttr, NULL, NULL);

		UA_VariableAttributes rpmAttr = UA_VariableAttributes_default;
		UA_Double rpm = 50.0;
		UA_Variant_setScalar(&rpmAttr.value, &rpm, &UA_TYPES[UA_TYPES_DOUBLE]);
		rpmAttr.displayName = UA_LOCALIZEDTEXT("en-US", "MotorRPM");
		UA_Server_addVariableNode(server, UA_NODEID_NULL, immId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
			UA_QUALIFIEDNAME(1, "MotorRPMs"),
			UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), rpmAttr, NULL, NULL);

		return immId;
	}


#endif // OPEN62541_HELPER_