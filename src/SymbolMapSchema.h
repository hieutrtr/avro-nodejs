#ifndef SYMBOLMAPSCHEMA_H
#define SYMBOLMAPSCHEMA_H

#include <avro/ValidSchema.hh>
#include "helpers.h"


using namespace v8;
using namespace node;
using namespace avro;
using namespace helper;
namespace avro {

class SymbolMapSchema: public ValidSchema {

  public:
    SymbolMapSchema(const NodePtr &root, const SymbolMap &symbolMap);
    SymbolMapSchema(const NodePtr &root);
    void toJson(std::ostream &os) const;
    const NodePtr &root() const {
      return root_;
    }
    ~SymbolMapSchema();

  private:
    static void validate(const NodePtr& p, SymbolMap &m);
    NodePtr root_;
    SymbolMap map_;

};
}

#endif /* SYMBOLMAPSCHEMA_H */
