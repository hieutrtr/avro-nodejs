#include <node.h>
#include <avro/DataFile.hh>
#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>
#include <avro/Compiler.hh>
#include "translate.h"

using namespace v8;
using namespace node;
using namespace avro;


class AvroFileWriter : public node::ObjectWrap {

  public:
    static void Init(Handle<Object> exports);

  private:
    AvroFileWriter(const char* filename, const char* jsonSchema);
    ~AvroFileWriter();

    static Handle<Value> Write(const Arguments& args);
    static Handle<Value> Close(const Arguments& args);
    static Handle<Value> New(const Arguments& args);
    static Persistent<Function> constructor;
    DataFileWriter<GenericDatum> *writer_;
    ValidSchema schema_;

};
