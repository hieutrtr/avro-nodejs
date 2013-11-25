#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <avro/Generic.hh>
#include <avro/ValidSchema.hh>
#include <avro/Schema.hh>
#include <vector>

namespace helper {
  using namespace std;
  using namespace node;
  using namespace v8;
  using namespace avro;
  
  vector<uint8_t> getBinaryData(Local<Value> val);

  typedef std::map<Name, NodePtr> SymbolMap;

  bool validate(const NodePtr &node, SymbolMap &symbolMap);

}
