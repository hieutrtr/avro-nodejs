#include "avro-node.h"

using namespace v8;
using namespace node;


void init (Handle<Object> target) 
{
  HandleScope scope;
  Initialize(target);
}

NODE_MODULE(avro, init)