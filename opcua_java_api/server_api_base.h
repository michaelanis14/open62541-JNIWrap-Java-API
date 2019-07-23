#ifndef OPEN62541_HELPER_
#define OPEN62541_HELPER_


#include "open62541.h"
#include <signal.h>
#include <stdlib.h>

class ServerAPIBase {

private:
	static ServerAPIBase *jAPIBase_local;
	
public:
	void *d;
	UA_Variant *output;
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

		 UA_ServerConfig_addSecurityPolicyNone(UA_Server_getConfig(server), NULL);

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

	 void addMonitoredItem(UA_Server *server, UA_NodeId immId, ServerAPIBase *jAPIBase);
	
	 UA_NodeId addObject(UA_Server *server, const UA_Int32 requestedNewNodeId, char* name);
	 UA_NodeId addVariableNode(UA_Server * server, UA_NodeId objectId, const UA_Int32 requestedNewNodeId, char * name, UA_Int32 typeId, UA_Int32 accessLevel);
	 UA_NodeId manuallyDefineIMM(UA_Server *server);
	 UA_NodeId manuallyDefineRobot(UA_Server *server);

	 UA_StatusCode writeVariable(UA_Server *server, UA_NodeId* nodeId, int intValue);
	 UA_StatusCode writeVariable(UA_Server *server, UA_NodeId* nodeId, char * stringValue);
	 UA_StatusCode writeVariable(UA_Server *server, UA_NodeId* nodeId, double  doubleValue);
	 UA_NodeId getDataTypeNode(UA_Int32 typeId);
	 void addMethod(UA_Server *server, UA_Argument inputArgument, UA_Argument outputArgument, UA_MethodAttributes methodAttr, ServerAPIBase *jAPIBase);


	 void setData(void *);
	 void *getData();
	 void setMethodOutput(UA_String output);

	 virtual void monitored_itemChanged(const UA_NodeId *nodeId, const UA_Int32 value) {}
	 virtual void methods_callback(ServerAPIBase *jAPIBase, const UA_NodeId *methodId, const UA_NodeId *objectId, UA_String input, UA_String output) {}




	virtual ~ServerAPIBase() {}
};
#endif // OPEN62541_HELPER_


