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

	UA_Server *  creatRunServerDefaultConfig(void) {
		signal(SIGINT, stopHandler);
		 signal(SIGTERM, stopHandler);

		UA_Server *server = UA_Server_new();
		UA_ServerConfig_setDefault(UA_Server_getConfig(server));

		UA_Server_run(server, &running);

		UA_StatusCode retval = UA_Server_run(server, &running);

		UA_Server_delete(server);
		return server;
	}


#endif // OPEN62541_HELPER_