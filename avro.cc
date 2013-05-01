#include <node.h>
#include <v8.h>

#include <boost/numeric/ublas/vector.hpp>


#include <fstream>
//include the schema that was sent
#include "cpx.hh"

#include <avro/ValidSchema.hh>
#include <avro/Compiler.hh>

#include <avro/Encoder.hh>
#include <avro/Decoder.hh>

#include "avro/Specific.hh"
#include "avro/Generic.hh"
#include "avro/Node.hh"
#include "avro/DataFile.hh"
#include <avro/Stream.hh>
#include <avro/DataStream.hh>

#include <iostream>
#include <fstream>

using namespace v8;

Handle<Value> DecodeAvro (const avro::GenericDatum& datum);
avro::GenericDatum& EncodeAvro (Handle<Value> datum);
std::auto_ptr<avro::OutputStream> output = avro::memoryOutputStream();
avro::StreamWriter writer = avro::StreamWriter(*output);
avro::ValidSchema schema;

/*
Handle<Value> Decode(const Arguments& args){
  HandleScope scope;
  Local<Array> datumArray = Array::New();

  //std::cout << "after schema" << std::endl;

  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*output);
  avro::DecoderPtr d = avro::binaryDecoder();
  d->init(*in);
  try
  {
    std::auto_ptr<avro::DataStreamReaderBase> dfrb(new avro::DataStreamReaderBase(in));
    std::cout << dfrb.get() << std::endl;
    avro::DataStreamReader<avro::GenericDatum> dfr(dfrb);
    std::cout << &dfr.base_ << std::endl;
    //avro::DataFileReader<avro::GenericDatum> dfr("test.bin");
    // we can derive the schema from the data file like so.
    if(schema == NULL){
      std::cout << "no schema yet." << std::endl;
      schema = &(dfr.dataSchema());

    }

    avro::GenericDatum datum(dfr.dataSchema());
    int i = 0;
    while(dfr.read(datum)){
      datumArray->Set(i, DecodeAvro(datum));
      //size_t outputLen = dfrb->stream_->byteCount();
      //std::cout << outputLen << std::endl;
      i++;
    }
  }
  catch (std::exception &e) {
      std::cerr<< "error" << e.what() << std::endl;
  }

  return scope.Close(datumArray);
}
*/

void DecodeFile(const char* filename, Local<Function> cb){
  const unsigned argc = 1;
  int length = 2;
  Local<Value> argv[2];
  try{
    avro::DataFileReader<avro::GenericDatum> dfr(filename);
    schema = dfr.dataSchema();
    avro::GenericDatum datum(dfr.dataSchema());
    Local<Array> datumArray = Array::New();

    int i = 0;
    while(dfr.read(datum)){
      datumArray->Set(i, DecodeAvro(datum));
      i++;
    }

    argv[0] = Local<Value>::New(String::New("file end")); 
    argv[1] = Local<Value>::New(datumArray);
  }catch(std::exception &e){
    argv[0] = Local<Value>::New(String::New(e.what()));
    length = 1;
  }

  cb->Call(Context::GetCurrent()->Global(), length, argv);
}

void EncodeFile(const char* filename, Local<Function> cb, Local<Object> data){

}

Handle<Value> SetSchema(const Arguments& args){
  HandleScope scope;

  if (args.Length() != 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  if(args[0]->IsString()){
    // get the param
    v8::String::Utf8Value param1(args[0]->ToString());
    std::ifstream ifs(*param1);
    avro::compileJsonSchema(ifs, schema);

  }else{
    ThrowException(Exception::TypeError(String::New("Schema Must be a string")));
    return scope.Close(Undefined());
  }
  return scope.Close(Undefined());
}
  
Handle<Value> GetSchema(const Arguments& args){

  HandleScope scope;
  /*
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Local<Value> argv[1];
  */
  std::ostringstream oss(std::ios_base::out);
  schema.toJson(oss);
  //argv[0] = Local<Value>::New(String::New(oss.str().c_str())); 
  //cb->Call(Context::GetCurrent()->Global(), 1, argv);
  
  return scope.Close(String::New(oss.str().c_str()));
}

Handle<Value> Decode(const Arguments& args) {
  HandleScope scope;

  if (args.Length() > 2) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  Local<Function> cb = Local<Function>::Cast(args[1]);

  if(args[0]->IsString()){
    // get the param
    v8::String::Utf8Value param1(args[0]->ToString());

    DecodeFile(*param1, cb);
  }

  if(args[0]->IsObject()){
    Local<Object> obj = args[0]->ToObject();

    //get length of array
    int len = obj->GetIndexedPropertiesExternalArrayDataLength();
    const uint8_t *buffer = static_cast<uint8_t*>(obj->GetIndexedPropertiesExternalArrayData());    
  }

/*
  writer.writeBytes(buffer, len);
  writer.flush();
*/

  return scope.Close(Undefined());
}


Handle<Value> Encode(const Arguments& args){
  HandleScope scope;
  std::ifstream ifs("cpx.json");

  avro::ValidSchema cpxSchema;
  avro::compileJsonSchema(ifs, cpxSchema);  //std::ifstream in("cpx.json");

  //avro::ValidSchema cpxSchema;
  //avro::compileJsonSchema(in, cpxSchema);

  std::auto_ptr<avro::OutputStream> out = avro::memoryOutputStream();
  avro::EncoderPtr e = avro::binaryEncoder();
  e->init(*out);
  avro::DataFileWriter<c::cpx> dfw("test.bin", cpxSchema);
  c::cpx c1;
  for(int i = 0;i<100; i++){
    c1.im = i;
    //setup array test
    std::vector<std::string> vec ;

    vec.push_back("16");
    vec.push_back("2");
    vec.push_back("77");
    vec.push_back("29");
    c1.array.set_array(vec);

    //setup map test
    std::map<std::string, int> map ;
    map.insert(std::make_pair("first",1));
    map.insert(std::make_pair("second",2));
    map.insert(std::make_pair("third",3));

    c1.map.set_map(map);

    avro::encode(*e, c1);

    dfw.write(c1);
  }
  
  dfw.close();

  return scope.Close(Object::New());
}

avro::GenericDatum& EncodeAvro(Handle<Value> datum){

}

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
      //DecodeAvro(*it);
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

Handle<Value> convertAvro(const avro::GenericDatum& datum){


}
void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("decode"),
    FunctionTemplate::New(Decode)->GetFunction());

  exports->Set(String::NewSymbol("encode"),
    FunctionTemplate::New(Encode)->GetFunction());

  exports->Set(String::NewSymbol("setSchema"),
    FunctionTemplate::New(SetSchema)->GetFunction());

  exports->Set(String::NewSymbol("getSchema"),
    FunctionTemplate::New(GetSchema)->GetFunction());
}

NODE_MODULE(avro, init)