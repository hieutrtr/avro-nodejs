#include <node.h>
#include <avro/DataFile.hh>
#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>
#include "translate.h"

using namespace v8;
using namespace node;
using namespace avro;


class AvroFileReader: public node::ObjectWrap {

  public:
    static void Init(Handle<Object> exports);

  private:
    AvroFileReader(const char* filename);
    ~AvroFileReader();

    static Handle<Value> Read(const Arguments& args);
    static Handle<Value> Close(const Arguments& args);
    static Handle<Value> New(const Arguments& args);
    static Persistent<Function> constructor;
    DataFileReader<GenericDatum> *reader_;
    ValidSchema schema_;

};
