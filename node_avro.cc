#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <avro/ValidSchema.hh>
#include <avro/Generic.hh>
#include <avro/DataFile.hh>
#include <avro/Compiler.hh>
#include <stdlib.h>

//#include "buffer.h"
typedef std::vector<char> buffer_type;


namespace node {
  using namespace v8;
  using namespace node;
  using namespace avro;

  Persistent<String> on_schema;
  Persistent<String> on_datum;
  Persistent<String> on_error;

  void InitAvro(Handle<Object> target);

  class Avro : public ObjectWrap {
  public:
    Avro() : ObjectWrap(){};
    ~Avro() {};
    //Buffer *buffer_;
    ValidSchema schema_;
    buffer_type buffer_;


    static Handle<Value> New(const Arguments& args){
      HandleScope scope;

      Avro *ctx = new Avro();
      //ctx->buffer_ = new Buffer();
      ctx->Wrap(args.This());

      return args.This();
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
            OnDatum(ctx, DecodeAvro(datum));
          }
        }catch(std::exception &e){
          OnError(ctx, e.what());
        }
      }else{
        OnError(ctx, "Wrong Argument. Must be string for filename");
      }

      return scope.Close(Undefined());
    }

    /**
     * 
     */
    static Handle<Value> DecodeBytes(const Arguments &args) {  
      HandleScope scope;
      Avro * avro = ObjectWrap::Unwrap<Avro>(args.This());

      if (args.Length() > 1) {
        OnError(avro, "Wrong number of arguments");
        return scope.Close(Undefined());
      }

      if(args[0]->IsObject()){

        Local<Object> in_buf = args[0]->ToObject();

        int len = Buffer::Length(in_buf);

        unsigned char *in = reinterpret_cast<unsigned char *>(Buffer::Data(in_buf));

        avro->buffer_.insert(avro->buffer_.end(), in, in + len);

      }else{
        OnError(avro, "Argument must be a Byte Array");
      }

      return avro->handle_;
    }

    /**
     * converts a GenericDatum into a v8 object that can be passed back to javascript
     */
    static Handle<Value> DecodeAvro(const avro::GenericDatum& datum){
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

    /**
     * [OnError description]
     * @param error [description]
     */
    static void OnError(Avro *ctx, const char* error){
      HandleScope scope;
      // locate callback from the module context if defined by script
      // example = require('example')
      // example.onschema = function( ... ) { ..
      
      assert(ctx->handle_->Get(on_error)->IsFunction() && "Invalid error handler");

      Local<Value> args[1] = { String::New(error) };

      MakeCallback(ctx->handle_, on_error, 1, args);
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
    static void OnDatum(Avro *ctx, Handle<Value> datum) {

      HandleScope scope;

      assert(ctx->handle_->Get(on_datum)->IsFunction() && "Invalid datum handler");

      Local<Value> args[1] = { Local<Value>::New(datum) };

      MakeCallback(ctx->handle_, on_datum, 1, args);
    }

  private:
    uv_work_t work_req_;

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
  NODE_SET_PROTOTYPE_METHOD(a_temp, "decodeBytes", Avro::DecodeBytes);
/*
  NODE_SET_PROTOTYPE_METHOD(a_temp, "encode", Avro::EncodeFile);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "setSchema", Avro::SetSchema);
  NODE_SET_PROTOTYPE_METHOD(a_temp, "getSchema", Avro::GetSchema);
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