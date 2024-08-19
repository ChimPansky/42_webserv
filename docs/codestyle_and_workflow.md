# Codestyle and workflow

1) 1 feature 1 merge request, big mr won't be merged
2) commit history should be clear and correspond to the merge request. see git_convention.md
3) mr should not break pipline. eg: should compile and run
4) mr should follow codestyle. try follow [google codestyle](https://google.github.io/styleguide/cppguide.html), main features of which are below


# Codestyle main points

1) PascalCase: typenames (classes, typedefs, structs, enums, so on), Function names (including class methods, except getters/setters)
2) snake_case: (var names, param names `int lines_cnt = 0`)
3) UPPER_CASE: defines and macros (`#define WS_WEBSERVER_CLUSTER_CLUSTER_H`, `MAX_INT`)
4) camelCase: constants (`static const int kBufSize = 666`)
5) exceptions allowed when mke sense (follow c style or existing library style)
6) indent with 4 spaces (never use tabs) (namespace does not increase indent) (change vs code settings if necessary: insert spaces(4) for tabs...)
7) name private members of class with underscore (e.g.: _name, _type,...)
8) no very long lines (aim 100 soft limit, 120 hard limit)
9) no very big functions (50 lines soft limit, no hard limit)
10) include what u use only, sort alphabetically.
11) ifndef guard with full path instead of pragmas: src/webserver/cluster/Cluster.h #ifndef WS_WEBSERVER_CLUSTER_CLUSTER_H ...
12) .cpp for source code, .h for headers
13) getter for private var _c is c(), setter is set_c()
14) if and loops always provide scope (even 1-liners), curly brace on same line:
```
if (a) {
	...
} else if (b) {
   ...
} else {
   ...
}
```

1) if class does not have private members it is a struct
2) class either should have a state or be an interface. otherwise use free functions in a namespace
3) anonimous namespace instead of static functions (functions that are only used in current implementation file)
4) as least exceptions as possible. if needed then exceptions only on initialization step, not afterwards
5) every resource (memory, socket, file descriptor, connection) should be obtained in constructor, freed/released in destructor. (RAII idiom: Ressource Acquisition Is Initialization)
6) Rule of 3: if either of destructor, copy-constructor, copy assignment operator is defined define all 3. otherwise define none
7) Variables declared right before use, no empty declarations, (if you have a variable that exists only inside of a loop, it should be declared in the loop...)
8) avoid #defines except include guards, prefer static const class variables
9) c-style casts are forbidden (e.g. `int a = (int)x` or `int a = int(x)`)  
10) native types (int, double, char, pointer,...) go by value, complex types by const ref or ref. const when possible
11) prefer refs to pointers

```
class A {
  public:  // 2 spaces for access modifires instead of 4
    A();
    const std::string& c() const;
    void set_c(const std::string& c);
  private:
    int _a;
    bool _b;
    std::string _c;
}

A::A(int a, bool b, const std::string& c)
  : _a(a),  // 2 spaces initialization list sign
    _b(b),  // 4 spaces
    _c(c)
{
    // do smth, 4 spaces
}

const std::string& A::c() const
{

}

void A::set_c(const std::string& c)
{

}
```
