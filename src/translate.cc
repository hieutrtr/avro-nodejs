#include "translate.h"


Handle<Value> DecodeAvro(const GenericDatum& datum){
  return DecodeAvro(datum, Array::New());
}

/**
 * converts a GenericDatum into a v8 object that can be passed back to javascript
 * @param datum [Turning the avro GenericDatum into a v8 object that we can pass back to javascript]
 * @return returns the v8 object that can be used by javascript.
 */
Handle<Value> DecodeAvro(const GenericDatum& datum,  Local<Array> reference){
  Handle<Object> obj = Object::New();
  //return this Object
  switch(datum.type())
  {
    case AVRO_RECORD:
      {
        const GenericRecord& record = datum.value<GenericRecord>();
        const NodePtr& node = record.schema();
        Handle<Object> obj = Object::New();
        for(uint i = 0; i<record.fieldCount(); i++){
          Local<String> datumName = String::New(node->nameAt(i).c_str(), node->nameAt(i).size());

          const GenericDatum& subDatum = record.fieldAt(i);
          if(!strcmp("com.gensler.scalavro.Reference", node->name().fullname().c_str())){
            return reference->Get(DecodeAvro(subDatum, reference)->Int32Value());
          }else {
            Handle<Value> tmp = DecodeAvro(subDatum, reference);
            obj->Set(datumName, tmp);
          }
        }
        reference->Set(reference->Length(), obj);
        return obj;
      }
    case AVRO_STRING:
      {
        return  v8::String::New(
          datum.value<std::string>().c_str(),
          datum.value<std::string>().size()
        );
      } 
    case AVRO_BYTES:
      {
        Local<Array> byteArray = Array::New();
        const vector<uint8_t> &v = datum.value<vector<uint8_t> >();
        for(size_t i = 0;i<v.size();i++){
          byteArray->Set(i, Int32::New(v[i]));
        }
        return byteArray;
      }        
    case AVRO_INT:
      return Number::New(datum.value<int32_t>());
    case AVRO_LONG:
      return Number::New(datum.value<int64_t>());
    case AVRO_FLOAT:
      return Number::New(datum.value<float>());
    case AVRO_DOUBLE:
      return Number::New(datum.value<double>());
    case AVRO_BOOL:
    {
      Handle<Boolean> boolVal = Boolean::New(datum.value<bool>());
      return boolVal;
    }
    case AVRO_NULL:
      return v8::Null();
    case AVRO_ARRAY:
      {
        const GenericArray &genArray = datum.value<GenericArray>();

        const vector<GenericDatum> &v = genArray.value();
        Local<Array> datumArray = Array::New();
        int i = 0;
        for(vector<GenericDatum>::const_iterator it = v.begin(); it != v.end(); ++it) {
          const GenericDatum &itDatum = * it;
          datumArray->Set(i, DecodeAvro(itDatum, reference));
          i++;
        }
        return datumArray;
      }
    case AVRO_MAP:
      {
        const GenericMap &genMap = datum.value<GenericMap>();

        const vector < std::pair < std::string, GenericDatum > > &v = genMap.value();
        Local<Array> datumArray = Array::New();
        int i = 0;
        for(vector< std::pair < std::string, GenericDatum> >::const_iterator it = v.begin(); it != v.end(); ++it) {
          const std::pair < std::string, GenericDatum> &itDatum = * it;
          datumArray->Set(v8::String::New(
            itDatum.first.c_str(),
            itDatum.first.size()
            ),DecodeAvro(itDatum.second, reference));

          i++;
        }
        return datumArray;
      }

    case AVRO_FIXED:
      {
        
        Local<Array> fixedBytes = Array::New();
        const GenericFixed &genFixed = datum.value<GenericFixed>();
        const vector<uint8_t> &v = genFixed.value();
        for(size_t i = 0;i<v.size();i++){
          fixedBytes->Set(i, Uint32::New(v[i]));
        }
        return fixedBytes;
      }
    case AVRO_ENUM:
      {
        const GenericEnum &genEnum = datum.value<GenericEnum>();
        int32_t enumVal = genEnum.value();
        return Number::New(enumVal);
      }

    //Unimplemented avro types
    case AVRO_SYMBOLIC:

    case AVRO_UNKNOWN:

    case AVRO_UNION:

    default:
      {
        printf("data type %d\n", datum.type());
        return obj;
      }
  }

}

GenericDatum DecodeV8(GenericDatum datum, Local<Value> object){
  vector<int> reference;
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
GenericDatum DecodeV8(GenericDatum datum, Local<Value> object, vector<int> *reference){

  //if the datum is a union we want to try
  // and pick the right branch. 
  if(datum.isUnion()){
    if(!object->IsNull()){
      Local<Object> obj = object->ToObject();
      //Set the object to the value of the union. 
      Local<Value> typeObject;
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
    case AVRO_RECORD:
      {
        Local<Object> obj = object->ToObject();
        int identityHash = obj->GetIdentityHash();
        //we can either have an object or a reference type (0,1)
        // this determines whether or not we can repeat. 
        vector<int>::iterator iter;
        if((iter = std::find(reference->begin(), reference->end(),identityHash)) != reference->end()){
          size_t index = std::distance(reference->begin(), iter); 
          datum.selectBranch(1);
          obj = Object::New();
          obj->Set(v8::String::New("id"), NumberObject::New(index));
        }
        const GenericRecord& record = datum.value<GenericRecord>();
        const NodePtr& node = record.schema();
        for(uint i = 0; i<record.fieldCount(); i++){
          //Add values
          Local<v8::String> datumName = v8::String::New(node->nameAt(i).c_str(), node->nameAt(i).size());
          datum.value<GenericRecord>().fieldAt(i) = DecodeV8(record.fieldAt(i), obj->Get(datumName), reference);
        }
        reference->push_back(identityHash);
      }
      break;
    case AVRO_STRING:
      {
        v8::String::Utf8Value avroString(object->ToString());
        datum.value<std::string>() = *avroString;
      }
      break;
    case AVRO_BYTES:
      {
        datum.value<vector<uint8_t> >() = helper::getBinaryData(object);
      }
      break;       
    case AVRO_INT:
      datum.value<int32_t>() = object->Int32Value() ;
      break;       
    case AVRO_LONG:
      datum.value<int64_t>() = object->IntegerValue() ;
      break;       
    case AVRO_FLOAT:
      datum.value<float>() = static_cast<float>(object->NumberValue());
      break;       
    case AVRO_DOUBLE:
      datum.value<double>() = object->NumberValue();
      break;       
    case AVRO_BOOL:
      datum.value<bool>() = object->ToBoolean()->Value();
      break;
    case AVRO_NULL:
      break;       
    case AVRO_ARRAY:
      {
        Local<Array> array = Local<Array>::Cast(object);
        GenericArray &genArray = datum.value<GenericArray>();
        vector<GenericDatum> &v = genArray.value();
        const NodePtr& node = genArray.schema();
        //gets the second value of the map. The first is always string as defined by avro
        GenericDatum item(node->leafAt(0));

        for( uint32_t i = 0;i<array->Length();i++){
          v.push_back(DecodeV8(item, array->Get(i), reference));
        }
        genArray.value() = v;
        datum.value<GenericArray>() = genArray;
      }
      break;
    case AVRO_MAP:
      {
        Local<Object> map = object->ToObject();
        GenericMap &genMap = datum.value<GenericMap>();
        const NodePtr& node = genMap.schema();
        //gets the second value of the map. The first is always string as defined by avro
        GenericDatum mapped(node->leafAt(1));
        Local<Array> propertyNames = map->GetPropertyNames();
        vector < std::pair < std::string, GenericDatum > > &v = genMap.value();;
        for(size_t i = 0;i<propertyNames->Length();i++){
          Local<String> key = propertyNames->Get(i)->ToString();
          String::Utf8Value propertyName(key);
          pair<string, GenericDatum> leaf(*propertyName, DecodeV8(mapped, map->Get(key), reference));
          v.push_back(leaf);
        }
        genMap.value() = v;

        datum.value<GenericMap>() = genMap;
      }
      break;     
    case AVRO_ENUM:  
      {
        GenericEnum &genEnum = datum.value<GenericEnum>();
        if(object->IsString()){
          String::Utf8Value avroString(object->ToString());
          genEnum.set(*avroString);
        }else{
          genEnum.set(object->Int32Value());
        }
        datum.value<GenericEnum>() = genEnum;
      }
      break;

    case AVRO_FIXED:
      {
        GenericFixed &genFixed = datum.value<GenericFixed>();
        //get length of buffer
        genFixed.value() = helper::getBinaryData(object);
        datum.value<GenericFixed>() = genFixed;
      }
      break;
    //Unimplemented avro types

    case AVRO_SYMBOLIC:

    case AVRO_UNKNOWN:

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
void unionBranch(GenericDatum *datum, const char *type){
  try{

    int branches = datum->unionBranch();
    for(int i = 0; i<branches;i++){
      datum->selectBranch(i);

      switch(datum->type())
      {
        case AVRO_RECORD:
          {
            //Get the name of the schema.
            // tried splitting this out just to cast to GenericContainer
            // to abstract it but got seg fault errors. 
            GenericRecord record = datum->value<GenericRecord>();
            NodePtr node = record.schema();
            const std::string name = node->name();
            if(strcmp(type,"record")  == 0 || strcmp(name.c_str(),type) == 0){
              return;
            }
          }
          break;
        case AVRO_STRING:
          {
            if(strcmp(type,"string") == 0){
              return;
            }
          }
          break;
        case AVRO_BYTES:
          if(strcmp(type,"bytes")  == 0){
            return;
          }
          break;
        case AVRO_INT:
          if(strcmp(type,"int")  == 0){
            return;
          }
          break;
        case AVRO_LONG:
          if(strcmp(type,"long")  == 0){
            return;
          }
          break;
        case AVRO_FLOAT:
          if(strcmp(type,"float")  == 0){
            return;
          }
          break;
        case AVRO_DOUBLE:
          if(strcmp(type,"double")  == 0){
            return;
          }
          break;
        case AVRO_BOOL:
          if(strcmp(type,"bool")  == 0){
            return;
          }
          break;
        case AVRO_NULL:
          if(strcmp(type,"null") == 0){
            return;
          }
          break;
        case AVRO_ARRAY:
          if(strcmp(type,"array")  == 0){
            return;
          }
          break;
        case AVRO_MAP:
          if(strcmp(type,"map")  == 0){
            return;
          }
          break;
        case AVRO_FIXED:
          {
            GenericFixed record = datum->value<GenericFixed>();
            NodePtr node = record.schema();            
            const std::string name = node->name();
            if(strcmp(type,"fixed")  == 0 || strcmp(name.c_str(),type) == 0){
              return;
            }
          }
          break;
        case AVRO_UNION:
          if(strcmp(type,"union")  == 0){
            return;
          }
          break;
        case AVRO_ENUM:
          {
            GenericEnum record = datum->value<GenericEnum>();
            NodePtr node = record.schema();            
            const std::string name = node->name();
            if(strcmp(type,"enum")  == 0 || strcmp(name.c_str(),type) == 0){
              return;
            }
          }
          break;
        case AVRO_SYMBOLIC:
          printf("We have a Symbolic of %s\n", type);
          break;
        case AVRO_UNKNOWN:
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
