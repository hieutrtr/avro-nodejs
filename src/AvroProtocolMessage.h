#ifndef AVROPROTOCOLMESSAGE_H
#define AVROPROTOCOLMESSAGE_H

#include <node.h>
#include <avro/ValidSchema.hh>
#include "helpers.h"
#include "AvroSchema.h"

using namespace v8;
using namespace node;
using namespace avro;

class AvroProtocolMessage: public ObjectWrap {

  public:
    static void Init(Handle<Object> exports);
    
  private:
    AvroProtocolMessage(const char* name, const char* doc, AvroSchema* request, AvroSchema* response, AvroSchema* error);
    ~AvroProtocolMessage();

    static Handle<Value> New(const Arguments& args);
    static Handle<Value> GetName(const Arguments& args);
    static Handle<Value> GetDoc(const Arguments& args);
    static Handle<Value> GetErrors(const Arguments& args);
    static Handle<Value> GetRequest(const Arguments& args);
    static Handle<Value> GetResponse(const Arguments& args);
    static Persistent<Function> constructor;

    const char* name_;
    const char* doc_;
    AvroSchema* request_;
    AvroSchema* response_;
    AVroSchema* error_;

}
