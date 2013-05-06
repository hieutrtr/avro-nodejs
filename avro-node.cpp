
#include "avro-node.h"
#include <fstream>

using namespace v8;
using namespace avro;

ValidSchema schema;

Persistent<String> on_schema;
Persistent<String> on_datum;
Persistent<String> on_error;

Persistent<Object> module_handle;

/**
 * [Initialize description]
 * @param target [description]
 */
void Initialize(Handle<Object> target){

  NODE_SET_METHOD(target, "decode", DecodeFile);
  NODE_SET_METHOD(target, "encode", EncodeFile);
  NODE_SET_METHOD(target, "setSchema", SetSchema);
  NODE_SET_METHOD(target, "getSchema", GetSchema);
  NODE_SET_METHOD(target, "decodeBytes", DecodeBytes);
  NODE_SET_METHOD(target, "encodeBytes", EncodeBytes);

  on_schema = NODE_PSYMBOL("onschema");
  on_datum = NODE_PSYMBOL("ondatum");
  on_error = NODE_PSYMBOL("onerror");

  module_handle = Persistent<Object>::New(target);
}

/**
 * 
 */
Handle<Value> EncodeFile(const Arguments &args) 
{
  HandleScope scope;
  return scope.Close(Undefined());
}

/**
 * 
 */
Handle<Value> EncodeBytes(const Arguments &args) 
{
  HandleScope scope;
  return scope.Close(Undefined());
}

/**
 * 
 */
Handle<Value> DecodeFile(const Arguments &args) 
{  
  HandleScope scope;

  if (args.Length() > 1) {
    ThrowException(v8::Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  if(args[0]->IsString()){
    // get the param
    v8::String::Utf8Value filename(args[0]->ToString());

    try{
      avro::DataFileReader<avro::GenericDatum> dfr(*filename);
      schema = dfr.dataSchema();
      OnSchema();
      avro::GenericDatum datum(dfr.dataSchema());

      while(dfr.read(datum)){
        OnDatum(DecodeAvro(datum));
      }
    }catch(std::exception &e){
      OnError(e.what());
    }
  }else{
    OnError("Wrong Argument. Must be string for filename");
  }

  return scope.Close(Undefined());
}

/**
 * 
 */
Handle<Value> DecodeBytes(const Arguments &args) 
{  
  HandleScope scope;
  return scope.Close(Undefined());
}

/**
 * 
 */
Handle<Value> SetSchema(const Arguments &args) 
{  
  HandleScope scope;

  if (args.Length() != 1) {
    ThrowException(v8::Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  if(args[0]->IsString()){
    // get the param
    v8::String::Utf8Value param1(args[0]->ToString());
    std::ifstream ifs(*param1);
    avro::compileJsonSchema(ifs, schema);
    OnSchema();
  }else{
    ThrowException(v8::Exception::TypeError(String::New("Schema Must be a string")));
    return scope.Close(Undefined());
  }
  return scope.Close(Undefined());
}

/**
 * 
 */
Handle<Value> GetSchema(const Arguments &args) 
{  
  HandleScope scope;
  std::ostringstream oss(std::ios_base::out);
  schema.toJson(oss);
  if(! schema.root()){
    std::cout << "root not defined" << std::endl;
    return scope.Close(Undefined());
  }else{
    return scope.Close(String::New(oss.str().c_str()));
  }
}

/**
 * [OnError description]
 * @param error [description]
 */
void OnError(const char* error){
  HandleScope scope;
  // locate callback from the module context if defined by script
  // example = require('example')
  // example.onschema = function( ... ) { ..
  Local<Value> callback_v = module_handle->Get(on_error);
  //If no function is defined in the javascript for handling error
  // we throw an error. 
  if (!callback_v->IsFunction()) {
    ThrowException(v8::Exception::TypeError(String::New(error)));
    return;
  }

  Local<Function> callback = Local<Function>::Cast(callback_v);

  // prepare arguments for the callback
  Local<Value> argv[1];
  argv[0] = Local<Value>::New(String::New(error));

  callback->Call(module_handle, 1, argv);
}

/**
 * [OnSchema description]
 */
void OnSchema(){
  HandleScope scope;
  // locate callback from the module context if defined by script
  // example = require('example')
  // example.onschema = function( ... ) { ..
  Local<Value> callback_v = module_handle->Get(on_schema);
  if (!callback_v->IsFunction()) {
    // callback not defined, ignore
    return;
  }

  Local<Function> callback = Local<Function>::Cast(callback_v);

  std::ostringstream oss(std::ios_base::out);
  schema.toJson(oss);
  // prepare arguments for the callback
  Local<Value> argv[1];
  argv[0] = Local<Value>::New(String::New(oss.str().c_str()));

  callback->Call(module_handle, 1, argv);
}

/**
 * [OnDatum description]
 * @param datum [description]
 */
void OnDatum(Handle<Value> datum){
    HandleScope scope;
  // locate callback from the module context if defined by script
  // example = require('example')
  // example.onschema = function( ... ) { ..
  Local<Value> callback_v = module_handle->Get(on_datum);
  if (!callback_v->IsFunction()) {
    // callback not defined, ignore
    return;
  }

  Local<Function> callback = Local<Function>::Cast(callback_v);

  // prepare arguments for the callback
  Local<Value> argv[1];
  argv[0] = Local<Value>::New(datum);

  callback->Call(module_handle, 1, argv);
}

/**
 * converts a GenericDatum into a v8 object that can be passed back to javascript
 */
Handle<Value> DecodeAvro(const avro::GenericDatum& datum){
    Handle<Object> obj = Object::New();

  //return this Object
  if(datum.type() == avro::AVRO_RECORD){
    const avro::GenericRecord& record = datum.value<avro::GenericRecord>();
    const avro::NodePtr& node = record.schema();
    Handle<Object> obj = Object::New();
    for(uint i = 0; i<record.fieldCount(); i++){
      //Add values
      Local<String> datumName = String::New(node->nameAt(i).c_str(), node->nameAt(i).size());
      const avro::GenericDatum& subDatum = record.fieldAt(i);
      obj->Set(datumName, DecodeAvro(subDatum));
    }
    return obj;
  }else if(datum.type() == avro::AVRO_STRING){
    return  String::New(
      datum.value<std::string>().c_str(),
      datum.value<std::string>().size()
    );
  }else if(datum.type() == avro::AVRO_BYTES){
    //obj->Set(datumName, );
  }else if(datum.type() == avro::AVRO_INT){
    return Number::New(datum.value<int>());
  }else if(datum.type() == avro::AVRO_LONG){
    return Number::New(datum.value<long>());
  }else if(datum.type() == avro::AVRO_FLOAT){
    return Number::New(datum.value<float>());
  }else if(datum.type() == avro::AVRO_DOUBLE){
    return Number::New(datum.value<double>());
  }else if(datum.type() == avro::AVRO_BOOL){
    return Boolean::New(datum.value<bool>());
  }else if(datum.type() == avro::AVRO_NULL){
    return v8::Null();
  }else if(datum.type() == avro::AVRO_RECORD){
    return DecodeAvro(datum.value<avro::GenericDatum>());
  }else if(datum.type() == avro::AVRO_ENUM){
    return v8::Null();
  }else if(datum.type() == avro::AVRO_ARRAY){
    const avro::GenericArray &genArray = datum.value<avro::GenericArray>();

    const std::vector<avro::GenericDatum> &v = genArray.value();
    Local<Array> datumArray = Array::New();
    int i = 0;
    for(std::vector<avro::GenericDatum>::const_iterator it = v.begin(); it != v.end(); ++it) {
      const avro::GenericDatum &itDatum = * it;
      datumArray->Set(i, DecodeAvro(itDatum));
      i++;
    }
    return datumArray;
  }else if(datum.type() == avro::AVRO_MAP){
    const avro::GenericMap &genMap = datum.value<avro::GenericMap>();

    const std::vector < std::pair < std::string, avro::GenericDatum > > &v = genMap.value();
    Local<Array> datumArray = Array::New();
    int i = 0;
    for(std::vector< std::pair < std::string, avro::GenericDatum> >::const_iterator it = v.begin(); it != v.end(); ++it) {
      const std::pair < std::string, avro::GenericDatum> &itDatum = * it;
      datumArray->Set(String::New(
        itDatum.first.c_str(),
        itDatum.first.size()
        ),DecodeAvro(itDatum.second));

      i++;
    }
    return datumArray;
  }else if(datum.type() == avro::AVRO_UNION){
    std::cout << "in union" << std::endl;
  }else if(datum.type() == avro::AVRO_FIXED){

  }else if(datum.type() == avro::AVRO_NUM_TYPES){

  }else if(datum.type() == avro::AVRO_SYMBOLIC){

  }else if(datum.type() == avro::AVRO_UNKNOWN){

  }
  return obj;
}
