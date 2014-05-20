#ifndef AVROSCHEMA_H 
#define AVROSCHEMA_H

#include <node.h>
#include <avro/ValidSchema.hh>
#include <avro/Compiler.hh>
#include <sstream>
#include "SymbolMapSchema.h"

using namespace v8;
using namespace node;
using namespace avro;

class AvroSchema: public node::ObjectWrap {

  public: 
    static void Init(Handle<Object> exports);
    NodePtr root() const { return schema_->root(); }
    AvroSchema(const char* jsonSchema);
    AvroSchema(const NodePtr &root);
    AvroSchema(const NodePtr &root, SymbolMap &symbols);
    Handle<Value> ToNode();
    ~AvroSchema();

  private: 

    static Handle<Value> ToJson(const Arguments& args);
    static Handle<Value> New(const Arguments& args);
    static Persistent<Function> constructor;
    SymbolMapSchema *schema_;

};

#endif /* AVROSCHEMA_H */
