

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"


extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");

    
    all_classes = nil_Classes();
}



ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {

    /* Fill this in */
    install_basic_classes();

    all_classes = append_Classes(all_classes, classes);
    class_number = all_classes->len();
    cout << "install basic classes" << std::endl;
    name2id = new SymbolTable<Symbol, int>();
    name2id -> enterscope();
    inheritance = new SymbolTable<int, int>();
    inheritance -> enterscope();
    name2class = new SymbolTable<Symbol, Class__class>();
    name2class -> enterscope();

    cout << "build inheritance" << std::endl;
    // build inheritance graph
    Class_ class_i;
    Symbol name_i, parname;
    int j = 0;
    name2id -> addid(No_class, new int(j++));
    for(int i = all_classes->first(); all_classes->more(i); i = all_classes->next(i)) {
        class_i = all_classes->nth(i);
        name_i = class_i->getname();
        if(name2id->lookup(name_i) != NULL) {
            // redefine
            semant_error(class_i) << "Class " << name_i << " redefine" << std::endl;
        }
        else {
            name2id -> addid(name_i, new int(j++));
            name2class -> addid(name_i, class_i);
        }
    }
    if(name2id->lookup(Main) == NULL) {
        semant_error() << "Class Main is not defined" << std::endl;
    }
    int *par_i, *id;
    cout << "build class table" << std::endl;
    for(int i = all_classes->first(), j = 0; all_classes->more(i); i = all_classes->next(i)) {
        class_i = all_classes->nth(i);
        name_i = class_i->getname();
        parname = class_i->getparent();
        par_i = name2id -> lookup(parname);
        id = name2id -> lookup(name_i);
        if(par_i == NULL) {
            semant_error(class_i) << "Class " << name_i << ": parent class " << parname << "not define" << std::endl;
        }
        else {
            cout << name_i << " is child of " << parname << std::endl;
            if(id == NULL) cout << "???" << std::endl;
            inheritance->addid(*id, par_i); 
        }
    }

    cout << "start check cycle" << std::endl;
    // check cycle
    bool *vis = new bool[class_number + 5];
    bool *flag = new bool[class_number + 5];
    cout << "check cycle" << std::endl;
    for(int i = 0; i < class_number; i++) {
        for(int j = 0; j < class_number; j++) {
            vis[j] = false;
            flag[j] = false;
        }
        Symbol name = all_classes->nth(i)->getname();
        int *id = name2id->lookup(name);
        if(dfs_check(*id, vis, flag) == true) {
            cout << "false check" << std::endl;
            semant_error() << "Class " << name << ", or an ancestor of " << name << ", is involved in an inheritance cycle." << std::endl;
        }
        // cout << std::endl;
    }

    cout << "finish build" << std::endl;

    
}

bool ClassTable::dfs_check(int id, bool *vis, bool *flag) {
    if(vis[id] == true) return false;
    vis[id] = true;
    flag[id] = true;
    int *par_id = inheritance -> lookup(id);
    if(par_id != NULL) {
        // cout << "dfs par id: " << *par_id << std::endl;
        if(flag[*par_id] == true)
            return true;
        if(dfs_check(*par_id, vis, flag))
            return true;
    }
    flag[id] = false;
    return false;
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);
    
    all_classes = append_Classes(all_classes, single_Classes(Object_class));

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  

        all_classes = append_Classes(all_classes, single_Classes(IO_class));

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    all_classes = append_Classes(all_classes, single_Classes(Int_class));

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    all_classes = append_Classes(all_classes, single_Classes(Bool_class));

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);

    all_classes = append_Classes(all_classes, single_Classes(Str_class));
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 

bool attr_class::attr_redefine(SymbolTable<Symbol, Entry> *curr_attr) {
    return (curr_attr->lookup(name) != NULL);
}

int attr_class::method_redefine(SymbolTable<Symbol, Feature_class> *curr_method, Symbol filename, ClassTable *classtable) {
    return 0;
}

bool method_class::attr_redefine(SymbolTable<Symbol, Entry> *curr_attr) {
    return false;
}

int method_class::method_redefine(SymbolTable<Symbol, Feature_class> *curr_method, Symbol filename, ClassTable *classtable) {
    // COOL Manual CH6: redefinition of inherited methods
    if(curr_method->probe(name) != NULL) {
        //redefine in the same class
        return 1;
    }
    // formal type error
    Formal curr_formal;
    for(int i = formals->first(); formals->more(i); i = formals->next(i)) {
        curr_formal = formals->nth(i);
        if(curr_formal->getname() == self)
            return 2;
        if(curr_formal->gettype() == SELF_TYPE)
            return 3;
        if(classtable->check_typerror(filename, curr_formal->gettype(), curr_formal) == false)
            return 4;
    }
    Feature pre_feature = curr_method->lookup(name);
    if(pre_feature == NULL)
        return 0;
    if(pre_feature->gettype() != return_type)
        return 5;
    Formals pformals = pre_feature->getformals();
    int i;
    for(i = formals->first(); formals->more(i); i = formals->next(i)) {
        if(!pformals->more(i))
            return 6;
        if(formals->nth(i)->gettype() != pformals->nth(i)->gettype())
            return 7;
    }
    if(pformals->more(i))
        return 6;
    return 0;
}

bool ClassTable::check_typerror(Symbol filename, Symbol curr_type, tree_node *t) {
    if(curr_type != SELF_TYPE && curr_type != prim_slot && name2class->lookup(curr_type) == NULL) {
        semant_error(filename, t) << "Type " << curr_type << "is not define" << std::endl;
        return false;
    }
    return true;
}

void method_class::add_method(SymbolTable<Symbol, Feature_class> *curr_method) {
    curr_method->addid(name, this);
}

void method_class::add_attr(SymbolTable<Symbol, Entry> *curr_method) {
    return;
}

void attr_class::add_method(SymbolTable<Symbol, Feature_class> *curr_attr) {
    return;
}

void attr_class::add_attr(SymbolTable<Symbol, Entry> *curr_attr) {
    curr_attr->addid(name, type_decl);
}

bool attr_class::isself() { return name == self; }
bool method_class::isself() { return false; }

void ClassTable::add_all_features(Class_ curr_class, SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, Feature_class> *curr_method) {
    Features features = curr_class->getfeatures();
    Feature curr_feature;
    for(int i = features->first(); features->more(i); i = features->next(i)) {
        curr_feature = features->nth(i);
        if(curr_feature->isself()) {
            semant_error(curr_class->get_filename(), curr_feature) << "self is not allowed to appear in an attribute binding.\n";
        }
        Symbol curr_type = curr_feature->gettype();
        if(check_typerror(curr_class->get_filename(), curr_type, curr_feature)) {
            // add attr & method
            Symbol feature_name = curr_feature->getname();
            if(curr_feature->attr_redefine(curr_attr) == true) {
                semant_error(curr_class->get_filename(), curr_feature) << "Attribute " << feature_name << " redefine" << std::endl;
            }
            else {
                curr_feature->add_attr(curr_attr);
            }
            int ret = curr_feature->method_redefine(curr_method, curr_class->get_filename(), this);
            if(ret == 0) {
                curr_feature->add_method(curr_method);
            }
            else {
                // handle error
            }
        }
    }
}

void ClassTable::rec_add_features(Class_ curr_class, SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, Feature_class> *curr_method) {
    Symbol par_name = curr_class->getparent();
    cout << curr_class->getname() << " rec add features " << par_name << std::endl;
    Class_ par_class = name2class->lookup(par_name);
    if(curr_class->getname() != Object) {
        rec_add_features(par_class, curr_attr, curr_method);
    }
    add_all_features(curr_class, curr_attr, curr_method);
}

void ClassTable::scoping_build() {
    cout << "start coping build" << std::endl;
    attr_table = new SymbolTable<Symbol, SymbolTable<Symbol, Entry> >();
    attr_table -> enterscope();
    method_table = new SymbolTable<Symbol, SymbolTable<Symbol, Feature_class> >();
    method_table -> enterscope();
    SymbolTable<Symbol, Entry> *curr_attr;
    SymbolTable<Symbol, Feature_class> *curr_method;
    for(int i = all_classes->first(); all_classes->more(i); i = all_classes->next(i)) {
        Class_ curr_class = all_classes->nth(i);
        Symbol name = curr_class->getname();
        cout << name << endl;
        curr_attr = new SymbolTable<Symbol, Entry>();
        curr_method = new SymbolTable<Symbol, Feature_class>();
        curr_attr->enterscope();
        curr_method->enterscope();
        if(name != Object) {
            Symbol par_name = curr_class->getparent();
            Class_ par_class = name2class->lookup(par_name);
            rec_add_features(par_class, curr_attr, curr_method);
        }
        cout << name << " add ancestor " << std::endl;
        curr_attr->enterscope();
        curr_method->enterscope();
        add_all_features(curr_class, curr_attr, curr_method);
        curr_attr->addid(self, name); // add self attribute
        if(name == Main) {
            if(curr_method->probe(main_meth) == NULL) {
                semant_error(curr_class->get_filename(), curr_class) << "No 'main' method in class Main.\n";
            }
            else if (curr_method->probe(main_meth)->getformals()->len() != 0)
                semant_error(curr_class->get_filename(), curr_class) << "Method main should take no formal parameters.\n";
        }
        attr_table->addid(name, curr_attr);
        method_table->addid(name, curr_method);
    }
}

void ClassTable::type_checking() {
    cout << "ClassTable type checking" << std::endl;
    for(int i = all_classes->first(); all_classes->more(i); i = all_classes->next(i)) {
        Class_ curr_class = all_classes->nth(i);
        Symbol name = curr_class->getname();
        if(name == Object || name == Bool || name == IO || name == Int || name == Str) continue;
        cout << "type checking: Class " << name << std::endl;
        curr_class->type_checking(attr_table->lookup(name), method_table->lookup(name), this);
        cout << std::endl << std::endl;
    }
}

Class_ ClassTable::lookup(Symbol name) {
    return name2class->lookup(name);
}

void class__class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, 
                                SymbolTable<Symbol, Feature_class> *curr_method, ClassTable* classtable) {
    cout << "class__class type checking" << std::endl;
    Features curr_features = features;
    Class_ class_i = this;
    Symbol par_name;
    do {
        curr_features = class_i->getfeatures();
        for(int i = curr_features->first(); curr_features->more(i); i = curr_features->next(i)) {
            if(class_i == this || curr_method->probe(curr_features->nth(i)->getname()) == NULL)
                curr_features->nth(i)->type_checking(curr_attr, curr_method, classtable, this);
        }
        par_name = class_i->getparent();
        class_i = classtable->lookup(par_name);
    }while(par_name != No_class);
}

bool ClassTable::check_inheritance(Symbol s1, Symbol s2) {
    cout << "check inheritance " << s1 << "  " << s2 << std::endl;
    if(s1 == s2 || s1 == No_type) return true;
    Class_ curr_class = name2class->lookup(s1);
    if(curr_class == NULL) return false;
    return check_inheritance(curr_class->getparent(), s2);
}

Symbol ClassTable::lca(Symbol s1, Symbol s2) {
    if(check_inheritance(s1, s2)) return s2;
    if(check_inheritance(s2, s1)) return s1;
    while(!check_inheritance(s2, s1)) {
        Class_ curr_class = name2class->lookup(s1);
        s1 = curr_class->getparent();
    }
    return s1;
}

void attr_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: Attribute " << name << std::endl;
    init = init->type_checking(curr_attr, curr_method, classtable, curr_class);
    Symbol init_type = init->get_type();
    if(init_type != No_type) {
        Symbol rtype = type_decl;
        if(rtype == SELF_TYPE)
            rtype = curr_attr->lookup(self);
        if(init_type == SELF_TYPE)
            init_type = curr_attr->lookup(self);
        if(classtable->check_inheritance(init_type, rtype) == false) {
            classtable->semant_error(curr_class->get_filename(), this) 
                << "Identifier " << name << " declared type " << rtype << " but assigned type " << init_type << std::endl;
        }
    }
}

void method_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: Method " << name << std::endl;
    curr_attr->enterscope();
    Formal curr_formal;
    for(int i = formals->first(); formals->more(i); i = formals->next(i)) {
        curr_formal = formals->nth(i);
        curr_attr->addid(curr_formal->getname(), curr_formal->gettype());
    }
    expr = expr->type_checking(curr_attr, curr_method, classtable, curr_class);
    Symbol expr_type = expr->get_type();
    Symbol rtype = return_type;
    if(return_type == SELF_TYPE) {
        rtype = curr_attr->lookup(self);
    }
    if(expr_type == SELF_TYPE) {
        cout << "expr type is self type" << std::endl;
        expr_type = curr_attr->lookup(self);
    }
    cout << "check inheritance : " << expr_type << " " << rtype << std::endl;
    if(!classtable->check_inheritance(expr_type, rtype)) {
        classtable->semant_error(curr_class->get_filename(), this) << "The declared return type of method " << name 
            << " is " << rtype << " but the type of the method body is " << expr_type << std::endl;
    }
    curr_attr->exitscope();
}

Expression assign_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: assign " << std::endl;
    expr = expr->type_checking(curr_attr, curr_method, classtable, curr_class);
    Symbol expr_type = expr->get_type();
    Symbol rtype = curr_attr->lookup(name);
    if(classtable->check_inheritance(expr_type, rtype)) {
        return set_type(expr_type);
    }
    else {
        // error handler see PA4 manual
        classtable->semant_error(curr_class->get_filename(), this) << "Identifier " << name << 
            " declared type " << rtype << " but assigned type " << expr_type << std::endl;
        return set_type(Object);
    }
}

Feature ClassTable::get_def_method(Symbol class_name, Symbol name) {
    return method_table->lookup(class_name)->lookup(name);
}

Expression static_dispatch_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: static dispatch " << std::endl;
    if(!classtable->check_typerror(curr_class->get_filename(), type_name, this)) {
        return set_type(Object);
    }
    expr = expr->type_checking(curr_attr, curr_method, classtable, curr_class);
    Symbol expr_type = expr->get_type();
    if(expr_type == SELF_TYPE)
        expr_type = curr_attr->lookup(self);
    if(classtable->check_inheritance(expr_type, type_name)) {
        // definition of static dispatch <expr>@<type>.id(<expr>,...,<expr>) to 
        // call methods of parent classes that have been hidden by redefinitions in child classes
        classtable->semant_error(curr_class->get_filename(), this) 
            << "The caller is not of type " << type_name << std::endl;
    }
    Feature def_method = classtable->get_def_method(expr_type, name);
    if(def_method == NULL) {
        classtable->semant_error(curr_class->get_filename(), this) << "The caller is not of type " << type_name << std::endl;
    }
    Expression typed_expr;
    Expressions new_actual;
    int i;
    for(i = actual->first(); actual->more(i); i = actual->next(i)) {
        if(!def_method->getformals()->more(i)) {
            classtable->semant_error(curr_class->get_filename(), this) << 
                "Number of parameters is not the same of declared" << std::endl;
            return set_type(Object);
        }
        typed_expr = actual->nth(i)->type_checking(curr_attr, curr_method, classtable, curr_class);
        if(classtable->check_inheritance(typed_expr->get_type(), def_method->getformals()->nth(i)->gettype()) == false) {
            classtable->semant_error(curr_class->get_filename(), this) << "Parameters are not of the types declared." << std::endl;
            typed_expr = typed_expr->set_type(Object);
        }
        if(i == actual->first())
            new_actual = single_Expressions(typed_expr);
        else new_actual = append_Expressions(new_actual, single_Expressions(typed_expr));
    }
    if(def_method->getformals()->more(i)) {
        classtable->semant_error(curr_class->get_filename(), this) << 
            "Number of parameters is not the same of declared" << std::endl;
        return set_type(Object);
    }
    actual = new_actual;
    Symbol rtype = def_method->gettype();
    if(rtype == SELF_TYPE)
        rtype = expr_type;
    return set_type(rtype);
}

Expression dispatch_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: dispatch " << curr_class->getname() << " " << name << std::endl;
    expr = expr->type_checking(curr_attr, curr_method, classtable, curr_class);
    Symbol expr_type = expr->get_type();
    if(expr_type == SELF_TYPE)
        expr_type = curr_attr->lookup(self);
    Feature def_method = classtable->get_def_method(expr_type, name);
    if(def_method == NULL) {
        classtable->semant_error(curr_class->get_filename(), this) << "The caller is not of type " << name << std::endl;
    }
    Expression typed_expr;
    Expressions new_actual;
    int i;
    for(i = actual->first(); actual->more(i); i = actual->next(i)) {
        if(!def_method->getformals()->more(i)) {
            classtable->semant_error(curr_class->get_filename(), this) << 
                "Number of parameters is not the same of declared" << std::endl;
            return set_type(Object);
        }
        typed_expr = actual->nth(i)->type_checking(curr_attr, curr_method, classtable, curr_class);
        if(classtable->check_inheritance(typed_expr->get_type(), def_method->getformals()->nth(i)->gettype()) == false) {
            classtable->semant_error(curr_class->get_filename(), this) << "Parameters are not of the types declared." << std::endl;
            typed_expr = typed_expr->set_type(Object);
        }
        if(i == actual->first())
            new_actual = single_Expressions(typed_expr);
        else new_actual = append_Expressions(new_actual, single_Expressions(typed_expr));
    }
    if(def_method->getformals()->more(i)) {
        classtable->semant_error(curr_class->get_filename(), this) << 
            "Number of parameters is not the same of declared" << std::endl;
        return set_type(Object);
    }
    actual = new_actual;
    Symbol rtype = def_method->gettype();
    if(rtype == SELF_TYPE)
        rtype = expr_type;
    return set_type(rtype);
}

Expression cond_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: condition " << std::endl;
    pred = pred->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(pred->get_type() != Bool) {
        classtable->semant_error(curr_class->get_filename(), this) << "Predicate is not of type Bool." << std::endl;
        return set_type(Object);
    }
    then_exp = then_exp->type_checking(curr_attr, curr_method, classtable, curr_class);
    else_exp = else_exp->type_checking(curr_attr, curr_method, classtable, curr_class);
    return set_type(classtable->lca(then_exp->get_type(), else_exp->get_type()));
}

Expression loop_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: loop " << std::endl;
    pred = pred->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(pred->get_type() != Bool) {
        classtable->semant_error(curr_class->get_filename(), this) << "Predicate is not of type Bool." << std::endl;
        return set_type(Object);
    }
    body = body->type_checking(curr_attr, curr_method, classtable, curr_class);
    return set_type(Object);
}

Expression typcase_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: case " << std::endl;
    Expression typed_expr;
    expr = expr->type_checking(curr_attr, curr_method, classtable, curr_class);
    Symbol curr_type;
    Cases new_case;
    Symbol rtype;
    List<Entry> *type_list = NULL;
    for(int i = cases->first(); cases->more(i); i = cases->next(i)) {
        Case curr_case = cases->nth(i);
        curr_type = curr_case->gettype();
        // check duplicate case type 
        for(List<Entry> *l = type_list; l; l = l->tl()) {
            if(l->hd() == curr_type) {
                classtable->semant_error(curr_class->get_filename(), this) << 
                                "Type " << curr_type << " appears multiple times." << std::endl;
                return set_type(Object);
            }
        }
        type_list = new List<Entry>(curr_type, type_list);
        if(!classtable->check_typerror(curr_class->get_filename(), curr_type, curr_case)) {
            typed_expr = curr_case->getexpr()->set_type(Object);
        }
        else {
            curr_attr->enterscope();
            curr_attr->addid(curr_case->getname(), curr_type);
            typed_expr = curr_case->getexpr()->type_checking(curr_attr, curr_method, classtable, curr_class);
            curr_attr->exitscope();
        }
        if(i == cases->first()) {
            rtype = typed_expr->get_type();
            new_case = single_Cases(branch(curr_case->getname(), curr_type, typed_expr));
        }
        else {
            rtype = classtable->lca(rtype, typed_expr->get_type());
            new_case = append_Cases(new_case, single_Cases(branch(curr_case->getname(), curr_type, typed_expr)));
        }
    }
    cases = new_case;
    return set_type(rtype);
}

Expression block_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: block " << std::endl;
    Expressions new_body;
    Expression typed_expr;
    for(int i = body->first(); body->more(i); i = body->next(i)) {
        typed_expr = body->nth(i)->type_checking(curr_attr, curr_method, classtable, curr_class);
        if(i == body->first())
            new_body = single_Expressions(typed_expr);
        else {
            new_body = append_Expressions(new_body, single_Expressions(typed_expr));
        }
        if(!body->more(body->next(i)))
            return set_type(typed_expr->get_type());
    }
}

Expression plus_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: plus " << std::endl;
    e1 = e1->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e1->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Left part of + is not of type Int. " << std::endl;
        return set_type(Object);
    }
    e2 = e2->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e2->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Right part of + is not of type Int. " << std::endl;
        return set_type(Object);
    }
    return set_type(Int);
}

Expression sub_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: sub " << std::endl;
    e1 = e1->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e1->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Left part of - is not of type Int. " << std::endl;
        return set_type(Object);
    }
    e2 = e2->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e2->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Right part of - is not of type Int. " << std::endl;
        return set_type(Object);
    }
    return set_type(Int);
}

Expression mul_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: mul " << std::endl;
    e1 = e1->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e1->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Left part of * is not of type Int. " << std::endl;
        return set_type(Object);
    }
    e2 = e2->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e2->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Right part of * is not of type Int. " << std::endl;
        return set_type(Object);
    }
    return set_type(Int);
}

Expression divide_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: divide " << std::endl;
    e1 = e1->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e1->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Left part of / is not of type Int. " << std::endl;
        return set_type(Object);
    }
    e2 = e2->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e2->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Right part of / is not of type Int. " << std::endl;
        return set_type(Object);
    }
    return set_type(Int);
}

Expression neg_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: neg " << std::endl;
    e1 = e1->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e1->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Left part of ~ is not of type Int. " << std::endl;
        return set_type(Object);
    }
    return set_type(Int);
}

Expression lt_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: lt " << std::endl;
    e1 = e1->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e1->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Left part of < is not of type Int. " << std::endl;
        return set_type(Object);
    }
    e2 = e2->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e2->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Right part of < is not of type Int. " << std::endl;
        return set_type(Object);
    }
    return set_type(Bool);
}

Expression eq_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: eq " << std::endl;
    e1 = e1->type_checking(curr_attr, curr_method, classtable, curr_class);
    e2 = e2->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e1->get_type() == Int) {
        if(e2->get_type() != Int) {
            classtable->semant_error(curr_class->get_filename(), this) << "Right part of = is not of type Int. " << std::endl;
            return set_type(Object);
        }
        else
            return set_type(Bool);
    }
    else if(e1->get_type() == Bool) {
        if(e2->get_type() != Bool) {
            classtable->semant_error(curr_class->get_filename(), this) << "Right part of = is not of type Bool. " << std::endl;
            return set_type(Object);
        }
        else
            return set_type(Bool);
    }
    else if(e1->get_type() == Str) {
        if(e2->get_type() != Str) {
            classtable->semant_error(curr_class->get_filename(), this) << "Right part of = is not of type Str. " << std::endl;
            return set_type(Object);
        }
        else
            return set_type(Bool);
    }
    else {
            classtable->semant_error(curr_class->get_filename(), this) << "Left part of = is not of type Int or Bool or Str. " << std::endl;
            return set_type(Object);
    }
}

Expression leq_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: leq " << std::endl;
    e1 = e1->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e1->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Left part of <= is not of type Int. " << std::endl;
        return set_type(Object);
    }
    e2 = e2->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e2->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Right part of <= is not of type Int. " << std::endl;
        return set_type(Object);
    }
    return set_type(Bool);
}

Expression comp_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: comp " << std::endl;
    e1 = e1->type_checking(curr_attr, curr_method, classtable, curr_class);
    if(e1->get_type() != Int) {
        classtable->semant_error(curr_class->get_filename(), this) << "Left part of \"not\" is not of type Int. " << std::endl;
        return set_type(Object);
    }
    return set_type(Bool);
}

Expression int_const_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: int const " << std::endl;
    return set_type(Int);
}

Expression bool_const_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: bool const " << std::endl;
    return set_type(Bool);
}

Expression string_const_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: string const " << std::endl;
    return set_type(Str);
}

Expression new__class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: new " << std::endl;
    if(!classtable->check_typerror(curr_class->get_filename(), type_name, this))
        return set_type(Object);
    else
        return set_type(type_name);
}

Expression isvoid_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: is void " << std::endl;
    return set_type(Bool);
}

Expression no_expr_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: no expr " << std::endl;
    return set_type(No_type);
}

Expression object_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: object " << name << std::endl;
    if(name == self) return set_type(SELF_TYPE);
    return set_type(curr_attr->lookup(name));
}

Expression let_class::type_checking(SymbolTable<Symbol, Entry> *curr_attr, SymbolTable<Symbol, 
                               Feature_class> *curr_method, ClassTable* classtable,
                               Class_ curr_class) {
    cout << "type checking: let " << std::endl;
    if(identifier == self) {
        classtable->semant_error(curr_class->get_filename(), this) << 
                "self is not allowed to appear in a let binding." << std::endl;
        return set_type(Object);
    }
    init = init->type_checking(curr_attr, curr_method, classtable, curr_class);
    Symbol init_type = init->get_type();
    if(init_type != No_type) { 
        Symbol rtype = type_decl;
        if(type_decl == SELF_TYPE)
            rtype =  curr_attr->lookup(self);
        if(init_type == SELF_TYPE)
            init_type = curr_attr->lookup(self);
        if(!classtable->check_inheritance(init_type, rtype)) {
            classtable->semant_error(curr_class->get_filename(), this) << "Identifier " << identifier << 
                " declared type " << rtype << " but assigned type " << init_type << std::endl;
        }
    }
    else init = init->set_type(type_decl);
    curr_attr->enterscope();
    curr_attr->addid(identifier, type_decl);
    body = body->type_checking(curr_attr, curr_method, classtable, curr_class);
    curr_attr->exitscope();
    return set_type(body->get_type());
}


/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();
    cout << "OK" << std::endl;

    /* ClassTable constructor may do some semantic analysis */
    ClassTable *classtable = new ClassTable(classes);


    /* some semantic analysis code may go here */

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }

    classtable->scoping_build();
    classtable->type_checking();

    if (classtable->errors()) {
	cerr << "Compilation halted due to static semantic errors." << endl;
	exit(1);
    }
}


