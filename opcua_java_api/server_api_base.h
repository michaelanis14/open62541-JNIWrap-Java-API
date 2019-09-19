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
	
	method_output  *methodOutputs;
	
	UA_Boolean running;
	static ServerAPIBase * Get();
	static bool AddOutput(method_output output);
	static int GetNodeIdIndex(UA_NodeId nodeId);
	static void stopHandler(int sig);

	static UA_Server *  CreateServerDefaultConfig(void);

	static  UA_Server * CreateServer( char* host, UA_UInt16 port);

	static  UA_StatusCode  RunServer(UA_Server * server);

	static  void AddMonitoredItem(ServerAPIBase *jAPIBase, UA_Server *server, UA_NodeId monitoredItemId );
	
	static  UA_NodeId AddObject(UA_Server *server, UA_NodeId requestedNewNodeId, char* name);
	static  UA_NodeId AddObject(UA_Server *server, UA_NodeId parent, UA_NodeId requestedNewNodeId, char* name);
	static  UA_NodeId AddVariableNode(UA_Server * server, UA_NodeId objectId, UA_NodeId requestedNewNodeId, char * name, UA_Int32 typeId, UA_Int32 accessLevel);
	static  UA_NodeId ManuallyDefineIMM(UA_Server *server);
	static UA_NodeId ManuallyDefineRobot(UA_Server *server);

	static  UA_StatusCode WriteVariable(UA_Server *server, UA_NodeId* nodeId, int intValue);
	static UA_StatusCode WriteVariable(UA_Server *server, UA_NodeId* nodeId, char * stringValue);
	static UA_StatusCode WriteVariable(UA_Server *server, UA_NodeId* nodeId, double  doubleValue);
	static UA_NodeId GetDataTypeNode(UA_Int32 typeId);
	static UA_NodeId AddMethod(ServerAPIBase *jAPIBase,UA_Server *server, UA_NodeId objectId, UA_NodeId requestedNewNodeId , UA_Argument inputArgument, UA_Argument outputArgument, UA_MethodAttributes methodAttr);

	

	 static UA_StatusCode methodCallback(UA_Server *server,
		 const UA_NodeId *sessionId, void *sessionHandle,
		 const UA_NodeId *methodId, void *methodContext,
		 const UA_NodeId *objectId, void *objectContext,
		 size_t inputSize, const UA_Variant *input,
		 size_t outputSize, UA_Variant *output);
	 static void dataChangeNotificationCallback(UA_Server *server, UA_UInt32 monitoredItemId,
		 void *monitoredItemContext, const UA_NodeId *nodeId,
		 void *nodeContext, UA_UInt32 attributeId,
		 const UA_DataValue *value);

	
	 static void SetMethodOutput(UA_NodeId methodId,UA_String output);
	 
	 virtual void monitored_itemChanged(const UA_NodeId *nodeId, const UA_Int32 value) {}
	 virtual void methods_callback( const UA_NodeId *methodId, const UA_NodeId *objectId, UA_String input, UA_String output,ServerAPIBase *jAPIBase) {}




	virtual ~ServerAPIBase() {}
};
#endif // OPEN62541_HELPER_


