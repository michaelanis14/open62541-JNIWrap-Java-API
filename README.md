# OpcUa_Java_API

This is a JNI Wrap for the C source for open62541 to enable direct Java comunication.
The JNI Wrap is generated using SWIG library

### STATE

first build and bug fixes

### PREREQUISITES

* JDK 1.8_211


### BUILD

The repository can be built either by Cmake using the `CMakeLists.txt` in the root or by compiling and installing each library separately using visual studio 2015.


### Cmake build

```bash
mkdir build
cd build 
cmake .. -DCMAKE_BUILD_TYPE=Release 
cmake --build .
```

Note: with the Cmake build the dependencies are searched for inside OpcUa directory if it is not found a git clone to dependcies -'mbedtls-2.16' is performed and linked automaticaly.

On the other hand, Using visual studio 2015 open .sln however make sure you have the dependcies compiled and listed inside OpcUa directory accordingly.

### Java usage code example 

```bash
SWIGTYPE_p_bool running = open62541.UA_Boolean_new() ;
SWIGTYPE_p_UA_Server server = open62541.UA_Server_new();
open62541.UA_ServerConfig_setDefault(open62541.UA_Server_getConfig(server));
SWIGTYPE_p_uint32_t retval = open62541.UA_Server_run_startup(server);
open62541.UA_Server_delete(server);
```
