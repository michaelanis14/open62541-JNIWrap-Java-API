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

	int nodeOutputIndex = ServerAPIBase::Get()->getNodeIdIndex(*nodeId);
	if (nodeOutputIndex != -1) {
		ServerAPIBase* serverApi = ServerAPIBase::Get()->methodOutputs[nodeOutputIndex].api_local;
		ServerAPIBase::Get()->methodOutputs[nodeOutputIndex].value = (UA_Variant*)value->value.data;
		serverApi->monitored_itemChanged(nodeId, *(UA_Int32*)value->value.data);
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server Received Notification on Monitored Item ");
	}
	else UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server received notification but coudlnt find the monitored item callback. ");
}
void ServerAPIBase::setData(void *d) {
	ServerAPIBase::Get()->d = d;
}

void *ServerAPIBase::getData() {
	return ServerAPIBase::Get()->d;
}

void ServerAPIBase::setMethodOutput(UA_NodeId methodId, UA_String output)
{
	ServerAPIBase* serverApi = ServerAPIBase::Get();
	int nodeOutputIndex = serverApi->getNodeIdIndex(methodId);
	if (nodeOutputIndex != -1) {
		UA_Variant_setScalarCopy(serverApi->methodOutputs[nodeOutputIndex].value, &output, &UA_TYPES[UA_TYPES_STRING]);
	}
}



UA_StatusCode  ServerAPIBase::methodCallback(UA_Server *server,
	const UA_NodeId *sessionId, void *sessionHandle,
	const UA_NodeId *methodId, void *methodContext,
	const UA_NodeId *objectId, void *objectContext,
	size_t inputSize, const UA_Variant *input,
	size_t outputSize, UA_Variant *output) {
	/*
*/
	
	UA_String *inputStr = (UA_String*)input->data;

	
	//serverApi->output = output;
	int nodeOutputIndex = ServerAPIBase::Get()->getNodeIdIndex(*methodId);
	if(nodeOutputIndex != -1){
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

bool ServerAPIBase::addOutput(method_output output)
{
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "addOutput, id %u \n", jAPIBase_local->outputs_length);


	if(jAPIBase_local->outputs_length != 1){
	method_output* temp = new method_output[jAPIBase_local->outputs_length];
	memcpy(temp, methodOutputs, jAPIBase_local->outputs_length * sizeof(method_output));
	delete[] methodOutputs;
	methodOutputs = temp;
	}
	else {
		jAPIBase_local->methodOutputs = new method_output[jAPIBase_local->outputs_length];
	}
	methodOutputs[jAPIBase_local->outputs_length - 1] = output;
	jAPIBase_local->outputs_length++;
	return true;
}

int ServerAPIBase::getNodeIdIndex(UA_NodeId nodeId)
{
	for (int i = 0; i < outputs_length; i++) {
		if (UA_NodeId_equal(&methodOutputs[i].key, &nodeId)){
			UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "getNodeIdIndex, id %u \n", i);
			return i;
		}
	}
	return -1;
}

UA_Server * ServerAPIBase::createServerDefaultConfig(void)
{
	UA_Server *server = UA_Server_new();
	UA_ServerConfig_setDefault(UA_Server_getConfig(server));
	return server;
}

UA_Server * ServerAPIBase::createServer(char * host, UA_UInt16 port)
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

UA_StatusCode ServerAPIBase::runServer(UA_Server * server)
{
	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);
	ServerAPIBase::Get()->running = true;
	UA_StatusCode retval = UA_Server_run(server, &(ServerAPIBase::Get()->running));
	while (ServerAPIBase::Get()->running)
		UA_Server_run_iterate(server, true);
	return UA_Server_run_shutdown(server);

}

void ServerAPIBase::addMonitoredItem(UA_Server *server, UA_NodeId monitoredItemId, ServerAPIBase *jAPIBase) {
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
	ServerAPIBase::Get()->addOutput(m_output);
}

UA_NodeId ServerAPIBase::addObject(UA_Server * server, const UA_Int32 requestedNewNodeId, char* name)
{
	UA_NodeId immId; /* get the nodeid assigned by the server */
	UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
	oAttr.displayName = UA_LOCALIZEDTEXT("en-US", name);
	UA_Server_addObjectNode(server, UA_NODEID_NUMERIC(1, requestedNewNodeId),
		UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
		UA_QUALIFIEDNAME(1, name), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
		oAttr, NULL, &immId);
	return immId;
}

UA_NodeId ServerAPIBase::addVariableNode(UA_Server * server, UA_NodeId objectId, const UA_Int32 requestedNewNodeId, char * name, UA_Int32 typeId, UA_Int32 accessLevel)
{
	UA_NodeId nodeId;
	UA_VariableAttributes attributes = UA_VariableAttributes_default;
	UA_Variant_setScalar(&attributes.value, UA_new(&UA_TYPES[typeId]), &UA_TYPES[typeId]);
	attributes.displayName = UA_LOCALIZEDTEXT("en-US", name);
	attributes.accessLevel = accessLevel;
	UA_Server_addVariableNode(server, UA_NODEID_NUMERIC(1, requestedNewNodeId), objectId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
		UA_QUALIFIEDNAME(1, name),
		UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attributes, NULL, &nodeId);
	return nodeId;
}

UA_StatusCode ServerAPIBase::writeVariable(UA_Server *server, UA_NodeId* nodeId, int  intValue) {
	UA_Variant myVar;
	UA_Variant_init(&myVar);
	UA_Variant_setScalar(&myVar, &intValue, &UA_TYPES[UA_TYPES_INT32]);

	return UA_Server_writeValue(server, (*nodeId), myVar);
}

UA_StatusCode ServerAPIBase::writeVariable(UA_Server *server, UA_NodeId* nodeId, char * stringValue) {
	UA_Variant myVar;
	UA_Variant_init(&myVar);
	UA_Variant_setScalar(&myVar, stringValue, &UA_TYPES[UA_TYPES_STRING]);
	return UA_Server_writeValue(server, (*nodeId), myVar);
}

UA_StatusCode ServerAPIBase::writeVariable(UA_Server *server, UA_NodeId* nodeId, double doubleValue) {
	UA_Variant myVar;
	UA_Variant_init(&myVar);
	UA_Variant_setScalar(&myVar, &doubleValue, &UA_TYPES[UA_TYPES_DOUBLE]);
	return UA_Server_writeValue(server, (*nodeId), myVar);
}

UA_NodeId ServerAPIBase::getDataTypeNode(UA_Int32 typeId)
{
	return UA_TYPES[typeId].typeId;
}

UA_NodeId ServerAPIBase::addMethod(UA_Server * server, UA_NodeId objectId, const UA_Int32 requestedNewNodeId, UA_Argument inputArgument, UA_Argument outputArgument, UA_MethodAttributes methodAttr, ServerAPIBase *jAPIBase)
{
	UA_NodeId nodeId;
	UA_NodeId reqNodeId;
	char* methodName = (char*)UA_malloc(sizeof(char)*methodAttr.displayName.text.length + 1);
	memcpy(methodName, methodAttr.displayName.text.data, methodAttr.displayName.text.length);
	methodName[methodAttr.displayName.text.length] = '\0';

	//ServerAPIBase::Get()->jAPIBase_local = jAPIBase;
	if (requestedNewNodeId != NULL) {
		reqNodeId = UA_NODEID_NUMERIC(1, requestedNewNodeId);
	}
	else {
		reqNodeId = UA_NODEID_STRING(1, methodName);
	}
	UA_Server_addMethodNode(server, reqNodeId,
		objectId,
		UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT),
		UA_QUALIFIEDNAME(1, methodName),
		methodAttr, &(jAPIBase->methodCallback),
		1, &inputArgument, 1, &outputArgument, NULL, &nodeId);


		ServerAPIBase::method_output m_output;
		m_output.key = nodeId;
	//	m_output.value = output;
		m_output.api_local = jAPIBase;
		ServerAPIBase::Get()->addOutput(m_output);
	

		return nodeId;
}

UA_NodeId ServerAPIBase::manuallyDefineIMM(UA_Server *server) {
	UA_NodeId statusNodeId;
	UA_NodeId immId; /* get the nodeid assigned by the server */

	immId = addObject(server, 10, "IMM");
	addVariableNode(server, immId, 11, "ManufacturerName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
	addVariableNode(server, immId, 12, "ModelName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
	addVariableNode(server, immId, 13, "MotorRPMs", UA_TYPES_DOUBLE, UA_ACCESSLEVELMASK_READ);
	statusNodeId = addVariableNode(server, immId, 14, "Status", UA_TYPES_INT32, (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));

	writeVariable(server, &statusNodeId, -1);

	return statusNodeId;
}

UA_NodeId ServerAPIBase::manuallyDefineRobot(UA_Server * server)
{
	UA_NodeId statusNodeId;
	UA_NodeId robotId; /* get the nodeid assigned by the server */

	robotId = addObject(server, 20, "Robot");
	addVariableNode(server, robotId, 21, "ManufacturerName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
	addVariableNode(server, robotId, 22, "ModelName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
	addVariableNode(server, robotId, 23, "MotorRPMs", UA_TYPES_DOUBLE, UA_ACCESSLEVELMASK_READ);
	statusNodeId = addVariableNode(server, robotId, 24, "Robot_Status", UA_TYPES_INT32, (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));

	writeVariable(server, &statusNodeId, 0);

	return statusNodeId;
}
