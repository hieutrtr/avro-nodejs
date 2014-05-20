#include "AvroProtocol.h"
#include <string.h>

//
Persistent<Function> AvroProtocol::constructor;

/**
 *
 * AvroProtocol creates a dictionary of types from a provided 
 * javascript object.
 * @param protocol: The javascript protocol object.
 */
AvroProtocol::AvroProtocol(std::string name, std::string nameSpace, std::string doc){
  name_ = name;
  namespace_ = nameSpace;
  doc_ = doc;
}

/**
 *
 */
AvroProtocol::~AvroProtocol() {

}

/**
 *
 *
 */
void AvroProtocol::Init(Handle<Object> exports){
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("AvroProtocol"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  //Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "createMessage", AvroProtocol::CreateMessage);
  NODE_SET_PROTOTYPE_METHOD(tpl, "setTypes", AvroProtocol::SetTypes);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getName", AvroProtocol::GetName);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getDoc", AvroProtocol::GetDoc);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getNamespace", AvroProtocol::GetNamespace);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getType", AvroProtocol::GetType);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getTypes", AvroProtocol::GetTypes);

  
  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("AvroProtocol"), constructor);

}


/**
 * A protocol JSObject is passed in to create a a AvroProtocol.
 * It is then parsed out to the correct parts.
 *
 */
Handle<Value> AvroProtocol::New(const Arguments& args){
  HandleScope scope;

  if(args.IsConstructCall()){
    Local<Object> obj = args[0]->ToObject();
    Local<Value> doc(obj->Get(String::New("doc")));
    Local<Value> Namespace(obj->Get(String::New("namespace")));
    Local<Value> name(obj->Get(String::New("protocol")));
    Local<Value> types(obj->Get(String::New("types")));
   

    const char* argDoc = doc->IsUndefined() ? "": *(String::Utf8Value(doc));
    const char* argNamespace = Namespace->IsUndefined() ? "": *(String::Utf8Value(Namespace));

    std::string argName = *(String::Utf8Value(name));
    Handle<Context> context = Context::GetCurrent();
    Handle<Object> global = context->Global();

    Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
    Handle<Function> JSON_stringify = Handle<Function>::Cast(JSON->Get(String::New("stringify")));

    Local<Value> argVals[1] = {types};
    Handle<Value> result = JSON_stringify->Call(global,1, argVals);

    ValidSchema schema;
    std::istringstream is(*(String::Utf8Value(result->ToString())));
    compileJsonSchema(is, schema);

   
    AvroSchema* schemaTypes = new AvroSchema(schema.root());

    AvroProtocol* protocol;

    protocol = new AvroProtocol(argName, "", "");
    protocol->SetTypes(schemaTypes);
    
    protocol->Wrap(args.This());
    return args.This();
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = {args[0]};
    return scope.Close(constructor->NewInstance(argc, argv));
  }

}

Handle<Value> AvroProtocol::SetTypes(AvroSchema* schema){
  HandleScope scope;

  try{
    helper::validate(schema->root(),dictionary_);
  }catch(std::exception &e){
    ThrowException(String::New(e.what()));
  }

  return scope.Close(Undefined());
}
/**
 * Assumes all of the types are a union schema of all of the
 * possible types for the avro protocol.
 *
 */
Handle<Value> AvroProtocol::SetTypes(const Arguments& args){
  HandleScope scope;
  AvroProtocol *protocol = ObjectWrap::Unwrap<AvroProtocol>(args.This());
  //grab schema out of first parameter
  AvroSchema *schema = ObjectWrap::Unwrap<AvroSchema>(args[0]->ToObject());

  try{
    helper::validate(schema->root(),protocol->dictionary_);
  }catch(std::exception &e){
    ThrowException(String::New(e.what()));
  }

  return scope.Close(Undefined());
}

/**
 *
 */
Handle<Value> AvroProtocol::CreateMessage(const Arguments& args){
  HandleScope scope;
  AvroProtocol *protocol = ObjectWrap::Unwrap<AvroProtocol>(args.This());

  return scope.Close(Undefined());

}

Handle<Value> AvroProtocol::GetDoc(const Arguments& args){
  HandleScope scope;
  AvroProtocol *protocol = ObjectWrap::Unwrap<AvroProtocol>(args.This());

  return scope.Close(String::New(protocol->doc_.c_str()));

}

Handle<Value> AvroProtocol::GetName(const Arguments& args){
  HandleScope scope;
  AvroProtocol *protocol = ObjectWrap::Unwrap<AvroProtocol>(args.This());

  return scope.Close(String::New(protocol->name_.c_str()));
}

Handle<Value> AvroProtocol::GetNamespace(const Arguments& args){
  HandleScope scope;
  AvroProtocol *protocol = ObjectWrap::Unwrap<AvroProtocol>(args.This());

  return scope.Close(String::New(protocol->namespace_.c_str()));

}

/**
 * Gets the associated AvroSchema from the protocol. 
 */
Handle<Value> AvroProtocol::GetType(const Arguments& args){
  HandleScope scope;
  AvroProtocol *protocol = ObjectWrap::Unwrap<AvroProtocol>(args.This());
  Name *fullName = new Name(*(String::Utf8Value(args[0])));
  helper::SymbolMap::iterator it;
  if((it = protocol->dictionary_.find(*fullName)) != protocol->dictionary_.end()){
    it->second;
    /*
    for(int i = 0;i<it->second->leaves();i++){
      printf("%s\n", it->second->nameAt(i).c_str());
 //     std::string fullname = ptr->name().fullname(); 
      printf("is Type: %d \n",it->second->leafAt(i)->leaves());
      for(int j = 0;j<it->second->leafAt(i)->leaves();j++){
        NodePtr ptr = it->second->leafAt(i)->leafAt(j);
        NodePtr result = resolveSymbol(ptr);
        
        printf("type: %d\n",result->type());
        
      }
//      printf("FullName of leaf: %s\n", fullname.c_str());
    }
    */
    AvroSchema* schema = new AvroSchema(it->second,protocol->dictionary_ );
    return schema->ToNode();
  }
 
  return scope.Close(Undefined());
}
/**
 * Gets array of AvroSchema types from the protocol
 */
Handle<Value> AvroProtocol::GetTypes(const Arguments& args){
  HandleScope scope;
  AvroProtocol *protocol = ObjectWrap::Unwrap<AvroProtocol>(args.This());
  //Create new array
  int count = 0;
  Handle<Array> types = Array::New((int)protocol->dictionary_.size());
  for(helper::SymbolMap::iterator iterator = protocol->dictionary_.begin(); iterator != protocol->dictionary_.end(); iterator++){
    AvroSchema *schema = new AvroSchema(iterator->second, protocol->dictionary_);
    types->Set(count,schema->ToNode());
    count++;
  }

  return scope.Close(types);

}
