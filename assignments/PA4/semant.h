#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;

public:
  ClassTable(Classes);
  bool dfs_check(int, bool*, bool*);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
  void scoping_check();
  void rec_add_features();
  bool check_typerror(Symbol, Symbol, tree_node*);
  void add_all_features(Class_, SymbolTable<Entry*, Entry>*, SymbolTable<Entry*, Feature_class>*);
  void rec_add_features(Class_, SymbolTable<Entry*, Entry>*, SymbolTable<Entry*, Feature_class>*);
  void scoping_build();
  void type_checking();
  bool check_inheritance(Symbol, Symbol);
  Feature get_def_method(Symbol, Symbol);
  Symbol lca(Symbol, Symbol);
  Class_ lookup(Symbol);
};

Classes all_classes;
int class_number;
SymbolTable<Symbol, int> *name2id;
SymbolTable<int, int> *inheritance;
SymbolTable<Symbol, SymbolTable<Symbol, Entry> > *attr_table;
SymbolTable<Symbol, SymbolTable<Symbol, Feature_class> > *method_table;
SymbolTable<Symbol, Class__class> *name2class;
#endif

