#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "v8_avro_base.h"

////////////////////////////////////////////////////////////////////////
//
// Wrap a callback function into a FunctionTemplate, providing the "this"
// pointer to the callback when v8 calls the callback func
//
// \parameter func Static callback to be used in FunctionTemplate
//
// \return Local<FunctionTemplate> containing func
//
////////////////////////////////////////////////////////////////////////
Local<FunctionTemplate> V8AvroBase::makeStaticCallableFunc(InvocationCallback func){
  HandleScope scope;
  Local<FunctionTemplate> funcTemplate = FunctionTemplate::New(func, classPtrToExternal());
  return scope.Close(funcTemplate);
}

////////////////////////////////////////////////////////////////////////
//
// Makes the "this" pointer be an external so that it can be accessed by
// the static callback functions
//
// \return Local<External> containing the "this" pointer
////////////////////////////////////////////////////////////////////////
Local<External> V8AvroBase::classPtrToExternal()
{
    HandleScope scope;
    return scope.Close(External::New(reinterpret_cast<void *>(this)));
}