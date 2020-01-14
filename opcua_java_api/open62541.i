/* File : open62541.i */
%include <arrays_java.i>
%module(directors="1") open62541
%{
#include "open62541.h"
#include "server_api_base.h"
#include "client_api_base.h"
#include "types_map.h"
%}
%include "typemaps.i"
%include "stdint.i"

/* %include "ignoreConstants.i" */

%pragma(java) jniclasscode=%{
    static {
        try {
            System.out.println("Looking for native lib");
            loadNativeLib();    //change the library in this method depending on your platform
            System.out.println("Found native lib");
        } catch (IOException e) {
            System.out.println("Cannot find native lib");
            e.printStackTrace();
        }
    }

    private static void loadNativeLib() throws IOException {
        String libName;
        if (System.getProperty("os.name").startsWith("Windows")) {
            libName = "opcua_java_api.dll"; //use this on windows (needs 32 bit java)
        } else {
            libName = "libOpcua-Java-API_hf.so"; //use this on BrickPi, use the one w/o _hf suffix on ev3
        }
        URL url = open62541JNI.class.getResource("/" + libName);
        File tmpDir = Files.createTempDirectory("my-native-lib").toFile();
        tmpDir.deleteOnExit();
        File nativeLibTmpFile = new File(tmpDir, libName);
        nativeLibTmpFile.deleteOnExit();
        try (InputStream in = url.openStream()) {
            Files.copy(in, nativeLibTmpFile.toPath());
        } catch (Exception e) {
            System.out.println("Error in loadNativeLib");
            e.printStackTrace();
        }
        System.load(nativeLibTmpFile.getAbsolutePath());
    }
%}


%typemap(out) UA_Boolean = bool;
%typemap(in) UA_Boolean = bool;
%typemap(in) UA_Int32 = int;
%typemap(out) UA_Int32 = int;


%typemap(memberin)Identifier {
 
if ($input) ($1).numeric = (*$input).numeric;
if ($input) ($1).byteString = (*$input).byteString;
if ($input) ($1).guid = (*$input).guid;
if ($input) ($1).string = (*$input).string;
};

%typemap(in) UA_String {
 const char *nativeString = (jenv)->GetStringUTFChars($input, 0);
$1 = UA_STRING((char *)nativeString);
};
%typemap(out) UA_String {
	char* $2 = (char*)UA_malloc(sizeof(char)*$1.length + 1);
	memcpy($2, $1.data, $1.length);
	$2[$1.length] = '\0';
	//strcpy($2, "123456789"); // with the null terminator the string adds up to 10 bytes
	$result = (jenv)->NewStringUTF($2);	
};

%typemap(directorin,descriptor="Ljava/lang/String;") UA_String 
%{ 
{
	char* $2 = (char*)UA_malloc(sizeof(char)*$1.length + 1);
	memcpy($2, $1.data, $1.length);
	$2[$1.length] = '\0';
	//strcpy($2, "123456789"); // with the null terminator the string adds up to 10 bytes
	$input = (jenv)->NewStringUTF($2);	
}  
  %};

%typemap(jstype) void* "java.lang.Object" ;


%typemap(javaout) void* {
  long cPtr = $jnicall;
   Object result = null;
   if (open62541.IsVariantType_Int(this)) {
    result = open62541.void2int(cPtr);
  }
 else if (open62541.IsVariantType_String(this)) {
    result = open62541.void2str(cPtr);
  }

   return result;
}
;




%ignore UA_Variant_setScalarCopy;
%ignore UA_DEPRECATED;
%ignore UA_Client_close;
%ignore UA_Client_manuallyRenewSecureChannel;
%ignore UA_Client_runAsync;
%ignore UA_sleep_ms;
%ignore UA_INT32_MIN;
%ignore define;
%ignore p_instance;
%ignore clientConfig;
%ignore subscriptionInactivityCallback;
%ignore deleteSubscriptionCallback;
%ignore dataChangeNotificationCallback;
%ignore methodCallback;
%ignore output;
%ignore handler_TheStatusChanged;
%ignore methodOutputs;
%ignore getNodeIdIndex;
%ignore addOutput;
%ignore method_output;
%ignore AddOutput;
%ignore outputs_length;
%ignore GetSubIdIndex;
%ignore GetNodeIdIndex;
typedef int UA_StatusCode;
typedef  boolean UA_Boolean;
typedef  int UA_UInt32;
typedef  int int32_t;
typedef  int UA_Int32,UA_UInt16;
typedef jstring UA_String;


%feature("director") ServerAPIBase;

%include "server_api_base.h"
%feature("director") ClientAPIBase;
%include "client_api_base.h"
%include "types.h"
%include "types_map.h"
%include cpointer.i
%pointer_functions(int, intp);