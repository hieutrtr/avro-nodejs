#ifndef avronode_Test_h
#define avronode_Test_h

#include <node.h>
#include <v8.h>
#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>
#include <avro/DataFile.hh>
#include <avro/Compiler.hh>

using namespace v8;
using namespace avro;

void Initialize(Handle<Object> target);

Handle<Value> EncodeFile(const Arguments &args);
Handle<Value> EncodeBytes(const Arguments &args);
Handle<Value> DecodeFile(const Arguments &args);
Handle<Value> DecodeBytes(const Arguments &args);
Handle<Value> SetSchema(const Arguments &args);
Handle<Value> GetSchema(const Arguments &args);

void OnError(const char* error);
void OnSchema();
void OnDatum(Handle<Value> datum);

Handle<Value> DecodeAvro(const GenericDatum& datum);

#endif // avronode_Test_h
