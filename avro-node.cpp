
# include "avro-node.h"
# include <fstream>
# include <pthread.h>


using namespace v8;
using namespace node;

Persistent<String> on_schema;
Persistent<String> on_datum;
Persistent<String> on_error;

Persistent<Object> module_handle;

Avro::Avro() {};
Avro::~Avro() {};

Persistent<Function> Avro::constructor;

/**
 * [Initialize description]
 * @param target [description]
 */
void Init(Handle<Object> target){
  HandleScope scope;

  Local<FunctionTemplate> a_temp = FunctionTemplate::New(Avro::New);

  a_temp->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(a_temp, "decode", Avro::DecodeFile);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "encode", Avro::EncodeFile);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "setSchema", Avro::SetSchema);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "getSchema", Avro::GetSchema);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "decodeBytes", Avro::DecodeBytes);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "encodeBytes", Avro::EncodeBytes);

  a_temp->SetClassName(String::NewSymbol("Avro"));
  target->Set(String::NewSymbol("Avro"), a_temp->GetFunction());

  on_schema = NODE_PSYMBOL("onschema");
  on_datum = NODE_PSYMBOL("ondatum");
  on_error = NODE_PSYMBOL("onerror");

  module_handle = Persistent<Object>::New(target);

}

Handle<Value> Avro::New(const Arguments& args){
  HandleScope scope;

  Avro *obj = new Avro();
  obj->buffer_ = new Buffer();
  obj->Wrap(args.This());

  return args.This();
}

/**
 * 
 */
Handle<Value> Avro::EncodeFile(const Arguments &args) {
  HandleScope scope;
  Avro * obj = ObjectWrap::Unwrap<Avro>(args.This());

  return scope.Close(Undefined());
}

/**
 * 
 */
Handle<Value> Avro::EncodeBytes(const Arguments &args) {
  HandleScope scope;
  return scope.Close(Undefined());
}

/**
 * 
 */
Handle<Value> Avro::DecodeFile(const Arguments &args) {  
  HandleScope scope;
  Avro * obj = ObjectWrap::Unwrap<Avro>(args.This());

  if (args.Length() > 1) {
    ThrowException(v8::Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  if(args[0]->IsString()){
    // get the param
    v8::String::Utf8Value filename(args[0]->ToString());

    try{
      avro::DataFileReader<avro::GenericDatum> dfr(*filename);
      obj->schema_ = dfr.dataSchema();
      std::ostringstream oss(std::ios_base::out);
      obj->schema_.toJson(oss);
      OnSchema(oss.str().c_str());
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
Handle<Value> Avro::DecodeBytes(const Arguments &args) {  
  HandleScope scope;
  Avro * avro = ObjectWrap::Unwrap<Avro>(args.This());

  if (args.Length() > 1) {
    OnError("Wrong number of arguments");
    return scope.Close(Undefined());
  }

  if(args[0]->IsObject()){
    Local<Object> obj = args[0]->ToObject();
    //get length of array
    int len = obj->GetIndexedPropertiesExternalArrayDataLength();
    uint8_t *buffer = (uint8_t*) obj->GetIndexedPropertiesExternalArrayData();
    avro->buffer_->appendData(buffer, 0, len);

    std::cout << avro->buffer_->pFirstBlock->data << std::endl;

    /*
    const uint8_t *buffer = static_cast<uint8_t*>(obj->GetIndexedPropertiesExternalArrayData());
    pthread_mutex_lock(&mVar);
    if(BufferIndex==BufferSize){
      pthread_cond_wait(&Buffer_Not_Full,&mVar);
    }
    BUFFER[BufferIndex++]='@';
    printf("Produce : %d \n",BufferIndex);
    pthread_mutex_unlock(&mVar);
    pthread_cond_signal(&Buffer_Not_Empty); 
    */

  }else{
    OnError("Argument must be a Byte Array");
  }

  return scope.Close(Undefined());
}

/**
 * 
 */
Handle<Value> Avro::SetSchema(const Arguments &args) {  
  HandleScope scope;

  Avro * obj = ObjectWrap::Unwrap<Avro>(args.This());

  if (args.Length() != 1) {
    OnError("Wrong number of arguments");
    return scope.Close(Undefined());
  }

  if(args[0]->IsString()){
    // get the param
    v8::String::Utf8Value param1(args[0]->ToString());
    std::ifstream ifs(*param1);
    avro::compileJsonSchema(ifs, obj->schema_);
    std::ostringstream oss(std::ios_base::out);
    obj->schema_.toJson(oss);
    OnSchema(oss.str().c_str());
  }else{
    ThrowException(v8::Exception::TypeError(String::New("Schema Must be a string")));
    return scope.Close(Undefined());
  }
  return scope.Close(Undefined());
}

/**
 * 
 */
Handle<Value> Avro::GetSchema(const Arguments &args) {  
  HandleScope scope;
  Avro * obj = ObjectWrap::Unwrap<Avro>(args.This());
  std::ostringstream oss(std::ios_base::out);
  obj->schema_.toJson(oss);

  return scope.Close(String::New(oss.str().c_str()));
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

/**void OnSchema(const char* schema)
 * [OnSchema description]
 */
void OnSchema(const char* schema){
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


  // prepare arguments for the callback
  Local<Value> argv[1];
  argv[0] = Local<Value>::New(String::New(schema));

  callback->Call(module_handle, 1, argv);
}

/**
 * [OnDatum description]
 * @param datum [description]
 */
void OnDatum(Handle<Value> datum) {
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
Handle<Value> Avro::DecodeAvro(const avro::GenericDatum& datum){
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
