#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>
#include <avro/Specific.hh>
#include <vector>
#include "helpers.h"

using namespace v8;
using namespace avro;
using namespace std;

void unionBranch(GenericDatum *datum, const char *type);

GenericDatum DecodeV8(GenericDatum datum, Local<Value> object);
GenericDatum DecodeV8(GenericDatum datum, Local<Value> object, std::vector<int> *reference);

Handle<Value> DecodeAvro(const GenericDatum& datum);
Handle<Value> DecodeAvro(const GenericDatum& datum, Local<Array> reference);
