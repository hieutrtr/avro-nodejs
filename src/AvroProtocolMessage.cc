#include "AvroProtocolMessage.h"

//
Persistent<Function> AvroProtocolMessage::constructor;

/**
 *
 */
AvroProtocolMessage::AvroProtocolMessage(const char* name, const char* doc, AvroSchema* request, AvroSchema* response, AvroSchema* error){
  name_ = name;
  doc_ = doc;
  request_ = request;
  response_ = response;
  error_ = error;
}

/**
 *
 */
AvroProtocolMessage::~AvroProtocolMessage() {

}


/**
 *
 */
AvroProtocolMessage::Init(Handle<Object> exports){
  //Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("AvroProtocolMessage"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  //Prototype

  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("AvroProtocolMessage"), constructor);

}

/**
 *
 */
AvroProtocolMessage::New(const Arguments& args){
  HandleScope scope;

  if(args.IsConstructCall()){
    const char* argName;
    const char* argDoc;

    AvroSchema* request;
    AvroSchema* response;
    AvroSchema* error;

    argName = args[0]->IsUndefined() ? "": *(String::Utf8Value(args[0]->ToString()));
    argDoc = args[1]->IsUndefined() ? "": *(String::Utf8Value(args[1]->ToString()));

    request = ObjectWrap::Unwrap<AvroSchema>(args[2]);
    response = ObjectWrap::Unwrap<AvroSchema>(args[3]);
    error = ObjectWrap::Unwrap<AvroSchema>(args[4]);

    AvroProtocolMessage* protocolMessage;
    protocolMessage = new AvroProtocolMessage(argName, argDoc, request, response, error);
    protocolMessage->Wrap(args.This());
    return args.This();
  } else {
    const in argc = 1;
    Local<Value> argv[argc] = {args[0]};
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}

Handle<Value> AvroProtocolMessage::GetName(const Arguments& args){
  HandleScope scope;
  AvroProtocolMessage* obj = ObjectWrap::Unwrap<AvroProtocolMessage>(args.This());

  return scope.Close(String::New(obj->name_));

}

Handle<Value> AvroProtocolMessage::GetDoc(const Arguments& args){
  HandleScope scope;
  AvroProtocolMessage* obj = ObjectWrap::Unwrap<AvroProtocolMessage>(args.This());

  return scope.Close(String::New(obj->doc_));
}

Handle<Value> AvroProtocolMessage::GetErrors(const Arguments& args){
  HandleScope scope;
  AvroProtocolMessage* obj = ObjectWrap::Unwrap<AvroProtocolMessage>(args.This());

  return scope.Close(obj->error_);

}

Handle<Value> AvroProtocolMessage::GetRequest(const Arguments& args){
  HandleScope scope;
  AvroProtocolMessage* obj = ObjectWrap::Unwrap<AvroProtocolMessage>(args.This());

  return scope.Close(obj->request_);

}

Handle<Value> AvroProtocolMessage::GetResponse(const Arguments& args){
  HandleScope scope;
  AvroProtocolMessage* obj = ObjectWrap::Unwrap<AvroProtocolMessage>(args.This());

  return scope.Close(obj->response_);

}
