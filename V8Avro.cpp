#include <string>
#include <vector>
#include <exception>
#include <sstream>

#include "V8AvroBase.h"

using namespace std;

class V8Avro : public V8AvroBase
{
    public:
        V8Avro(){};
        ~V8Avro()
        {
            for(vector<Persistent<Object> >::iterator i = instanceList.begin(); i != instanceList.end(); i++)
            {
                (*i).Dispose();
            }
        };
        
        bool initialize();
        
        void runReadyStateChange();
        Handle<Function> getReadyStateChange(Local<Object> obj);
        
    private:
        //Explicitly disallowing copy constructor
        V8Avro(const V8Avro &cpy){};
        
        static Handle<Value>callAsFuncHandler(const Arguments &args);
        static Handle<Value>openHandler(const Arguments &args);
        
        vector<Persistent<Object> > instanceList;
};

bool V8Avro::initialize()
{
    HandleScope scope;
    
    Local<ObjectTemplate> V8AvroObj = ObjectTemplate::New();
    V8AvroObj->SetCallAsFunctionHandler(callAsFuncHandler, classPtrToExternal());
    
    Local<ObjectTemplate> globals = ObjectTemplate::New();
    globals->Set(static_cast<Handle<String> >(String::New("V8Avro")), V8AvroObj, ReadOnly);
    
    V8TutorialBase::initialize(globals);
    
    return true;
}

Handle<Function> V8Avro::getReadyStateChange(Local<Object> obj)
{
    Local<String> funcName = String::New("onreadystatechange");
    if(obj->Has(funcName))
    {
        Local<Value> tmp = obj->Get(funcName);
        if(tmp->IsNull())
        {
            log("onreadystatechange is null for this instance\n");
        }
        else
        {
            Local<Function> func = Function::Cast(*(obj->Get(funcName)));
            if(!func.IsEmpty())
            {
                return func;
            }    
        }
    }
    
    cout<<"Can't find onreadystatechange"<<endl;
    Local<FunctionTemplate> funcTmp = FunctionTemplate::New();
    return funcTmp->GetFunction();
}

void V8Avro::runReadyStateChange()
{
    HandleScope scope;
    
    vector<Persistent<Object> >::iterator i;
    for(i=instanceList.begin(); i != instanceList.end(); i++)
    {
        Local<Function> func = Local<Function>::New(getReadyStateChange(*(*i)));
        func->Call(getContext()->Global(), 0, NULL);
    }
}

Handle<Value> V8Avro::callAsFuncHandler(const Arguments &args)
{
    HandleScope scope;
    
    cout<<__PRETTY_FUNCTION__<<" called"<<endl;
    
    V8Avro *objPtr = externalToClassPtr<V8Avro>(args.Data());
    
    Local<ObjectTemplate> instanceTemplate = ObjectTemplate::New();
    instanceTemplate->SetInternalFieldCount(1);
    
    //Set readyState property
    Local<Number> tmpNum = Number::New(0);
    instanceTemplate->Set(static_cast<Handle<String> >(String::New("readyState")), tmpNum);
    
    //Set onreadystatechange property to an empty function
    Local<FunctionTemplate> funcTmp = FunctionTemplate::New();
    instanceTemplate->Set(static_cast<Handle<String> >(String::New("onreadystatechange")), funcTmp);

    //Set onschema property to an empty function
    Local<FunctionTemplate> funcTmp = FunctionTemplate::New();
    instanceTemplate->Set(static_cast<Handle<String> >(String::New("onschema")), funcTmp);

    //Set onstreamend property to an empty function
    Local<FunctionTemplate> funcTmp = FunctionTemplate::New();
    instanceTemplate->Set(static_cast<Handle<String> >(String::New("onstreamend")), funcTmp);

    

    //Set open property
    Local<FunctionTemplate> tmpOpen = objPtr->makeStaticCallableFunc(openHandler);
    instanceTemplate->Set(static_cast<Handle<String> >(String::New("open")), tmpOpen, ReadOnly);
    
    Local<Object> instance = instanceTemplate->NewInstance();
    instance->SetInternalField(0, Integer::New(objPtr->instanceList.size()));
    
    cout<<"Internal field set to "<<instance->GetInternalField(0)->ToNumber()->Value()<<endl;
    objPtr->instanceList.push_back(Persistent<Object>::New(instance));
    
    return scope.Close(instance);
}

Handle<Value> V8Avro::openHandler(const Arguments &args)
{
    HandleScope scope;
    cout<<__PRETTY_FUNCTION__<<" called"<<endl;
    
    Local<Object> thisObj = args.This(); //*(objPtr->instanceList[num->Value()]);
    if(thisObj.IsEmpty())
    {
        cout<<"Error getting this object"<<endl;
        return v8::Null();
    }
    
    Local<String> propName = String::New("readyState");
    if(thisObj->Has(propName))
    {
        Local<Number> stateNum = thisObj->Get(propName)->ToNumber();
        cout<<"Current value of readyState: "<<stateNum->Value()<<endl;
        thisObj->Set(propName, Number::New(1));
        cout<<"New value of readyState: "<<thisObj->Get(propName)->ToNumber()->Value()<<endl;
    }
    
    return v8::Null();
}

int main()
{
    V8Avro testObj;
    if(!testObj.initialize())
    {
        cout<<"Failed to initialize"<<endl;
        return 1;
    }
    else
        cout<<"intialized"<<endl;
    
    string code = testObj.readFile("V8Avro_test.js");
    if(code.length())
    {
        if(!testObj.runScript(code))
            cout<<"Failed to run code"<<endl;
        else
        {
            cout<<"Script ran"<<endl;
            testObj.runReadyStateChange();
            cout<<"runReadyStateChange executed"<<endl;
        }
    }
    else
        cout<<"Failed to read file"<<endl;
        
    return 0;
}
