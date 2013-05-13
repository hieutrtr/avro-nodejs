#ifndef avronode_Test_h
#define avronode_Test_h

#include <node.h>
#include "buffer.h"
#include <v8.h>
#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>
#include <avro/DataFile.hh>
#include <avro/Compiler.hh>
#include <stdlib.h>

using namespace v8;
using namespace avro;

namespace node {

class Avro : ObjectWrap {
public:
  static Handle<Value> New(const Arguments& args);
  Buffer *buffer_;
  ValidSchema schema_;
  static Handle<Value> DecodeFile(const Arguments &args);
  static Handle<Value> DecodeBytes(const Arguments &args);
  static Handle<Value> EncodeFile(const Arguments &args);
  static Handle<Value> EncodeBytes(const Arguments &args);
  static Handle<Value> SetSchema(const Arguments &args);
  static Handle<Value> GetSchema(const Arguments &args);
  static Handle<Value> DecodeAvro(const avro::GenericDatum& datum);

protected:

private: 
  Avro();
  ~Avro();
  static Persistent<Function> constructor;

};

void Initialize(Handle<Object> target);
void Init(Handle<Object> target);
void OnSchema(const char* schema);
void OnError(const char* schema);
void OnDatum(Handle<Value> datum);
}

#endif // avronode_Test_h
