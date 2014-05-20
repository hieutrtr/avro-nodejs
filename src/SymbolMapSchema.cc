#include "SymbolMapSchema.h"


static void validate(const NodePtr& p, SymbolMap& m)
{
  helper::validate(p, m);
}


SymbolMapSchema::SymbolMapSchema(const NodePtr &root, const SymbolMap & symbols) :root_(root), map_(symbols)
{
  helper::validate(root_, map_);
}
SymbolMapSchema::SymbolMapSchema(const NodePtr &root): root_(root)
{

  printf("leaves in symmapsch: %d internal: %d\n",root->leaves(), root_->leaves()); 
  SymbolMap m;
  map_ = m;
  helper::validate(root_, map_);
  printf("leaves in symmapsch: %d internal: %d\n",root->leaves(), map_.size()); 
}

void SymbolMapSchema::toJson(std::ostream &os) const
{
  root_->printJson(os, 0);
  os << '\n';
}


