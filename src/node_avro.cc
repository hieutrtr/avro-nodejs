#include "node_avro.h"
#include "translate.h"
#include "helpers.h"

namespace node {

Persistent<String> on_schema;
Persistent<String> on_datum;
Persistent<String> on_error;
Persistent<String> on_close;
static ValidSchema getValidSchema(string type, string schemaString, helper::SymbolMap dictionary);

static void ResultEvent(uv_async_t *handle, int status);
static void Process(uv_work_t* work_req);
static void After(uv_work_t* work_req, int status);
static void OnError(Avro *ctx, Persistent<Value> callback, const char* error);
static void OnSchema(Avro *ctx, const char* schema);
static void OnClose(Avro *ctx, Persistent<Value> callback);
static void OnDatum(Avro *ctx, Persistent<Value> callback, Handle<Value> datum);
static void handleCallbacks(Avro *ctx, datumBaton *baton, const Arguments &args, int startPos);


/**
 * Constructs a new avro object. Creates new worker thread and async callbacks
 * for async support.
 */
Handle<Value> Avro::New(const Arguments& args){
    HandleScope scope;

    Avro *ctx = new Avro();
    ctx->decoder_ = binaryDecoder();
    std::vector<uint8_t> data;
    ctx->buffer_ = new avronode::BufferedInputStream(data, 0, 0);
    ctx->read_  = true;
    ctx->avro_loop_ = uv_default_loop();
    ctx->Wrap(args.This());
    uv_sem_init(&ctx->sem_, 0);
    uv_mutex_init(&ctx->datumLock_);
    uv_mutex_init(&ctx->queueLock_);

    uv_async_init(ctx->avro_loop_, &ctx->async_, ResultEvent);
    //create new work_t
    uv_work_t *work_req = new uv_work_t;

    work_req->data = ctx;

    uv_queue_work(ctx->avro_loop_,
                  work_req,
                  Process,
                  After);  

    return args.This();
}

/**
 * [ResultEvent description]
 * @param handle [The uv_async handle ]
 * @param status [description]
 */
static void ResultEvent(uv_async_t *handle, int status) {
  Avro* ctx = (Avro *)handle->data;
  // loop through datums then release lock.
  // Thread safe block here 
  // ---------------------------------------------------------------------
  uv_mutex_lock(&ctx->datumLock_);
  for(size_t i = 0;i < ctx->datums_.size();i++){
    datumBaton baton = ctx->datums_[i];
    //test to see if we have an error;
    if(!strlen(baton.errorMessage)){
      //success
      OnDatum(ctx, baton.onSuccess, DecodeAvro(*baton.datum));
    }else{
      //print error message
      printf("baton error '%s' length %d\n",baton.errorMessage, strlen(baton.errorMessage));
      OnError(ctx, baton.onError, baton.errorMessage);
    }

    delete baton.datum;
  }
  ctx->datums_.clear();
  uv_mutex_unlock(&ctx->datumLock_);

  // Thread safe block ends here
  // ---------------------------------------------------------------------
}

/**
 * [Closes the avro object. ]
 */
Handle<Value> Avro::Close(const Arguments &args){
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());
  uv_mutex_lock(&ctx->queueLock_);
  while(!ctx->processQueue_.empty()){
    ctx->processQueue_.pop();
  }
  ctx->read_ = false;
  ctx->buffer_->close();

  //set the smart pointer to 0 so that it can be cleaned up.
  //ctx->decoder_.reset();
  uv_sem_post(&ctx->sem_);
  uv_mutex_unlock(&ctx->queueLock_);
  
  uv_run(ctx->avro_loop_, UV_RUN_DEFAULT); 

  //fires off the on close event. 
  OnClose(ctx, on_close);
  return scope.Close(Undefined());
}

/**
 * Add a schema onto the schema queue. This is then processes 
 * in the loop
 */
Handle<Value> Avro::QueueSchema(const Arguments &args){
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());
  datumBaton baton;
  baton.errorMessage = "";

  if(!args[0]->IsString()){
    OnError(ctx, on_error, "schema must be a string");
    return scope.Close(Undefined());
  }
  //grab string from input.
  String::Utf8Value schemaString(args[0]->ToString());

  try{
    baton.schema = getValidSchema(*schemaString, *schemaString, ctx->dictionary_);

    // if args > 2 and args[1] is a function set our onSuccess  
    handleCallbacks(ctx, &baton, args, 1);

    // ------------------------------------------------
    // lock section here adding to queue
    uv_mutex_lock(&ctx->queueLock_);

    //push new schema to queue
    ctx->processQueue_.push(baton);
    uv_sem_post(&ctx->sem_);
    uv_mutex_unlock(&ctx->queueLock_);
    // ------------------------------------------------
    // release lock 
    // 
  }catch(std::exception &e){
    //TODO should send back the bad schema for user reference. 
    printf("an exception for the schema\n");
    OnError(ctx, on_error, e.what());
    return scope.Close(Undefined());
  }
  return scope.Close(Undefined());
}


/**
 * Gets the number of schemas that are queued.  
 */
 Handle<Value> Avro::PendingSchemas(const Arguments &args){
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());

  return scope.Close(Number::New(ctx->processQueue_.size()));
 }

/**
 * Sending bytes to the decode async thread loop.
 * @param bytes [A Buffer containing a series of bytes.]
 * @return [a javascript undefined]
 */
Handle<Value> Avro::Push(const Arguments &args){
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());

  if (args.Length() > 1) {
    OnError(ctx, on_error, "Wrong number of arguments");
    return scope.Close(Undefined());
  }
  try{
      // ------------------------------------------------
      // lock section here adding to BufferedInputStream.
      //uv_mutex_lock(&ctx->datumLock_);
      std::vector<uint8_t> bytes = helper::getBinaryData(args[0]); 

      //get data of the buffer
      uint8_t *in = bytes.data();


      ctx->buffer_->append(in,bytes.size());

      //uv_mutex_unlock(&ctx->datumLock_); 
      // release lock section here.      
      // ------------------------------------------------

  }catch(std::exception &e){
    //TODO should send back the bad schema for user reference. 
    OnError(ctx, on_error, e.what());
    return scope.Close(Undefined());
  }


  return scope.Close(Undefined());
}


/**
 * Gets the size of the underlining Buffer.
 */
Handle<Value> Avro::BufferLength(const Arguments &args){
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());

  return scope.Close(Number::New(ctx->buffer_->size()));
   
}


/**
 * Validates and adds the schema definition to the dictionary.
 */
Handle<Value> Avro::AddSchema(const Arguments &args){
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());

  if(!args[0]->IsString()){
    OnError(ctx, on_error, "schema must be a string");
    return scope.Close(Undefined());
  }

  v8::String::Utf8Value schemaString(args[0]->ToString());
  ValidSchema schema;
  try{
    //grab string from input.
    std::istringstream is(*schemaString);
    compileJsonSchema(is, schema);
    helper::validate(schema.root(),ctx->dictionary_);
  }catch(std::exception &e){
    OnError(ctx, on_error, e.what());
  }

  return scope.Close(Undefined());
}


/**
 *
 *
 */
Handle<Value> Avro::ClearDictionary(const Arguments &args){
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());

  //clear the dictionary
  ctx->dictionary_.clear();

  return scope.Close(Undefined());
}

/**
 * Takes a avro data file that must contain the schema definition
 * as part of the file. For each datum that is parsed out of the file
 * OnDatum will be called. 
 * @param avrofile [The file containing the Avro data and schema. ]
 * TODO to be depricated functionality being moved over to AvroReadFile. 
 */
Handle<Value> Avro::DecodeFile(const Arguments &args) {  
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());

  if (args.Length() < 1) {
    ThrowException(v8::Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }
  datumBaton baton;
    // if args > 2 and args[1] is a function set our onSuccess  
  handleCallbacks(ctx, &baton, args, 1);

  if(args[0]->IsString()){
    // get the param
    v8::String::Utf8Value filename(args[0]->ToString());

    try{
      avro::DataFileReader<avro::GenericDatum> dfr(*filename);
      ValidSchema schema = dfr.dataSchema();
      
      std::ostringstream oss(std::ios_base::out);
      avro::GenericDatum datum(dfr.dataSchema());

      while(dfr.read(datum)){
        OnDatum(ctx, baton.onSuccess, DecodeAvro(datum));
      }
      
      if(baton.onSuccess->IsFunction()){
        baton.onSuccess.Dispose();
      }
    }catch(std::exception &e){
      OnError(ctx, baton.onError, e.what());
    }
  }else{
    OnError(ctx, on_error, "Wrong Argument. Must be string for filename");
  }

  return scope.Close(Undefined());
}
/**
 * The sync version of decode datum. The buffer of bytes provided must 
 * contain the entire datum to be decoded. Otherwise an error will be thrown.
 * @param bytes [A node Buffer object containing all of the bytes of the datum.]
 * @param type [A string representing the type to be decoded]
 * @param schema [The schema to decode bytes with.]
 */
Handle<Value> Avro::DecodeDatum(const Arguments &args){
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());
  Handle<Value> datumObject;
  ValidSchema schema;

  if(args.Length() < 2){
    OnError(ctx, on_error, "Wrong number of arguments");
    return scope.Close(Undefined());        
  }
  Local<String> type;

  //create schema from string
  String::Utf8Value typeStr(args[1]->ToString());

  try{

    schema = getValidSchema(*typeStr, *typeStr, ctx->dictionary_);
    DecoderPtr decoder = binaryDecoder();
    vector<uint8_t> bytes = helper::getBinaryData(args[0]);
    uint8_t *in = bytes.data();

    auto_ptr<avro::InputStream> inputStream = memoryInputStream(in,bytes.size());
    decoder->init(*inputStream);

    //create reader and generic datum.
    GenericReader reader(schema, decoder);
    GenericDatum *datum = new GenericDatum(schema);
    reader.read(*datum);
    datumObject = DecodeAvro(*datum);
    decoder.reset();
    delete datum;
  }catch(std::exception &e){
    OnError(ctx, on_error, e.what());
  }

  return scope.Close(datumObject);

}

/**
 * A synchronous function to encode a datum into avro binary
 * @param datum [The datum to be encoded]
 * @param (optional) schema [The schema to encode the datum with]
 */
Handle<Value> Avro::EncodeDatum(const Arguments &args){
  HandleScope scope;
  Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());
  Local<Array> byteArray = Array::New();
  ValidSchema schema;
  EncoderPtr e;
  auto_ptr<avro::OutputStream> out = avro::memoryOutputStream();

  if(args.Length()< 1){
    OnError(ctx, on_error, "EncodeDatum: no value to encode");
    return scope.Close(Array::New());
  }

  if(args.Length() > 1 && !args[1]->IsString()){
    OnError(ctx, on_error, "schema must be a string");
    return scope.Close(Array::New());
  }

  v8::String::Utf8Value schemaString(args[1]->ToString());
  Local<Value> value = args[0];

  try{
    // The object (currently defined as namespace)
    Local<String> objectType;

    //if is object then see if there is a namespace else set type to schema
    if(value->IsObject()){
      Local<Object> object = value->ToObject();
      //if namespace use that else use args[1] for type
      if(object->Has(String::New("namespace"))){
        objectType = object->Get(String::New("namespace"))->ToString();
      }else{
        objectType = args[1]->ToString();
      }
    }else{
      objectType = args[1]->ToString();
    }
    
    String::Utf8Value typeString(objectType);

    //Get the schema either from the dictionary or the provided
    //Schema (adding the schema if it has a type)
    schema = getValidSchema(*typeString, *schemaString, ctx->dictionary_); 

    GenericDatum datum(schema);

    datum = DecodeV8(datum, value);

    e = validatingEncoder(schema, binaryEncoder());
    e->init(*out);
    encode(*e, datum);
    
    //need to flush the bytes to the stream (aka out);
    e->flush();

    auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);

    avro::StreamReader reader(*in);
    int i = 0;
    // could initialize a buffer and then do a while read of x chunk say 4k
    while(reader.hasMore()){
      byteArray->Set(i, Uint32::New(reader.read()));
      i++;
    }
  }catch(MissingDatumFieldException &e){
    string error = e.what();
    OnError(ctx, on_error, error.c_str());
    return scope.Close(Array::New());
    
  }catch(std::exception &e){
    string error = e.what();
    string errorMessage = error + *schemaString + "\n";
    OnError(ctx, on_error, errorMessage.c_str());
    return scope.Close(Array::New());
  }

  //construct a byte array to send back to the javascript

  return scope.Close(byteArray);
}

/**
 * A helper function to get a ValidSchema. 
 *
 * @param type. Is a possible mapping into the SymbolMap. If is found the schema associated with it is returned.
 * @param schemaString. The schema as a json string.
 * @param dictionary. The symbol map for known types to schema definitions. 
 */
static ValidSchema getValidSchema(string type, string schemaString, helper::SymbolMap dictionary){

  try{

    Name *fullName = new Name(type);
    helper::SymbolMap::iterator it;
    if((it = dictionary.find(*fullName)) != dictionary.end()){
      return  *(new ValidSchema(it->second));
    }
  }catch(exception &e){
    //not a valid name wish there was final 
  }

  ValidSchema schema;
  std::istringstream is(schemaString);
  compileJsonSchema(is, schema);
  helper::validate(schema.root(),dictionary);

  return schema;
}

/**
 * [Internal thread to process avro decode bytes in a none
 * blocking IO manor.]
 * @param work_req [description]
 */
static void Process(uv_work_t* work_req){
  Avro* ctx = (Avro *)work_req->data;
  ctx->decoder_->init(*(ctx->buffer_));

  while(true){
    uv_sem_wait(&ctx->sem_);
    //if we got a signal to end and we have an empty queue break;
    if(!ctx->read_){
      break;
    }
    uv_mutex_lock(&ctx->queueLock_);
      datumBaton baton = ctx->processQueue_.front();
      ctx->processQueue_.pop();
    uv_mutex_unlock(&ctx->queueLock_);

    try{
      //create reader and generic datum.
      GenericReader reader(baton.schema, ctx->decoder_);
      baton.datum = new GenericDatum(baton.schema);

      //This is a blocking read
      reader.read(*baton.datum);

    }catch(std::exception &e){
      printf("we got an error in the process\n");
      baton.errorMessage = e.what();
    }
        // Thread safe area here
    // ---------------------------------------------------------------
    // mutex lock for writing to datum on avro object. 
    // 
    uv_mutex_lock(&ctx->datumLock_);

    ctx->datums_.push_back(baton);

    uv_mutex_unlock(&ctx->datumLock_);
    //Thread safe area leave
    // ---------------------------------------------------------------
    //Send data to returning javascript callback
    ctx->async_.data = (void*) ctx;
    uv_async_send(&ctx->async_);

  }
}

/**
 * This is the function that is called directly after the Process function
 * is finished. Currently it runs the entire lifetime of Avro instance. 
 * @param work_req [description]
 * @param status   [description]
 */
static void After(uv_work_t* work_req, int status){
  HandleScope scope;

  Avro *ctx = static_cast<Avro*>(work_req->data);
  uv_close((uv_handle_t*)&ctx->async_, NULL);
  free(work_req);

}


/**
 * Parses out the two callbacks for the arguments. It is assumed that they are 
 * defined in pairs. OnSuccess, OnError.  
 * @param ctx              [Our Avro Object]
 * @param baton            [the baton we're setting the callbacks on]
 * @param args             [The arguments]
 * @param startingPosition [The starting position to look for the callbacks in the Arugments]
 */
void handleCallbacks(Avro *ctx, datumBaton *baton, const Arguments &args, int startPos){
  if(args.Length() > startPos+1 ){
    if(args[startPos]->IsFunction()){
      baton->onSuccess = Persistent<Function>::New(Handle<Function>::Cast(args[startPos]));
    }else{
      //error onSuccess must be a function
      OnError(ctx, on_error, "onSuccess must be a callback function.");
    }
  }else{
    baton->onSuccess = on_datum;
  }
  startPos++;
  // set onerror callback for the proccess struct 
  if(args.Length() > startPos+1 ){
    if(args[startPos]->IsFunction()){
      baton->onError = Persistent<Function>::New(Handle<Function>::Cast(args[startPos]));
    }else{
      OnError(ctx, on_error, "onError must be a callback function.");
    }
    //error onSuccess must be a function
  }else{
    baton->onError = on_error;
  }
}

/**
 * [returns the error message to the callback specified.]
 * @param ctx [The avro object that we get the handle from]
 * @param callback [The call back that we return the data error message to]
 * @param error [error message]
 * 
 */
static void OnError(Avro *ctx, Persistent<Value> callback, const char* error){

  Local<Value> args[1] = { String::New(error) };
  if(callback->IsFunction()){
    MakeCallback(ctx->handle_, Persistent<Function>::Cast(callback), 1, args);
    //make sure to remove the callback. Actually don't do that here. 
    //callback.Dispose();
  }else if(callback->IsString()){
    Local<Value> callback_v = ctx->handle_->Get(callback);
    //if there is no error callback defined we'll just throw an exception;
    if(callback_v->IsFunction()){
      MakeCallback(ctx->handle_, Local<Function>::Cast(callback_v), 1, args);
    }else{
      ThrowException(String::New(error));
    }

  }else{
    printf("error wtf\n"); 
  }
}

/**
 * [Returns the schema definiton in json format]
 * @param ctx [The context for our callback]
 * @param schema [The scheam that we set.]
 */
static void OnSchema(Avro *ctx, const char* schema){
  HandleScope scope;

  assert(ctx->handle_->Get(on_schema)->IsFunction() && "Invalid schema handler");

  Local<Value> args[1] = { String::New(schema) };

  MakeCallback(ctx->handle_, on_schema, 1, args);
}

/**
 * [OnDatum return the datum to the specified callback method]
 * @param ctx [The avro object that we get the handle from]
 * @param callback [The call back that we return the data error message to]
 * @param datum [The datum that we're passing to the callback function]
 */
static void OnDatum(Avro *ctx, Persistent<Value> callback, Handle<Value> datum) {

  Local<Value> args[1] = { Local<Value>::New(datum) };

  if(callback->IsFunction()){
    MakeCallback(ctx->handle_, Persistent<Function>::Cast(callback), 1, args);
    //callback.Dispose();
  }else if(callback->IsString()){
    MakeCallback(ctx->handle_, Persistent<String>::Cast(callback), 1, args);
  }else{
    printf("datum wtf\n"); 
  }
}

/**
 * [OnClose the callback to be called on the avro close]
 * @param ctx      [description]
 * @param callback [description]
 */
 static void OnClose(Avro *ctx, Persistent<Value> callback){

    Local<Value> args[0] = {};

    if(callback->IsFunction()){
      MakeCallback(ctx->handle_, Persistent<Function>::Cast(callback), 0, args);
    }else if(callback->IsString()){
      Local<Value> callback_v = ctx->handle_->Get(callback);
      ////if there is no error callback defined we'll just throw an exception;
      if(callback_v->IsFunction()){
        MakeCallback(ctx->handle_, Local<Function>::Cast(callback_v), 0, args);
      }
    }else{
      printf("close wtf\n");
    }
 } 

/**
 * [Creates all of the possible calls from the javascript]
 * @param target [description]
 */
void Avro::Initialize(Handle<Object> target){
  HandleScope scope;

  Local<FunctionTemplate> a_temp = FunctionTemplate::New(Avro::New);

  a_temp->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(a_temp, "decodeFile", Avro::DecodeFile);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "addSchema", Avro::AddSchema);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "clearDictionary", Avro::ClearDictionary);


  NODE_SET_PROTOTYPE_METHOD(a_temp, "push", Avro::Push);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "bufferLength", Avro::BufferLength);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "queueSchema", Avro::QueueSchema);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "pendingSchemas", Avro::PendingSchemas);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "decodeDatum", Avro::DecodeDatum);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "encodeDatum", Avro::EncodeDatum);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "close", Avro::Close);
  a_temp->SetClassName(String::NewSymbol("Avro"));
  target->Set(String::NewSymbol("Avro"), a_temp->GetFunction());

  on_schema = NODE_PSYMBOL("onschema");
  on_datum = NODE_PSYMBOL("ondatum");
  on_error = NODE_PSYMBOL("onerror");
  on_close = NODE_PSYMBOL("onclose");

}

}
