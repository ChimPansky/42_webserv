# Codestyle and workflow

1) 1 feature 1 merge request, big mr won't be merged
2) commit history should be clear and correspond to the merge request. see git_convention.md
3) mr should not break pipline. eg: should compile and run
4) mr should follow codestyle. try follow [google codestyle](https://google.github.io/styleguide/cppguide.html), main features of which are below


# Codestyle main points

1) CamelCase for typenames (classes, typedefs, structs, enums, so on)
2) snake_case for all the rest of the code
3) indent with 4 spaces (never use tabs) (namespace does not increase indent)
4) name private members of class with underscore
5) no very long lines (aim 100 soft limit, 120 hard limit)
6) no very big functions (50 lines soft limit, no hard limit)
7) include what u use only, sort alphabetically.
8) ifndef guard with full path instead of pragmas: src/webserver/cluster/Cluster.h #ifndef WS_WEBSERVER_CLUSTER_CLUSTER_H ...
9) .cpp for source code, .h for headers

1) if class does not have private members it is a struct
2) class either should have a state or be an interface. otherwise use free functions in a namespace
3) anonimous namespace instead of static functions
4) as least exceptions as possible on init step, no exceptions in the main loop
5) every resource (memory, socket, file descriptor, connection) should be obtained in constructor, free in destructor. (RAII idiom)
6) Rule of 3: if either of destructor, copy-constructor, copy assygnment op is defined defyne all 3. otherwise define none
7) Variables declared right before use, no empty declarations, outer namespace
8) avoid #defines except include guards, prefer static const class variables
9) c casts are forbidden
