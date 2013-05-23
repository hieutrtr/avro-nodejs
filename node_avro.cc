#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>
#include <avro/DataFile.hh>
#include <avro/Compiler.hh>
#include <avro/Decoder.hh>
#include <avro/Encoder.hh>
#include <avro/Specific.hh>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <queue>
#include "cpx.hh"
#include "BufferedInputStream.hh"

struct datumBaton {
  avro::ValidSchema schema;
  avro::GenericDatum datum;
  std::string errorMessage;
  v8::Persistent<v8::Value> onSuccess;
  v8::Persistent<v8::Value> onError;
};

namespace node {
  using namespace v8;
  using namespace node;
  using namespace avro;

  Persistent<String> on_schema;
  Persistent<String> on_datum;
  Persistent<String> on_error;

  uv_async_t async;
  uv_mutex_t datumLock;

  void InitAvro(Handle<Object> target);

  class Avro : public ObjectWrap {
  public:
    Avro() : ObjectWrap(){};
    ~Avro() {};
    //Buffer *buffer_;
    std::queue<datumBaton> processQueue_;
    ValidSchema schema_;
    bool write_in_progress_;
    std::vector<datumBaton> datums_;
    DecoderPtr decoder_;
    uv_sem_t sem_;
    uv_mutex_t datumLock_;
    uv_mutex_t queueLock_;
    avronode::BufferedInputStream *buffer_;

    static Handle<Value> New(const Arguments& args){
      HandleScope scope;

      Avro *ctx = new Avro();
      ctx->write_in_progress_ = false;
      ctx->decoder_= binaryDecoder();
      std::vector<uint8_t> data;
      ctx->buffer_ = new avronode::BufferedInputStream(data, 0, 0);

      ctx->Wrap(args.This());
      uv_sem_init(&ctx->sem_, 0);
      uv_mutex_init(&ctx->datumLock_);
      uv_mutex_init(&ctx->queueLock_);

      uv_async_init(uv_default_loop(), &async, print_progress);
      //create new work_t
      uv_work_t *work_req = new uv_work_t;
      work_req->data = ctx;

      uv_queue_work(uv_default_loop(),
                    work_req,
                    Avro::Process,
                    Avro::After);  

      return args.This();
    }

    static void print_progress(uv_async_t *handle, int status /*UNUSED*/) {
      Avro* ctx = (Avro *)handle->data;
      // loop through datums then release lock.
      // Thread safe block here 
      // ---------------------------------------------------------------------
      uv_mutex_lock(&ctx->datumLock_);
      for(int i = 0;i < ctx->datums_.size();i++){
        datumBaton baton = ctx->datums_[i];
        //test to see if we have an error;
        if(baton.errorMessage.empty()){
          //success
          OnDatum(ctx, baton.onSuccess, DecodeAvro(baton.datum));
        }else{
          //print error message
          OnError(ctx, baton.onError, baton.errorMessage.c_str());
        }
      }
      ctx->datums_.clear();
      uv_mutex_unlock(&ctx->datumLock_);
      // Thread safe block ends here
      // ---------------------------------------------------------------------
    }

    /**
     * Add a schema onto the schema queue. This is then processes 
     * in the loop
     */
    static Handle<Value> QueueSchema(const Arguments &args){
      HandleScope scope;
      Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());

      datumBaton baton;

      if(!args[0]->IsString()){
        OnError(ctx, on_error, "schema must be a string");
        return scope.Close(Undefined());
      }
      //grab string from input.
      v8::String::Utf8Value schemaString(args[0]->ToString());
      std::istringstream is(*schemaString);
      //create schema
      //
      ValidSchema schema;
      try{
        compileJsonSchema(is, schema);
      }catch(std::exception &e){
        //TODO should send back the bad schema for user reference. 
        OnError(ctx, on_error, e.what());
        return scope.Close(Undefined());
      }

      baton.schema = schema;


      // if args > 2 and args[1] is a function set our onSuccess  
      if(args.Length() > 2 ){
        if(args[1]->IsFunction()){
          baton.onSuccess = Persistent<Function>::New(Handle<Function>::Cast(args[1]));
        }else{
          //error onSuccess must be a function
          OnError(ctx, on_error, "onSuccess must be a callback function.");
          return scope.Close(Undefined());
        }
      }else{
        baton.onSuccess = on_datum;
      }

      // set onerror callback for the proccess struct 
      if(args.Length() > 3 ){
        if(args[2]->IsFunction()){
          baton.onError = Persistent<Function>::New(Handle<Function>::Cast(args[2]));
        }else{
          OnError(ctx, on_error, "onError must be a callback function.");
          return scope.Close(Undefined());
        }
        //error onSuccess must be a function
      }else{
        baton.onError = on_error;
      }
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

      return scope.Close(Undefined());
    }

    /**
     * Push bytes to BufferedInputStream
     */
    static Handle<Value> Push(const Arguments &args){
      HandleScope scope;
      Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());

      if (args.Length() > 1) {
        OnError(ctx, on_error, "Wrong number of arguments");
        return scope.Close(Undefined());
      }

      if(args[0]->IsObject()){
        //convert argument to Object
        Local<Object> in_buf = args[0]->ToObject();

        //get length of buffer
        int len = Buffer::Length(in_buf);
        uint8_t *in = reinterpret_cast<uint8_t*>(Buffer::Data(in_buf));

        //get data of the buffer
        // ------------------------------------------------
        // lock section here adding to BufferedInputStream.
        uv_mutex_lock(&ctx->datumLock_);

        ctx->buffer_->append(in,len);

        uv_mutex_unlock(&ctx->datumLock_);      
        // ------------------------------------------------
        // release lock section here. 

      }else{
        OnError(ctx, on_error, "Argument must be a Byte Array");
      }

      return scope.Close(Undefined());
    }


    static Handle<Value> DecodeFile(const Arguments &args) {  
      HandleScope scope;
      Avro * ctx = ObjectWrap::Unwrap<Avro>(args.This());

      if (args.Length() > 1) {
        ThrowException(v8::Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
      }

      if(args[0]->IsString()){
        // get the param
        v8::String::Utf8Value filename(args[0]->ToString());

        try{
          avro::DataFileReader<avro::GenericDatum> dfr(*filename);
          ctx->schema_ = dfr.dataSchema();
          std::ostringstream oss(std::ios_base::out);
          ctx->schema_.toJson(oss);
          OnSchema(ctx, oss.str().c_str());
          avro::GenericDatum datum(dfr.dataSchema());

          while(dfr.read(datum)){
            OnDatum(ctx, on_datum, DecodeAvro(datum));
          }
        }catch(std::exception &e){
          OnError(ctx, on_error, e.what());
        }
      }else{
        OnError(ctx, on_error, "Wrong Argument. Must be string for filename");
      }

      return scope.Close(Undefined());
    }

    static Handle<Value> ParseDatum(const Arguments &args){
      HandleScope scope;
      if(args.Length() > 2){
        ThrowException(v8::Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());        
      }

      if(args[0]->IsString()&&args[1]->IsObject()){
        //create schema from string
        v8::String::Utf8Value schemaString(args[0]->ToString());
        std::istringstream is(*schemaString);

        ValidSchema schema;
        compileJsonSchema(is, schema);

        DecoderPtr decoder = binaryDecoder();
        Local<Object> in_buf = args[1]->ToObject();
        int len = Buffer::Length(in_buf);
        uint8_t *in = reinterpret_cast<uint8_t*>(Buffer::Data(in_buf));

        std::auto_ptr<avro::InputStream> inputStream = memoryInputStream(in,len);
        decoder->init(*inputStream);

        //create reader and generic datum.
        GenericReader reader(schema, decoder);
        GenericDatum *datum = new GenericDatum(schema);
        reader.read(*datum);
        return scope.Close(DecodeAvro(*datum));
      }

      return scope.Close(Undefined());
    }

    static Handle<Value> EncodeFile(const Arguments &args) { 
      HandleScope scope; 

      if(args[0]->IsString()){
        // get the param
        v8::String::Utf8Value filename(args[0]->ToString());

        std::ifstream ifs("cpx.json");

        avro::ValidSchema cpxSchema;
        avro::compileJsonSchema(ifs, cpxSchema);

        std::auto_ptr<avro::OutputStream> out = memoryOutputStream(1);
        avro::EncoderPtr e = binaryEncoder();
        e->init(*out);

        for(int i = 0;i<1;i++){
          c::cpx c1;
          c1.re.set_double(10*i);
          c1.im = 105;
          encode(*e,c1);
        }

        std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*out);

        StreamReader reader(*in);
        std::ofstream myFile ("data.bin", std::ios::out | std::ios::binary);
        while(reader.hasMore()){
          myFile.put(reader.read());
        }
        //for some reason calling myFile.flush() was 
        //causeing the un defined buffer of reader or in 
        // to be written causing all sorts of problems. 
        myFile.close();
      }
      return scope.Close(Undefined());
    }

    static void Process(uv_work_t* work_req){
      Avro* ctx = (Avro *)work_req->data;
      ctx->decoder_->init(*(ctx->buffer_));

      while(true){
        uv_sem_wait(&ctx->sem_);
        //create reader and generic datum.
        //
        datumBaton baton = ctx->processQueue_.front();
        GenericReader reader(baton.schema, ctx->decoder_);
        //printf("---- set onSuccess %d------\n", ctx->processQueue_.front().onSuccess->IsFunction());
        GenericDatum *datum = new GenericDatum(baton.schema);
        //disgard the top schema
        
        //should probably lock this area for thread safe
        uv_mutex_lock(&ctx->queueLock_);
        ctx->processQueue_.pop();
        uv_mutex_unlock(&ctx->queueLock_);
        //This is a blocking read
        reader.read(*datum);
        baton.datum = *datum;
        // Thread safe area here
        // ---------------------------------------------------------------
        // mutex lock for writing to datum on avro object. 
        uv_mutex_lock(&ctx->datumLock_);
        ctx->datums_.push_back(baton);

        uv_mutex_unlock(&ctx->datumLock_);
        //Thread safe area leave
        // ---------------------------------------------------------------
        //Send data to returning javascript callback
        async.data = (void*) ctx;
        uv_async_send(&async);
      }
    }

    //v8 land!
    static void After(uv_work_t* work_req, int status){
      HandleScope scope;

      Avro* ctx = (Avro *)work_req->data;

      ctx->write_in_progress_ = false;

      //ctx->Unref(); 
    }

    /**
     * converts a GenericDatum into a v8 object that can be passed back to javascript
     */
    static Handle<Value> DecodeAvro(const avro::GenericDatum& datum){
      Handle<Object> obj = Object::New();
      //return this Object
      switch(datum.type())
      {
        case avro::AVRO_RECORD:
          {
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
          }
        case avro::AVRO_STRING:
          return  String::New(
            datum.value<std::string>().c_str(),
            datum.value<std::string>().size()
          );
        case avro::AVRO_BYTES:
          {
            Local<Array> byteArray = Array::New();
            const std::vector<uint8_t> &v = datum.value<std::vector<uint8_t> >();
            for(int i = 0;i<v.size();i++){
              byteArray->Set(i, Uint32::New(v[i]));
            }
            return byteArray;
          }        
        case avro::AVRO_INT:
          return Number::New(datum.value<int>());
        case avro::AVRO_LONG:
          return Number::New(datum.value<long>());
        case avro::AVRO_FLOAT:
          return Number::New(datum.value<float>());
        case avro::AVRO_DOUBLE:
          return Number::New(datum.value<double>());
        case avro::AVRO_BOOL:
          return Boolean::New(datum.value<bool>());
        case avro::AVRO_NULL:
          return v8::Null();
        case avro::AVRO_ENUM:
          return v8::Null();
        case avro::AVRO_ARRAY:
          {
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
          }
        case avro::AVRO_MAP:
          {
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
          }
        case avro::AVRO_UNION:

        case avro::AVRO_FIXED:

        case avro::AVRO_SYMBOLIC:

        case avro::AVRO_UNKNOWN:

        default:
          {
            printf("%d\n", datum.type());
            return obj;
          }
      }

    }

    /**
     * converts a v8 object into a GenericDatum so that it can be encoded by avro.
     * TODO
     */
    static avro::GenericDatum DecodeV8(Local<Value> datum){
      return NULL;
    }

    /**
     * [OnError description]
     * @param error [description]
     */
    static void OnError(Avro *ctx, Persistent<Value> callback, const char* error){

      Local<Value> args[1] = { String::New(error) };

      if(callback->IsFunction()){
        MakeCallback(ctx->handle_, Persistent<Function>::Cast(callback), 1, args);
        //make sure to remove the callback. 
        callback.Dispose();
      }else if(callback->IsString()){
        MakeCallback(ctx->handle_, Persistent<String>::Cast(callback), 1, args);
      }else{
        //something went terribly wrong. 
      }
    }

    /**
     * void OnSchema(const char* schema)
     * @param schema [description]
     * [OnSchema description]
     */
    static void OnSchema(Avro *ctx, const char* schema){
      HandleScope scope;

      assert(ctx->handle_->Get(on_schema)->IsFunction() && "Invalid schema handler");

      Local<Value> args[1] = { String::New(schema) };

      MakeCallback(ctx->handle_, on_schema, 1, args);
    }

    /**
     * [OnDatum description]
     * @param datum [description]
     */
    static void OnDatum(Avro *ctx, Persistent<Value> callback, Handle<Value> datum) {

      Local<Value> args[1] = { Local<Value>::New(datum) };

      if(callback->IsFunction()){
        MakeCallback(ctx->handle_, Persistent<Function>::Cast(callback), 1, args);
        callback.Dispose();
      }else if(callback->IsString()){
        MakeCallback(ctx->handle_, Persistent<String>::Cast(callback), 1, args);
      }else{
        //something went terribly wrong. 
      }

    }

  };


/**
 * [Initialize description]
 * @param target [description]
 */
void InitAvro(Handle<Object> target){
  HandleScope scope;

  Local<FunctionTemplate> a_temp = FunctionTemplate::New(Avro::New);

  a_temp->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(a_temp, "decode", Avro::DecodeFile);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "encode", Avro::EncodeFile);

  NODE_SET_PROTOTYPE_METHOD(a_temp, "push", Avro::Push);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "queueSchema", Avro::QueueSchema);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "parseDatum", Avro::ParseDatum);

/*
  NODE_SET_PROTOTYPE_METHOD(a_temp, "encodeBytes", Avro::EncodeBytes);
*/
  a_temp->SetClassName(String::NewSymbol("Avro"));
  target->Set(String::NewSymbol("Avro"), a_temp->GetFunction());

  on_schema = NODE_PSYMBOL("onschema");
  on_datum = NODE_PSYMBOL("ondatum");
  on_error = NODE_PSYMBOL("onerror");

}

NODE_MODULE(avro, node::InitAvro);

}