#include "server_api_base.h"





ServerAPIBase * ServerAPIBase::jAPIBase_local = 0;
void ServerAPIBase::stopHandler(int sig)
{
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c " + sig);
	ServerAPIBase::Get()->running = false;
}

void ServerAPIBase::dataChangeNotificationCallback(UA_Server *server, UA_UInt32 monitoredItemId,
	void *monitoredItemContext, const UA_NodeId *nodeId,
	void *nodeContext, UA_UInt32 attributeId,
	const UA_DataValue *value) {

	// ServerAPIBase::Get()->monitored_itemChanged(nodeId, value);

	int nodeOutputIndex = ServerAPIBase::Get()->GetNodeIdIndex(*nodeId);
	if (nodeOutputIndex != -1) {
		ServerAPIBase* serverApi = ServerAPIBase::Get()->methodOutputs[nodeOutputIndex].api_local;
		ServerAPIBase::Get()->methodOutputs[nodeOutputIndex].value = (UA_Variant*)value->value.data;
		serverApi->monitored_itemChanged(nodeId, *(UA_Int32*)value->value.data);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server Received Notification on Monitored Item ");
	}
	else UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server received notification but coudlnt find the monitored item callback. ");
}


void ServerAPIBase::SetMethodOutput(UA_NodeId methodId, UA_String output)
{

	ServerAPIBase* serverApi = ServerAPIBase::Get();
	int nodeOutputIndex = serverApi->GetNodeIdIndex(methodId);

	if (nodeOutputIndex != -1) {
		UA_Variant_setScalarCopy(serverApi->methodOutputs[nodeOutputIndex].value, &output, &UA_TYPES[UA_TYPES_STRING]);
	}
}

UA_NodeId ServerAPIBase::CreateStringNodeId(UA_UInt16 nsIndex, char * chars)
{
	return UA_NODEID_STRING(nsIndex, chars);
}




UA_StatusCode  ServerAPIBase::methodCallback(UA_Server *server,
	const UA_NodeId *sessionId, void *sessionHandle,
	const UA_NodeId *methodId, void *methodContext,
	const UA_NodeId *objectId, void *objectContext,
	size_t inputSize, const UA_Variant *input,
	size_t outputSize, UA_Variant *output) {
	/*
*/
	UA_String *inputStr = &UA_STRING("-1");
	if(UA_Variant_isScalar(input))
		inputStr = (UA_String*)input->data;
	

	//serverApi->output = output;
	int nodeOutputIndex = ServerAPIBase::Get()->GetNodeIdIndex(*methodId);
	if (nodeOutputIndex != -1) {
		ServerAPIBase* serverApi = ServerAPIBase::Get()->methodOutputs[nodeOutputIndex].api_local;
		ServerAPIBase::Get()->methodOutputs[nodeOutputIndex].value = output;
		serverApi->methods_callback(methodId, objectId, *inputStr, *inputStr, serverApi);
		return UA_STATUSCODE_GOOD;
	}
	return UA_STATUSCODE_BADNOTFOUND;
}


/**
* Method:    Get
* FullName:  Get
* Access:    public
* * @return   ServerAPIBase
*/
ServerAPIBase * ServerAPIBase::Get()
{
	if (jAPIBase_local == 0) {
		//printf("New Refrence SERVER API");
		jAPIBase_local = new ServerAPIBase();

	}//else printf("Keeping Refrence SERVER API");

	return jAPIBase_local;
}

bool ServerAPIBase::AddOutput(method_output output)
{
	if (ServerAPIBase::Get()->outputs_length == 1) {
		ServerAPIBase::Get()->methodOutputs = new method_output[ServerAPIBase::Get()->outputs_length];
	}
	else {
		method_output* temp = new method_output[ServerAPIBase::Get()->outputs_length];
		memcpy(temp, ServerAPIBase::Get()->methodOutputs, ServerAPIBase::Get()->outputs_length * sizeof(method_output));
		delete[] ServerAPIBase::Get()->methodOutputs;
		ServerAPIBase::Get()->methodOutputs = temp;
	}
	
	ServerAPIBase::Get()->methodOutputs[ServerAPIBase::Get()->outputs_length - 1] = output;
	ServerAPIBase::Get()->outputs_length++;
	return true;
}

int ServerAPIBase::GetNodeIdIndex(UA_NodeId nodeId)
{
	for (int i = 0; i < ServerAPIBase::Get()->outputs_length; i++) {
		if (UA_NodeId_equal(&ServerAPIBase::Get()->methodOutputs[i].key, &nodeId)) {
			//UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "getNodeIdIndex, id %u \n", i);
			return i;
		}
	}
	return -1;
}

UA_Server * ServerAPIBase::CreateServerDefaultConfig(void)
{
	UA_Server *server = UA_Server_new();
	UA_ServerConfig_setDefault(UA_Server_getConfig(server));
	return server;
}

UA_Server * ServerAPIBase::CreateServer(char * host, UA_UInt16 port)
{
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

UA_StatusCode ServerAPIBase::RunServer(UA_Server * server)
{
	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);
	ServerAPIBase::Get()->running = true;
	/* Should the server networklayer block (with a timeout) until a message
	arrives or should it return immediately? */
	UA_Boolean waitInternal = false;

	UA_StatusCode retval = UA_Server_run(server, &(ServerAPIBase::Get()->running));
	while (ServerAPIBase::Get()->running)
		UA_Server_run_iterate(server, waitInternal);
	return UA_Server_run_shutdown(server);

}

void ServerAPIBase::AddMonitoredItem(ServerAPIBase *jAPIBase, UA_Server *server, UA_NodeId monitoredItemId) {
	UA_MonitoredItemCreateRequest monRequest =
		UA_MonitoredItemCreateRequest_default(monitoredItemId);
	//jAPIBase_local = jAPIBase;
	monRequest.requestedParameters.samplingInterval = 100.0; /* 100 ms interval */
	UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
		monRequest, NULL, jAPIBase->dataChangeNotificationCallback);

	ServerAPIBase::method_output m_output;
	m_output.key = monitoredItemId;
	//	m_output.value = output;
	m_output.api_local = jAPIBase;
	ServerAPIBase::Get()->AddOutput(m_output);
}

UA_NodeId ServerAPIBase::AddObject(UA_Server * server, UA_NodeId requestedNewNodeId, char* name)
{
	UA_NodeId immId; /* get the nodeid assigned by the server */
	UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
	oAttr.displayName = UA_LOCALIZEDTEXT("en-US", name);
	UA_Server_addObjectNode(server, requestedNewNodeId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
		UA_QUALIFIEDNAME(1, name), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
		oAttr, NULL, &immId);
	return immId;
}

UA_NodeId ServerAPIBase::AddObject(UA_Server * server, UA_NodeId parent, UA_NodeId requestedNewNodeId, char * name)
{
	UA_NodeId objId; /* get the nodeid assigned by the server */
	UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
	oAttr.displayName = UA_LOCALIZEDTEXT("en-US", name);
	UA_Server_addObjectNode(server, requestedNewNodeId,
		parent,
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
		UA_QUALIFIEDNAME(1, name), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
		oAttr, NULL, &objId);
	return objId;
}

UA_NodeId ServerAPIBase::AddVariableNode(UA_Server * server, UA_NodeId objectId, UA_NodeId requestedNewNodeId, char * name, UA_Int32 typeId, UA_Int32 accessLevel)
{
	UA_NodeId nodeId;
	UA_VariableAttributes attributes = UA_VariableAttributes_default;
	UA_Variant_setScalar(&attributes.value, UA_new(&UA_TYPES[typeId]), &UA_TYPES[typeId]);
	attributes.displayName = UA_LOCALIZEDTEXT("en-US", name);
	attributes.accessLevel = accessLevel;
	UA_Server_addVariableNode(server, requestedNewNodeId, objectId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, name),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attributes, NULL, &nodeId);
	return nodeId;
}

UA_StatusCode ServerAPIBase::WriteVariable(UA_Server *server, UA_NodeId* nodeId, int  intValue) {
	UA_Variant myVar;
	UA_Variant_init(&myVar);
	UA_Variant_setScalar(&myVar, &intValue, &UA_TYPES[UA_TYPES_INT32]);

	return UA_Server_writeValue(server, (*nodeId), myVar);
}

UA_StatusCode ServerAPIBase::WriteVariable(UA_Server *server, UA_NodeId* nodeId, char * stringValue) {
	UA_Variant myVar;
	UA_Variant_init(&myVar);
	UA_String string_value;
	UA_String_init(&string_value);

	string_value.length = strlen(stringValue);
	string_value.data = (UA_Byte *)stringValue;

	UA_Variant_setScalar(&myVar, &string_value, &UA_TYPES[UA_TYPES_STRING]);
	return UA_Server_writeValue(server, (*nodeId), myVar);
}

UA_StatusCode ServerAPIBase::WriteVariable(UA_Server *server, UA_NodeId* nodeId, double doubleValue) {
	UA_Variant myVar;
	UA_Variant_init(&myVar);
	UA_Variant_setScalar(&myVar, &doubleValue, &UA_TYPES[UA_TYPES_DOUBLE]);
	return UA_Server_writeValue(server, (*nodeId), myVar);
}

UA_NodeId ServerAPIBase::GetDataTypeNode(UA_Int32 typeId)
{
	return UA_TYPES[typeId].typeId;
}

UA_NodeId ServerAPIBase::AddMethod(ServerAPIBase *jAPIBase, UA_Server * server, UA_NodeId objectId, UA_NodeId requestedNewNodeId, UA_Argument inputArgument, UA_Argument outputArgument, UA_MethodAttributes methodAttr)
{
	UA_NodeId nodeId;

	char* methodName = (char*)UA_malloc(sizeof(char)*methodAttr.displayName.text.length + 1);
	memcpy(methodName, methodAttr.displayName.text.data, methodAttr.displayName.text.length);
	methodName[methodAttr.displayName.text.length] = '\0';

		UA_Server_addMethodNode(server, requestedNewNodeId,
		objectId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT),
		UA_QUALIFIEDNAME(1, methodName),
		methodAttr, &(jAPIBase->methodCallback),
		1, &inputArgument, 1, &outputArgument, NULL, &nodeId);


	ServerAPIBase::method_output m_output;
	m_output.key = nodeId;
	//	m_output.value = output;
	m_output.api_local = jAPIBase;
	ServerAPIBase::Get()->AddOutput(m_output);


	return nodeId;
}

UA_NodeId ServerAPIBase::AddArrayMethod(ServerAPIBase *jAPIBase, UA_Server *server, UA_NodeId objectId, UA_NodeId requestedNewNodeId
	, UA_Argument outputArgument, UA_MethodAttributes methodAttr, char * name, char * description, int typeId, UA_UInt32 pDimension){
	UA_NodeId nodeId;

	char* methodName = (char*)UA_malloc(sizeof(char)*methodAttr.displayName.text.length + 1);
	memcpy(methodName, methodAttr.displayName.text.data, methodAttr.displayName.text.length);
	methodName[methodAttr.displayName.text.length] = '\0';

	UA_Argument inputArguments[1];
	UA_Argument_init(&inputArguments[0]);
	inputArguments[0].description = UA_LOCALIZEDTEXT("en-US", description);
	inputArguments[0].name = UA_STRING(name);
	inputArguments[0].dataType = UA_TYPES[typeId].typeId;
	inputArguments[0].valueRank = UA_VALUERANK_ONE_DIMENSION;
	UA_UInt32 pInputDimension = pDimension;
	inputArguments[0].arrayDimensionsSize = 1;
	inputArguments[0].arrayDimensions = &pInputDimension;


	//UA_Argument_copy(&inputArgument, &inputArguments[0]);

	UA_Server_addMethodNode(server, requestedNewNodeId,
		objectId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT),
		UA_QUALIFIEDNAME(1, methodName),
		methodAttr, &(jAPIBase->methodCallback),
		1, &inputArguments[0], 1, &outputArgument, NULL, &nodeId);


	ServerAPIBase::method_output m_output;
	m_output.key = nodeId;
	//	m_output.value = output;
	m_output.api_local = jAPIBase;
	ServerAPIBase::Get()->AddOutput(m_output);

	return nodeId;
}

UA_NodeId ServerAPIBase::ManuallyDefineIMM(UA_Server *server) {
	UA_NodeId statusNodeId;
	UA_NodeId immId; /* get the nodeid assigned by the server */

	immId = AddObject(server, UA_NODEID_NUMERIC(1, 10), "IMM");
	AddVariableNode(server, immId, UA_NODEID_NUMERIC(1, 11), "ManufacturerName", UA_TYPES_STRING, (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));
	AddVariableNode(server, immId, UA_NODEID_NUMERIC(1, 12), "ModelName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
	AddVariableNode(server, immId, UA_NODEID_NUMERIC(1, 13), "MotorRPMs", UA_TYPES_DOUBLE, UA_ACCESSLEVELMASK_READ);
	statusNodeId = AddVariableNode(server, immId, UA_NODEID_NUMERIC(1, 14), "Status", UA_TYPES_STRING, (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));

	WriteVariable(server, &statusNodeId, -1);

	return statusNodeId;
}

UA_NodeId ServerAPIBase::ManuallyDefineRobot(UA_Server * server)
{
	UA_NodeId statusNodeId;
	UA_NodeId robotId; /* get the nodeid assigned by the server */

	robotId = AddObject(server, UA_NODEID_NUMERIC(1, 20), "Robot");
	AddVariableNode(server, robotId, UA_NODEID_NUMERIC(1, 21), "ManufacturerName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
	AddVariableNode(server, robotId, UA_NODEID_NUMERIC(1, 22), "ModelName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
	AddVariableNode(server, robotId, UA_NODEID_NUMERIC(1, 23), "MotorRPMs", UA_TYPES_DOUBLE, UA_ACCESSLEVELMASK_READ);
	statusNodeId = AddVariableNode(server, robotId, UA_NODEID_NUMERIC(1, 24), "Robot_Status", UA_TYPES_INT32, (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));

	WriteVariable(server, &statusNodeId, 0);

	return statusNodeId;
}
