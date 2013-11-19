#include "translate.h"


v8::Handle<v8::Value> DecodeAvro(const avro::GenericDatum& datum){
  return DecodeAvro(datum, v8::Array::New());
}

/**
 * converts a GenericDatum into a v8 object that can be passed back to javascript
 * @param datum [Turning the avro GenericDatum into a v8 object that we can pass back to javascript]
 * @return returns the v8 object that can be used by javascript.
 */
v8::Handle<v8::Value> DecodeAvro(const avro::GenericDatum& datum,  v8::Local<v8::Array> reference){
  v8::Handle<v8::Object> obj = v8::Object::New();
  //return this Object
  switch(datum.type())
  {
    case avro::AVRO_RECORD:
      {
        const avro::GenericRecord& record = datum.value<avro::GenericRecord>();
        const avro::NodePtr& node = record.schema();
        v8::Handle<v8::Object> obj = v8::Object::New();
        for(uint i = 0; i<record.fieldCount(); i++){
          v8::Local<v8::String> datumName = v8::String::New(node->nameAt(i).c_str(), node->nameAt(i).size());

          const avro::GenericDatum& subDatum = record.fieldAt(i);
          if(!strcmp("com.gensler.scalavro.Reference", node->name().fullname().c_str())){
            return reference->Get(DecodeAvro(subDatum, reference)->Int32Value());
          }else {
            v8::Handle<v8::Value> tmp = DecodeAvro(subDatum, reference);
            obj->Set(datumName, tmp);
          }
        }
        reference->Set(reference->Length(), obj);
        return obj;
      }
    case avro::AVRO_STRING:
      {
        return  v8::String::New(
          datum.value<std::string>().c_str(),
          datum.value<std::string>().size()
        );
      } 
    case avro::AVRO_BYTES:
      {
        v8::Local<v8::Array> byteArray = v8::Array::New();
        const std::vector<uint8_t> &v = datum.value<std::vector<uint8_t> >();
        for(size_t i = 0;i<v.size();i++){
          byteArray->Set(i, v8::Int32::New(v[i]));
        }
        return byteArray;
      }        
    case avro::AVRO_INT:
      return v8::Number::New(datum.value<int32_t>());
    case avro::AVRO_LONG:
      return v8::Number::New(datum.value<int64_t>());
    case avro::AVRO_FLOAT:
      return v8::Number::New(datum.value<float>());
    case avro::AVRO_DOUBLE:
      return v8::Number::New(datum.value<double>());
    case avro::AVRO_BOOL:
    {
      v8::Handle<v8::Boolean> boolVal = v8::Boolean::New(datum.value<bool>());
      return boolVal;
    }
    case avro::AVRO_NULL:
      return v8::Null();
    case avro::AVRO_ARRAY:
      {
        const avro::GenericArray &genArray = datum.value<avro::GenericArray>();

        const std::vector<avro::GenericDatum> &v = genArray.value();
        v8::Local<v8::Array> datumArray = v8::Array::New();
        int i = 0;
        for(std::vector<avro::GenericDatum>::const_iterator it = v.begin(); it != v.end(); ++it) {
          const avro::GenericDatum &itDatum = * it;
          datumArray->Set(i, DecodeAvro(itDatum, reference));
          i++;
        }
        return datumArray;
      }
    case avro::AVRO_MAP:
      {
        const avro::GenericMap &genMap = datum.value<avro::GenericMap>();

        const std::vector < std::pair < std::string, avro::GenericDatum > > &v = genMap.value();
        v8::Local<v8::Array> datumArray = v8::Array::New();
        int i = 0;
        for(std::vector< std::pair < std::string, avro::GenericDatum> >::const_iterator it = v.begin(); it != v.end(); ++it) {
          const std::pair < std::string, avro::GenericDatum> &itDatum = * it;
          datumArray->Set(v8::String::New(
            itDatum.first.c_str(),
            itDatum.first.size()
            ),DecodeAvro(itDatum.second, reference));

          i++;
        }
        return datumArray;
      }

    case avro::AVRO_FIXED:
      {
        
        v8::Local<v8::Array> fixedBytes = v8::Array::New();
        const avro::GenericFixed &genFixed = datum.value<avro::GenericFixed>();
        const std::vector<uint8_t> &v = genFixed.value();
        for(size_t i = 0;i<v.size();i++){
          fixedBytes->Set(i, v8::Uint32::New(v[i]));
        }
        return fixedBytes;
      }
    case avro::AVRO_ENUM:
      {
        const avro::GenericEnum &genEnum = datum.value<avro::GenericEnum>();
        int32_t enumVal = genEnum.value();
        return v8::Number::New(enumVal);
      }

    //Unimplemented avro types
    case avro::AVRO_SYMBOLIC:

    case avro::AVRO_UNKNOWN:

    case avro::AVRO_UNION:

    default:
      {
        printf("data type %d\n", datum.type());
        return obj;
      }
  }

}

avro::GenericDatum DecodeV8(avro::GenericDatum datum, v8::Local<v8::Value> object){
  std::vector<int> reference;
  return DecodeV8(datum, object, &reference); 
}
/**

 * [DecodeV8 returns the generated GenericDatum or throws a exception]
 * @param  ctx    [The avro context object for error handling.]
 * @param  datum  [The datum that is being built up for return.]
 * @param  object [The Javascript object that is being converted to a GenericDatum]
 * @param reference [The reference for known JSObjects]
 * @return        [description]
 */
avro::GenericDatum DecodeV8(avro::GenericDatum datum, v8::Local<v8::Value> object, std::vector<int> *reference){

  //if the datum is a union we want to try
  // and pick the right branch. 
  if(datum.isUnion()){
    if(!object->IsNull()){
      v8::Local<v8::Object> obj = object->ToObject();
      //Set the object to the value of the union. 
      v8::Local<v8::Value> typeObject;
      if(obj->Has(v8::String::New("namespace"))){
        typeObject = obj->Get(v8::String::New("namespace"));
        v8::String::Utf8Value constructorString(obj->GetConstructorName());
      }else{
        typeObject = obj->GetPropertyNames()->Get(0);
        object = obj->Get(typeObject->ToString());
      }
      v8::String::Utf8Value typeString(typeObject->ToString());
      unionBranch(&datum, *typeString);
    }else{
      unionBranch(&datum, "null");
    }
    
  }

  switch(datum.type())
  {
    case avro::AVRO_RECORD:
      {
        v8::Local<v8::Object> obj = object->ToObject();
        int identityHash = obj->GetIdentityHash();
        //we can either have an object or a reference type (0,1)
        // this determines whether or not we can repeat. 
        std::vector<int>::iterator iter;
        if((iter = std::find(reference->begin(), reference->end(),identityHash)) != reference->end()){
          size_t index = std::distance(reference->begin(), iter); 
          datum.selectBranch(1);
          obj = v8::Object::New();
          obj->Set(v8::String::New("id"), v8::NumberObject::New(index));
        }
        const avro::GenericRecord& record = datum.value<avro::GenericRecord>();
        const avro::NodePtr& node = record.schema();
        for(uint i = 0; i<record.fieldCount(); i++){
          //Add values
          v8::Local<v8::String> datumName = v8::String::New(node->nameAt(i).c_str(), node->nameAt(i).size());
          datum.value<avro::GenericRecord>().fieldAt(i) = DecodeV8(record.fieldAt(i), obj->Get(datumName), reference);
        }
        reference->push_back(identityHash);
      }
      break;
    case avro::AVRO_STRING:
      {
        v8::String::Utf8Value avroString(object->ToString());
        datum.value<std::string>() = *avroString;
      }
      break;
    case avro::AVRO_BYTES:
      {
        v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(object);
        std::vector<uint8_t> bytes(array->Length());
        for(uint32_t i = 0;i<array->Length();i++){
          bytes[i] = array->Get(i)->Int32Value();
        }
        datum.value<std::vector<uint8_t> >() = bytes;
      }
      break;       
    case avro::AVRO_INT:
      datum.value<int32_t>() = object->Int32Value() ;
      break;       
    case avro::AVRO_LONG:
      datum.value<int64_t>() = object->IntegerValue() ;
      break;       
    case avro::AVRO_FLOAT:
      datum.value<float>() = static_cast<float>(object->NumberValue());
      break;       
    case avro::AVRO_DOUBLE:
      datum.value<double>() = object->NumberValue();
      break;       
    case avro::AVRO_BOOL:
      datum.value<bool>() = object->ToBoolean()->Value();
      break;
    case avro::AVRO_NULL:
      break;       
    case avro::AVRO_ARRAY:
      {
        v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(object);
        avro::GenericArray &genArray = datum.value<avro::GenericArray>();
        std::vector<avro::GenericDatum> &v = genArray.value();
        const avro::NodePtr& node = genArray.schema();
        //gets the second value of the map. The first is always string as defined by avro
        avro::GenericDatum item(node->leafAt(0));

        for( uint32_t i = 0;i<array->Length();i++){
          v.push_back(DecodeV8(item, array->Get(i), reference));
        }
        genArray.value() = v;
        datum.value<avro::GenericArray>() = genArray;
      }
      break;
    case avro::AVRO_MAP:
      {
        v8::Local<v8::Object> map = object->ToObject();
        avro::GenericMap &genMap = datum.value<avro::GenericMap>();
        const avro::NodePtr& node = genMap.schema();
        //gets the second value of the map. The first is always string as defined by avro
        avro::GenericDatum mapped(node->leafAt(1));
        v8::Local<v8::Array> propertyNames = map->GetPropertyNames();
        std::vector < std::pair < std::string, avro::GenericDatum > > &v = genMap.value();;
        for(size_t i = 0;i<propertyNames->Length();i++){
          v8::Local<v8::String> key = propertyNames->Get(i)->ToString();
          v8::String::Utf8Value propertyName(key);
          std::pair<std::string, avro::GenericDatum> leaf(*propertyName, DecodeV8(mapped, map->Get(key), reference));
          v.push_back(leaf);
        }
        genMap.value() = v;

        datum.value<avro::GenericMap>() = genMap;
      }
      break;     
    case avro::AVRO_ENUM:  
      {
        avro::GenericEnum &genEnum = datum.value<avro::GenericEnum>();
        if(object->IsString()){
          v8::String::Utf8Value avroString(object->ToString());
          genEnum.set(*avroString);
        }else{
          genEnum.set(object->Int32Value());
        }
        datum.value<avro::GenericEnum>() = genEnum;
      }
      break;

    case avro::AVRO_FIXED:
      {
        avro::GenericFixed &genFixed = datum.value<avro::GenericFixed>();
        v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(object);
        //get length of buffer
        std::vector<uint8_t> bytes(array->Length());
        for(uint32_t i = 0;i<array->Length();i++){
          bytes[i] = array->Get(i)->Int32Value();
        }
        genFixed.value() = bytes;
        datum.value<avro::GenericFixed>() = genFixed;
      }
      break;
    //Unimplemented avro types

    case avro::AVRO_SYMBOLIC:

    case avro::AVRO_UNKNOWN:

    default:
      {
        printf("unimplemented avro type\n");
        return datum;
      }
  }
  return datum;
}

/**
 * [Takes in the pointer of a datum which is an union. It will loop through the
 * branches checking for the type that was supplied in the JSON format (aka type). 
 * Once a match is made The GenericDatum will be left at the current selected branch and
 * control returned to the calling function]
 * @param datum [pointer to the GenericDatum]
 * @param type  [The type of Union specified by the JSON]
 */
void unionBranch(avro::GenericDatum *datum, const char *type){
  try{

    int branches = datum->unionBranch();
    for(int i = 0; i<branches;i++){
      datum->selectBranch(i);

      switch(datum->type())
      {
        case avro::AVRO_RECORD:
          {
            //Get the name of the schema.
            // tried splitting this out just to cast to GenericContainer
            // to abstract it but got seg fault errors. 
            avro::GenericRecord record = datum->value<avro::GenericRecord>();
            avro::NodePtr node = record.schema();
            const std::string name = node->name();
            if(strcmp(type,"record")  == 0 || strcmp(name.c_str(),type) == 0){
              return;
            }
          }
          break;
        case avro::AVRO_STRING:
          {
            if(strcmp(type,"string") == 0){
              return;
            }
          }
          break;
        case avro::AVRO_BYTES:
          if(strcmp(type,"bytes")  == 0){
            return;
          }
          break;
        case avro::AVRO_INT:
          if(strcmp(type,"int")  == 0){
            return;
          }
          break;
        case avro::AVRO_LONG:
          if(strcmp(type,"long")  == 0){
            return;
          }
          break;
        case avro::AVRO_FLOAT:
          if(strcmp(type,"float")  == 0){
            return;
          }
          break;
        case avro::AVRO_DOUBLE:
          if(strcmp(type,"double")  == 0){
            return;
          }
          break;
        case avro::AVRO_BOOL:
          if(strcmp(type,"bool")  == 0){
            return;
          }
          break;
        case avro::AVRO_NULL:
          if(strcmp(type,"null") == 0){
            return;
          }
          break;
        case avro::AVRO_ARRAY:
          if(strcmp(type,"array")  == 0){
            return;
          }
          break;
        case avro::AVRO_MAP:
          if(strcmp(type,"map")  == 0){
            return;
          }
          break;
        case avro::AVRO_FIXED:
          {
            avro::GenericFixed record = datum->value<avro::GenericFixed>();
            avro::NodePtr node = record.schema();            
            const std::string name = node->name();
            if(strcmp(type,"fixed")  == 0 || strcmp(name.c_str(),type) == 0){
              return;
            }
          }
          break;
        case avro::AVRO_UNION:
          if(strcmp(type,"union")  == 0){
            return;
          }
          break;
        case avro::AVRO_ENUM:
          {
            avro::GenericEnum record = datum->value<avro::GenericEnum>();
            avro::NodePtr node = record.schema();            
            const std::string name = node->name();
            if(strcmp(type,"enum")  == 0 || strcmp(name.c_str(),type) == 0){
              return;
            }
          }
          break;
        case avro::AVRO_SYMBOLIC:
          printf("We have a Symbolic of %s\n", type);
          break;
        case avro::AVRO_UNKNOWN:
          printf("We have a Unknown of %s\n", type);
          break;
        default:
          {
            printf("unimplemented avro type\n");
          }
      }
    }
  
  }catch(std::exception &e){

  }
}
