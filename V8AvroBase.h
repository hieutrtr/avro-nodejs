#ifndef V8_AVRO_BASE_H
#define V8_AVRO_BASE_H

#include <iostream>
#include <v8.h>
#include <string>

using namespace v8;
using namespace std;


///////////////////////////////////////////////////////////////////////////////
//
// This class is NOT intended to be used directly
// Users should inherit this class and do whatever else they need to do.
//
// NOTE:
// Child classes MUST call destructor to make sure that the Context is disposed
// of accordingly. Unless, you want to suck up memory be my guest
//
///////////////////////////////////////////////////////////////////////////////
class V8AvroBase
{
  public:
    //Destructor
    virtual ~V8AvroBase(){ cout<<__PRETTY_FUNCTION__<<" called"<<endl; context.Dispose(); };
    
  protected:
    ////////////////////////////////////////////////////////////////////////
    //
    // Constructor. This was made protected on purpose to prevent creating
    // instances of this class.
    //
    ////////////////////////////////////////////////////////////////////////
    explicit V8AvroBase(){};
    
    
    V8AvroBase(const V8AvroBase &cpy);
    
    void initialize(Local<ObjectTemplate> passedGlobals = Local<ObjectTemplate>());
    virtual void log(const string &str);
    static Handle<Value> logCallback(const Arguments &args);
    
    Handle<Context> getContext()
    {
      return context;
    };
    
    Local<FunctionTemplate> makeStaticCallableFunc(InvocationCallback func);
    Local<External> classPtrToExternal();

    ////////////////////////////////////////////////////////////////////////
    //
    // Converts an External to a V8AvroBase pointer. This assumes that the
    // data inside the v8::External is a "this" pointer that was wrapped by
    // makeStaticCallableFunc
    //
    // \parameter data Shoudld be v8::Arguments::Data()
    //
    // \return "this" pointer inside v8::Arguments::Data() on success, NULL otherwise
    //
    ////////////////////////////////////////////////////////////////////////        
    template <typename T>
    static T *externalToClassPtr(Local<Value> data)
    {
      if(data.IsEmpty())
        cout<<"Data empty"<<endl;
      else if(!data->IsExternal())
        cout<<"Data not external"<<endl;
      else
        return static_cast<T *>(External::Unwrap(data));
          
      //If function gets here, one of the checks above failed
      return NULL;
    }
      
  private:
    Persistent<Context> context;
};

#endif