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

Handle<Value> Decode(const Arguments& args){
  HandleScope scope;

  std::cout << "after schema" << std::endl;

  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(*writer.out_);
  avro::DecoderPtr d = avro::binaryDecoder();
  d->init(*in);

  std::auto_ptr<avro::DataStreamReaderBase> dfrb(new avro::DataStreamReaderBase(in));

  avro::DataStreamReader<avro::GenericDatum> dfr(dfrb);
  //avro::DataFileReader<avro::GenericDatum> dfr("test.bin");
  // we can derive the schema from the data file like so. 
  avro::GenericDatum datum(dfr.dataSchema());
  int i = 0;
  Local<Array> datumArray = Array::New();
  while(dfr.read(datum)){
    datumArray->Set(i, DecodeAvro(datum));
    i++;
  }
  return scope.Close(datumArray);
  //return scope.Close(Object::New());

}

Handle<Value> DecodeAppend(const Arguments& args) {
  HandleScope scope;

  //avro::ValidSchema cpxSchema;
  //avro::compileJsonSchema(ifs, cpxSchema);  //std::ifstream in("cpx.json");

  if (args.Length() != 1) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }


  if (!args[0]->IsObject()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }

  Local<Object> obj = args[0]->ToObject();

  //get length of array
  int len = obj->GetIndexedPropertiesExternalArrayDataLength();
  const uint8_t *buffer = static_cast<uint8_t*>(obj->GetIndexedPropertiesExternalArrayData()); 

  writer.writeBytes(buffer, len);
  writer.flush();
  std::auto_ptr<avro::InputStream> in = avro::memoryInputStream(buffer, len);

  return scope.Close(Number::New(len));
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
  c::cpx c1;
  c1.im = 2.13;

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

  avro::DataFileWriter<c::cpx> dfw("test.bin", cpxSchema);
  dfw.write(c1);
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
  exports->Set(String::NewSymbol("decodeAppend"),
    FunctionTemplate::New(DecodeAppend)->GetFunction());
  exports->Set(String::NewSymbol("encode"),
    FunctionTemplate::New(Encode)->GetFunction());
}

NODE_MODULE(avro, init)