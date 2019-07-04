# OpcUa_Java_API

This is a JNI Wrap for a helper C classes over the source of open62541 to enable direct Java comunication.
The JNI Wrap is generated using SWIG library.

This approach was a bit cleaner for the mapping, for using the nessecary functions or even bulding custome ones and just calling them from the Java side. Thus you can refrence any functions in the c helper class or even add other classes and the cmakelist will include them autmaticaly in the build.

### STATE

first build and bug fixes.

### TODO

- [ ] let the cmakelist generate the java classes duing the build.
- [ ] package the generated java classes into a single jar.
- [ ] Add more generaric methods in the helper classes ex. init a server with custome port and ip.

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
  ServerAPIBase	serverAPI = new ServerAPIBase();
	SWIGTYPE_p_UA_Servers erver = serverAPI.createServerDefaultConfig();
	UA_NodeId	statusNodeID = serverAPI.manuallyDefineIMM(server);
	serverAPI.addMonitoredItem(new MoldingMachine_OPCUA(), server, statusNodeID);
```
