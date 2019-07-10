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

	/**
	* Method:    Get
	* FullName:  Get
	* Access:    public
	* * @return   ServerAPIBase
	*/
	ServerAPIBase * ServerAPIBase::Get()
	{
		if (jAPIBase_local == 0) {
			jAPIBase_local = new ServerAPIBase();
		}

		return jAPIBase_local;
	}

	void ServerAPIBase::addMonitoredItem(ServerAPIBase *jAPIBase, UA_Server *server, UA_NodeId immId) {
		UA_MonitoredItemCreateRequest monRequest =
			UA_MonitoredItemCreateRequest_default(immId);
		jAPIBase_local = jAPIBase;
		monRequest.requestedParameters.samplingInterval = 100.0; /* 100 ms interval */
		UA_Server_createDataChangeMonitoredItem(server, UA_TIMESTAMPSTORETURN_SOURCE,
			monRequest, NULL, dataChangeNotificationCallback);
	}
