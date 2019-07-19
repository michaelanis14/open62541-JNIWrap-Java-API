#include "server_api_base.h"





ServerAPIBase * ServerAPIBase::jAPIBase_local = 0;

static void dataChangeNotificationCallback(UA_Server *server, UA_UInt32 monitoredItemId,
		void *monitoredItemContext, const UA_NodeId *nodeId,
		void *nodeContext, UA_UInt32 attributeId,
		const UA_DataValue *value) {

		// ServerAPIBase::Get()->monitored_itemChanged(nodeId, value);
		
		ServerAPIBase::Get()->monitored_itemChanged(nodeId, *(UA_Int32*)value->value.data);
		
		UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server Received Notification on Monitored Item ");
	}
void ServerAPIBase::setData(void *d) {
	ServerAPIBase::Get()->d = d;
}

void *ServerAPIBase::getData() {
	return ServerAPIBase::Get()->d;
}

void ServerAPIBase::setMethodOutput(UA_String output)
{
	UA_Variant_setScalarCopy(ServerAPIBase::Get()->output, &output, &UA_TYPES[UA_TYPES_STRING]);
}



static UA_StatusCode methodCallback(UA_Server *server,
	const UA_NodeId *sessionId, void *sessionHandle,
	const UA_NodeId *methodId, void *methodContext,
	const UA_NodeId *objectId, void *objectContext,
	size_t inputSize, const UA_Variant *input,
	size_t outputSize, UA_Variant *output) {

	
	UA_String *inputStr = (UA_String*)input->data;

	char* inputChar = (char*)UA_malloc(sizeof((char)inputStr->length) + 1);
	memcpy(inputChar, &inputStr->data, inputStr->length);
	inputChar[inputStr->length] = '\0';

	UA_String tmp = UA_STRING_ALLOC("Hello ");
	if (inputStr->length > 0) {
		tmp.data = (UA_Byte *)UA_realloc(tmp.data, tmp.length + inputStr->length);
		memcpy(&tmp.data[tmp.length], inputStr->data, inputStr->length);
		tmp.length += inputStr->length;
	}
	
	//UA_Variant_setScalarCopy(output, &tmp, &UA_TYPES[UA_TYPES_STRING]);
	UA_String_clear(&tmp);
	ServerAPIBase* serverApi = ServerAPIBase::Get();
	serverApi->output = output;
	
	serverApi->methods_callback(serverApi, methodId, objectId, *inputStr, *inputStr);

	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Hello World was called");
	
	return UA_STATUSCODE_GOOD;
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

	void ServerAPIBase::addMonitoredItem( UA_Server *server, UA_NodeId immId, ServerAPIBase *jAPIBase) {
		UA_MonitoredItemCreateRequest monRequest =
			UA_MonitoredItemCreateRequest_default(immId);
		jAPIBase_local = jAPIBase;
		monRequest.requestedParameters.samplingInterval = 100.0; /* 100 ms interval */
		UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
			monRequest, NULL, dataChangeNotificationCallback);
	}

	UA_NodeId ServerAPIBase::addObject(UA_Server * server, char* name)
	{
		UA_NodeId immId; /* get the nodeid assigned by the server */
		UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
		oAttr.displayName = UA_LOCALIZEDTEXT("en-US", name);
		UA_Server_addObjectNode(server, UA_NODEID_NULL,
			UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
			UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
			UA_QUALIFIEDNAME(1, name), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
			oAttr, NULL, &immId);
		return immId;
	}

	UA_NodeId ServerAPIBase::addVariableNode(UA_Server * server, UA_NodeId objectId, char * name, UA_Int32 typeId, UA_Int32 accessLevel)
	{
		UA_NodeId nodeId;
		UA_VariableAttributes attributes = UA_VariableAttributes_default;
		UA_Variant_setScalar(&attributes.value, UA_new(&UA_TYPES[typeId]), &UA_TYPES[typeId]);
		attributes.displayName = UA_LOCALIZEDTEXT("en-US", name);
		attributes.accessLevel = accessLevel;
		UA_Server_addVariableNode(server, UA_NODEID_NULL, objectId,
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

	void ServerAPIBase::addMethod(UA_Server * server, UA_Argument inputArgument, UA_Argument outputArgument, UA_MethodAttributes methodAttr, ServerAPIBase *jAPIBase)
	{
		char* methodName = (char*)UA_malloc(sizeof(char)*methodAttr.displayName.text.length + 1);
		memcpy(methodName, methodAttr.displayName.text.data, methodAttr.displayName.text.length);
		methodName[methodAttr.displayName.text.length] = '\0';

		ServerAPIBase::Get()->jAPIBase_local = jAPIBase;
		UA_Server_addMethodNode(server, UA_NODEID_STRING(1, methodName),
			UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASORDEREDCOMPONENT),
			UA_QUALIFIEDNAME(1, methodName),
			methodAttr, &methodCallback,
			1, &inputArgument, 1, &outputArgument, NULL, NULL);
	}
	
	UA_NodeId ServerAPIBase::manuallyDefineIMM(UA_Server *server) {
		UA_NodeId statusNodeId;
		UA_NodeId immId; /* get the nodeid assigned by the server */

		immId = addObject(server, "IMM");
		addVariableNode(server, immId, "ManufacturerName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
		addVariableNode(server, immId, "ModelName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
		addVariableNode(server, immId, "MotorRPMs", UA_TYPES_DOUBLE, UA_ACCESSLEVELMASK_READ);
		statusNodeId = addVariableNode(server, immId, "Status", UA_TYPES_STRING, (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));
		
		writeVariable(server, &statusNodeId, -1);
		return statusNodeId;
	}

	UA_NodeId ServerAPIBase::manuallyDefineRobot(UA_Server * server)
	{
		UA_NodeId statusNodeId;
		UA_NodeId immId; /* get the nodeid assigned by the server */

		immId = addObject(server, "IMM");
		addVariableNode(server, immId, "ManufacturerName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
		addVariableNode(server, immId, "ModelName", UA_TYPES_STRING, UA_ACCESSLEVELMASK_READ);
		addVariableNode(server, immId, "MotorRPMs", UA_TYPES_DOUBLE, UA_ACCESSLEVELMASK_READ);
		statusNodeId = addVariableNode(server, immId, "Status", UA_TYPES_STRING, (UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE));
		//int number = -1;
		writeVariable(server, &statusNodeId, -1);
		return statusNodeId;
	}
