#ifndef OPEN62541_HELPER_
#define OPEN62541_HELPER_


#include "open62541.h"
#include <signal.h>
#include <stdlib.h>

class ServerAPIBase {

private:
	static ServerAPIBase *jAPIBase_local;
	
	size_t outputs_length = 1;
	
public:
	struct method_output
	{
		UA_NodeId key;
		UA_Variant *value;
		ServerAPIBase *api_local;
	};
	void *d;
	
	method_output  *methodOutputs;
	
	UA_Boolean running;
	static ServerAPIBase * Get();
	bool addOutput(method_output output);
	int getNodeIdIndex(UA_NodeId nodeId);
	static void stopHandler(int sig);

	UA_Server *  createServerDefaultConfig(void); 

	 UA_Server * createServer( char* host, UA_UInt16 port); 

	 UA_StatusCode  runServer(UA_Server * server);

	 void addMonitoredItem(UA_Server *server, UA_NodeId immId, ServerAPIBase *jAPIBase);
	
	 UA_NodeId addObject(UA_Server *server, const UA_Int32 requestedNewNodeId, char* name);
	 UA_NodeId addVariableNode(UA_Server * server, UA_NodeId objectId, const UA_Int32 requestedNewNodeId, char * name, UA_Int32 typeId, UA_Int32 accessLevel);
	 UA_NodeId manuallyDefineIMM(UA_Server *server);
	 UA_NodeId manuallyDefineRobot(UA_Server *server);

	 UA_StatusCode writeVariable(UA_Server *server, UA_NodeId* nodeId, int intValue);
	 UA_StatusCode writeVariable(UA_Server *server, UA_NodeId* nodeId, char * stringValue);
	 UA_StatusCode writeVariable(UA_Server *server, UA_NodeId* nodeId, double  doubleValue);
	 UA_NodeId getDataTypeNode(UA_Int32 typeId);
	 UA_NodeId addMethod(UA_Server *server, UA_NodeId objectId, const UA_Int32 requestedNewNodeId , UA_Argument inputArgument, UA_Argument outputArgument, UA_MethodAttributes methodAttr, ServerAPIBase *jAPIBase);

	 static UA_StatusCode methodCallback(UA_Server *server,
		 const UA_NodeId *sessionId, void *sessionHandle,
		 const UA_NodeId *methodId, void *methodContext,
		 const UA_NodeId *objectId, void *objectContext,
		 size_t inputSize, const UA_Variant *input,
		 size_t outputSize, UA_Variant *output);

	 void setData(void *);
	 void *getData();
	 void setMethodOutput(UA_NodeId methodId,UA_String output);
	 
	 virtual void monitored_itemChanged(const UA_NodeId *nodeId, const UA_Int32 value) {}
	 virtual void methods_callback( const UA_NodeId *methodId, const UA_NodeId *objectId, UA_String input, UA_String output,ServerAPIBase *jAPIBase) {}




	virtual ~ServerAPIBase() {}
};
#endif // OPEN62541_HELPER_


