#include <node.h>
#include "AvroFileReader.h"

using namespace v8;
using namespace avro;
using namespace node;


Persistent<Function> AvroFileReader::constructor;

/**
 *
 * AvroFileReader is a v8/node wrapper for Avro's DataFileReader
 * class. simple features are; open, read and close. 
 *
 * @param filename: The name of the file to be decoded.
 */
AvroFileReader::AvroFileReader(const char* filename) {
  try{
    auto_ptr<DataFileReaderBase> readerPt(new DataFileReaderBase(filename));
    reader_ = new DataFileReader<GenericDatum>(readerPt);
    schema_ = reader_->dataSchema();
  }catch(exception &e){
    ThrowException(String::New(e.what()));
  }
}

AvroFileReader::~AvroFileReader() {

}

void AvroFileReader::Init(Handle<Object> exports){
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("AvroFileReader"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  //Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "read", AvroFileReader::Read);
  NODE_SET_PROTOTYPE_METHOD(tpl, "close", AvroFileReader::Close);

  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("AvroFileReader"), constructor);
 
}

/**
 * constructs a new AvroFileReader 
 *@param filename (an avro data file)
 *
 */
Handle<Value> AvroFileReader::New(const Arguments& args){
  HandleScope scope;

  if(args.IsConstructCall()){
    String::Utf8Value fileName(args[0]->ToString());
    AvroFileReader* reader;

    const char* filename = args[0]->IsUndefined() ? "": *(fileName);
    reader = new AvroFileReader(filename);
    reader->Wrap(args.This());
    return args.This();
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = {args[0]};
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}

/**
 * If there are more data to read in the file. we will grab the next
 * datum. Else we will return nothing.
 *
 * TODO possible throw EOF if we try to read past the end. 
 */
Handle<Value> AvroFileReader::Read(const Arguments& args){
  HandleScope scope;

  AvroFileReader * obj = ObjectWrap::Unwrap<AvroFileReader>(args.This());

  //TODO construct Generic Datum here
  GenericDatum datum(obj->schema_);
  if(obj->reader_->read(datum)){
    return scope.Close(DecodeAvro(datum));
  }else{
    //TODO throw an exception
  }

  //convert to javascript object
  return scope.Close(Undefined());
}
/**
 *
 * Closes the underlining DataFileReader
 *
 */
Handle<Value> AvroFileReader::Close(const Arguments& args){
  HandleScope scope;

  AvroFileReader* obj = ObjectWrap::Unwrap<AvroFileReader>(args.This());

  obj->reader_->close();

  return scope.Close(Undefined());
}
