#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "node_avro.h"
#include "AvroFileReader.h"
#include "AvroFileWriter.h"


using namespace v8;

void InitAll(Handle<Object> exports){

  Avro::Initialize(exports);
  //FileReader Object
  AvroFileReader::Init(exports);
  //FileWriter Object
  AvroFileWriter::Init(exports);
}


NODE_MODULE(addon, InitAll)
