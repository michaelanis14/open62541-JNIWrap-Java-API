

UA_Int32 void2int(jlong v) {
	return *(UA_Int32*)v;
}

const char *void2str(jlong v) {
	UA_String input = *(UA_String*)v;
	char* $2 = (char*)UA_malloc(sizeof(char)*(&input)->length + 1);
	memcpy($2, (&input)->data, (&input)->length);
	$2[(&input)->length] = '\0';
	return $2;
}


UA_Boolean IsVariantType_Int(UA_Variant* variant) {
	if (variant->type == &UA_TYPES[UA_TYPES_INT32]) {
		return true; // One way of returning int32_t via void*!
	}
	return false;
}

UA_Boolean IsVariantType_String(UA_Variant* variant) {
	if (variant->type == &UA_TYPES[UA_TYPES_STRING]) {
		return true; // One way of returning int32_t via void*!
	}
	return false;
}