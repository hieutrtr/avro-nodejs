#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <vector>

namespace helper {
  using namespace std;
  using namespace node;
  using namespace v8;
  
  vector<uint8_t> getBinaryData(Local<Value> val);

}
