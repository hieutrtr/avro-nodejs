#ifndef AVROPROTOCOL_H
#define AVROPROTOCOL_H

#include <node.h>
#include <avro/ValidSchema.hh>
#include "helpers.h"
#include "AvroSchema.h"
#include "SymbolMapSchema.h"


using namespace v8;
using namespace node;
using namespace avro;

class AvroProtocol: public ObjectWrap {

  public:
    static void Init(Handle<Object> exports);
    Handle<Value> SetTypes(AvroSchema *types);

  private:
    AvroProtocol(std::string name, std::string nameSpace, std::string doc);
    ~AvroProtocol();

    static Handle<Value> New(const Arguments& args);
    static Handle<Value> CreateMessage(const Arguments& args);
    static Handle<Value> GetDoc(const Arguments& args);
    static Handle<Value> GetMessages(const Arguments& args);
    static Handle<Value> GetName(const Arguments& args);
    static Handle<Value> GetNamespace(const Arguments& args);
    static Handle<Value> GetType(const Arguments& args);
    static Handle<Value> GetTypes(const Arguments& args);
    static Handle<Value> GetMD5(const Arguments& args);
    static Handle<Value> SetTypes(const Arguments& args);
    static Persistent<Function> constructor;

    std::string name_;
    std::string namespace_;
    std::string doc_;

    helper::SymbolMap dictionary_;

};

#endif /* AVROPROTOCOL_H */
