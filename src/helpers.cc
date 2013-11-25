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


bool validate(const NodePtr &node, SymbolMap &symbolMap) 
{
    if (! node->isValid()) {
        throw avro::Exception(boost::format("Schema is invalid, due to bad node of type %1%")
            % node->type());
    }

    if (node->hasName()) {
        const Name& nm = node->name();
        SymbolMap::iterator it = symbolMap.find(nm);
        bool found = it != symbolMap.end() && nm == it->first;

        if (node->type() == AVRO_SYMBOLIC) {
            if (! found) {
                throw avro::Exception(boost::format("Symbolic name \"%1%\" is unknown") %
                    node->name());
            }

            boost::shared_ptr<NodeSymbolic> symNode =
                boost::static_pointer_cast<NodeSymbolic>(node);

            // if the symbolic link is already resolved, we return true,
            // otherwise returning false will force it to be resolved
            return symNode->isSet();
        }

        if (found) {
            return false;
        }
        symbolMap.insert(it, make_pair(nm, node));
    }

    node->lock();
    size_t leaves = node->leaves();
    for (size_t i = 0; i < leaves; ++i) {
        const NodePtr &leaf(node->leafAt(i));

        if (! validate(leaf, symbolMap)) {

            // if validate returns false it means a node with this name already
            // existed in the map, instead of keeping this node twice in the
            // map (which could potentially create circular shared pointer
            // links that could not be easily freed), replace this node with a
            // symbolic link to the original one.
            
            node->setLeafToSymbolic(i, symbolMap.find(leaf->name())->second);
        }
    }
    return true;
}


}
