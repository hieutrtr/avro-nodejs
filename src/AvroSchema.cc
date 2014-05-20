#include <node.h>
#include "AvroSchema.h"


Persistent<Function> AvroSchema::constructor;

AvroSchema::AvroSchema(const char* jsonSchema){
  std::istringstream is(jsonSchema);
  ValidSchema schema;
  try{
    compileJsonSchema(is, schema);
    schema_  = new SymbolMapSchema(schema.root());
  }catch(std::exception &e){
    ThrowException(String::New(e.what()));
  }
}
AvroSchema::AvroSchema(const NodePtr &root){
  try{
    schema_ = new SymbolMapSchema(root);

    printf("Leaves: %d\n",schema_->root()->leaves());
  }catch(std::exception &e){
    ThrowException(String::New(e.what()));
  }
}

/**
 *
 * @param root: The root Node element.
 * @param symbols: The Symbol map of known types.
 */
AvroSchema::AvroSchema(const NodePtr &root, SymbolMap &symbols){
  try{
    schema_ = new SymbolMapSchema(root,symbols);
  }catch(std::exception &e){
    ThrowException(String::New(e.what()));
  }
}

/**
 * Destructor call for AvroSchema
 * TODO should free items it has handles to.
 */
AvroSchema::~AvroSchema() {

}

/**
 * Inits the FunctionTemplate for the AvroSchema object.
 */
void AvroSchema::Init(Handle<Object> exports){

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("AvroSchema"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  //Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "toJson", AvroSchema::ToJson);

  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("AvroSchema"), constructor);
}

/**
 * New constructor call from javascript.
 */
Handle<Value> AvroSchema::New(const Arguments& args){
  HandleScope scope;
  if(args.IsConstructCall()){
    if(args[0]->IsExternal()){
      AvroSchema *obj = (AvroSchema*)External::Unwrap(args[0]);
      obj->Wrap(args.This());
      return args.This();
    }
    String::Utf8Value jsonString(args[0]->ToString());
    
    const char* jsonSchema = args[0]->IsUndefined() ? "": *(jsonString);
    AvroSchema* schema = new AvroSchema(jsonSchema);
    schema->Wrap(args.This());
    return args.This();
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = {args[0]};
    return scope.Close(constructor->NewInstance(argc, argv));

  }
}

/**
 * ToNode() wraps the the c++ object in v8 value so that
 * it can be passed back to the javscript world.
 */
Handle<Value> AvroSchema::ToNode(){
  HandleScope scope;

  Handle<External> external = External::New(this);

  const int argc = 1;
  Handle<Value> args[argc] = {external};

  return scope.Close(constructor->NewInstance(argc, args));
}

Handle<Value> AvroSchema::ToJson(const Arguments& args){
  HandleScope scope;

  AvroSchema *obj = ObjectWrap::Unwrap<AvroSchema>(args.This());
  std::ostringstream oss;
  obj->schema_->toJson(oss);

  return scope.Close(String::New(oss.str().c_str()));

}
