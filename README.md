# OPCUA Java Wrapper

This is a JNI Wrap for a helper C class(s) over the source of open62541 to enable direct Java comunication.
The JNI Wrap is generated using SWIG library.

This approach was a bit cleaner for the mapping, for using the nessecary functions or even bulding custome ones and just calling them from the Java side. Thus you can refrence any functions in the c helper class or even add other classes and the cmakelist will include them autmaticaly in the build.

However one can still wrap the whole open62541 C library source, but this approach is not recommended.

### STATE

first build and bug fixes.

### TODO

- [ ] let the cmakelist generate the java classes duing the build.
- [ ] better cmakelist options for example build server files only.
- [ ] package the generated java classes into a single jar.

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
 serverAPI = new ServerAPIBase();
 server = serverAPI.createServer(4840, "localhost");
 
 UA_NodeId type = new UA_NodeId();
 type.setIdentifierType(UA_NodeIdType.UA_NODEIDTYPE_NUMERIC);
 
 UA_NodeId object = serverAPI.addObject(server, "OPCUA Object");
 int accessRights = open62541.UA_ACCESSLEVELMASK_WRITE | open62541.UA_ACCESSLEVELMASK_READ;
 serverAPI.addVariableNode(server, object, "Hello Variable from Java", open62541.UA_TYPES_STRING, accessRights);
	
```
