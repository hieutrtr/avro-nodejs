#include "helpers.h"

namespace helper {

vector<uint8_t> getBinaryData(Local<Value> val){
  int length;
  uint8_t *data;
  vector<uint8_t> bytes;

  if(Buffer::HasInstance(val)){
    length = Buffer::Length(val); 
    data = reinterpret_cast<uint8_t*>(Buffer::Data(val));
    return vector<uint8_t>(data, data+length);
  }else if(val->IsArray()){
    Local<Array> array = Local<Array>::Cast(val);
    length = array->Length();
    bytes = vector<uint8_t>(length);
    for(uint32_t i = 0; i<length;i++){
      bytes[i] = array->Get(i)->Int32Value();
    }
    return bytes; 
  }else {
    throw "value is not a Buffer or an instance of an Array";
    return bytes;
  }
}

}
