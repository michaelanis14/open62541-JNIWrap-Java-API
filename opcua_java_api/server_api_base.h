#ifndef OPEN62541_HELPER_
#define OPEN62541_HELPER_


#include "open62541.h"
#include <signal.h>
#include <stdlib.h>

class ServerAPIBase {

private:
	static ServerAPIBase *jAPIBase_local;
public:

	static ServerAPIBase * Get();

	static void stopHandler(int sig) {
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
		
	}

	 UA_Server *  createServerDefaultConfig(void) {
		UA_Server *server = UA_Server_new();
		UA_ServerConfig_setDefault(UA_Server_getConfig(server));
		return server;
	}

	 UA_Server * createServer(UA_UInt16 port, char* host) {
		 // argv[1] contains your ip address
		 // personalize server configuration
		 UA_Server *server = UA_Server_new();
		 
		 UA_ServerConfig_setMinimal(UA_Server_getConfig(server), port, NULL); // set the port
																			  // set hostname to ip address
		 UA_String name;
		 UA_String_init(&name);

		 name.length = strlen(host);
		 name.data = (UA_Byte *)host;
		
		 UA_ServerConfig_setCustomHostname(UA_Server_getConfig(server), name);

		 return server;																
		 
	 }

	 UA_Server *  runServer(UA_Server * server) {
		signal(SIGINT, stopHandler);
		signal(SIGTERM, stopHandler);
		UA_Boolean running = true;
		UA_Server_run(server, &running);
		UA_StatusCode retval = UA_Server_run(server, &running);
		UA_Server_delete(server);
		
		return server;
	}

	 void addMonitoredItem(ServerAPIBase *jAPIBase,UA_Server *server, UA_NodeId immId);
	
	 UA_NodeId manuallyDefineIMM(UA_Server *server) {
		UA_NodeId statusNodeId;
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
		UA_Int32 status = 0;
		UA_Variant_setScalar(&statusAttr.value, &status, &UA_TYPES[UA_TYPES_INT32]);
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
	

	 UA_NodeId manuallyDefineRobot(UA_Server *server) {
		 UA_NodeId statusNodeId;
		 UA_NodeId immId; /* get the nodeid assigned by the server */
		 UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
		 oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Robot");
		 UA_Server_addObjectNode(server, UA_NODEID_NULL,
			 UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
			 UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
			 UA_QUALIFIEDNAME(1, "Robot"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
			 oAttr, NULL, &immId);

		 UA_VariableAttributes mnAttr = UA_VariableAttributes_default;
		 UA_String manufacturerName = UA_STRING("Robot Ltd.");
		 UA_Variant_setScalar(&mnAttr.value, &manufacturerName, &UA_TYPES[UA_TYPES_STRING]);
		 mnAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ManufacturerName");
		 UA_Server_addVariableNode(server, UA_NODEID_NULL, immId,
			 UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
			 UA_QUALIFIEDNAME(1, "ManufacturerName"),
			 UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), mnAttr, NULL, NULL);

		 UA_VariableAttributes modelAttr = UA_VariableAttributes_default;
		 UA_String modelName = UA_STRING("Robot 3000");
		 UA_Variant_setScalar(&modelAttr.value, &modelName, &UA_TYPES[UA_TYPES_STRING]);
		 modelAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ModelName");
		 UA_Server_addVariableNode(server, UA_NODEID_NULL, immId,
			 UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
			 UA_QUALIFIEDNAME(1, "ModelName"),
			 UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), modelAttr, NULL, NULL);

		 UA_VariableAttributes statusAttr = UA_VariableAttributes_default;
		 UA_Int32 status = 0;
		 UA_Variant_setScalar(&statusAttr.value, &status, &UA_TYPES[UA_TYPES_INT32]);
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
			 UA_QUALIFIEDNAME(1, "ArmMotorRPMs"),
			 UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), rpmAttr, NULL, NULL);

		 return statusNodeId;
	 }

	 void writeVariable(UA_Server *server, UA_NodeId* nodeId, UA_Int32 intValue) {
		UA_Variant myVar;
		UA_Variant_init(&myVar);
		UA_Variant_setScalar(&myVar, &intValue, &UA_TYPES[UA_TYPES_INT32]);
		UA_Server_writeValue(server, (*nodeId), myVar);
	}
	
	 virtual void monitored_itemChanged(const UA_NodeId *nodeId, const UA_Int32 value) {}
	


	virtual ~ServerAPIBase() {}
};
#endif // OPEN62541_HELPER_


