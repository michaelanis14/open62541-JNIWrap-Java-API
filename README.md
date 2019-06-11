# OpcUa_Java_API

This is a JNI Wrap for the C source for open62541 to enable direct Java comunication.
The JNI Wrap is generated using SWIG library

### STATE

first build and bug fixes

### PREREQUISITES

* JDK 1.8_211


### BUILD

Using visual studio 2015 open .sln

### Java Code example 

```bash
SWIGTYPE_p_bool running = open62541.UA_Boolean_new() ;
SWIGTYPE_p_UA_Server server = open62541.UA_Server_new();
open62541.UA_ServerConfig_setDefault(open62541.UA_Server_getConfig(server));
SWIGTYPE_p_uint32_t retval = open62541.UA_Server_run_startup(server);
open62541.UA_Server_delete(server);
```
