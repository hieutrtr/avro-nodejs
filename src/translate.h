#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>
#include <avro/Specific.hh>
#include <vector>
#include <sstream>
#include "helpers.h"

#ifndef TRANSLATE_H
#define TRANSLATE_H

using namespace v8;
using namespace avro;
using namespace std;


struct MissingDatumFieldException : public std::exception
{
  std::string s;
  MissingDatumFieldException(std::string ss) : s(ss) {}
  ~MissingDatumFieldException() throw() {}
  const char* what() const throw() { return s.c_str(); }
};

void unionBranch(GenericDatum *datum, const char *type);

GenericDatum DecodeV8(GenericDatum datum, Local<Value> object);
GenericDatum DecodeV8(GenericDatum datum, Local<Value> object, std::vector<int> *reference);

Handle<Value> DecodeAvro(const GenericDatum& datum);
Handle<Value> DecodeAvro(const GenericDatum& datum, Local<Array> reference);

#endif /* TRANSLATE_H */

