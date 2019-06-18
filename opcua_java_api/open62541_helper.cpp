#include "..\..\..\..\..\..\..\build\swing\open62541_helper.h"





ServerAPIBase* ServerAPIBase::p_instance = 0;
 ServerAPIBase* ServerAPIBase::Get() {
	if (!ServerAPIBase::p_instance) {
		ServerAPIBase::p_instance = new ServerAPIBase();
	}
	return ServerAPIBase::p_instance;
}
 static void dataChangeNotificationCallback(UA_Server *server, UA_UInt32 monitoredItemId,
	 void *monitoredItemContext, const UA_NodeId *nodeId,
	 void *nodeContext, UA_UInt32 attributeId,
	 const UA_DataValue *value) {

	 ServerAPIBase::Get()->monitored_itemChanged(nodeId, value);
	 UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Server Received Notification on Monitored Item ");
 }

 void ServerAPIBase::addMonitoredItemToCurrentTimeVariable(UA_Server *server, UA_NodeId immId) {
	 UA_MonitoredItemCreateRequest monRequest =
		 UA_MonitoredItemCreateRequest_default(immId);

	 monRequest.requestedParameters.samplingInterval = 100.0; /* 100 ms interval */
	 UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
		 monRequest, NULL, dataChangeNotificationCallback);
 }