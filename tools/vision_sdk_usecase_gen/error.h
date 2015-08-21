#ifndef ERROR_H
#define ERROR_H

#include <sstream>
#include <string>
#include <cstdlib>
using namespace std;

extern bool debug;
#define SHOULD_NOT_REACH false

#define CHECK_ERROR(x,y) \
     {  stringstream ___new___string___, dstr; \
        ___new___string___ << y; \
        dstr << " (Condition at " << __LINE__ <<  ", file " << __FILE__ << ")"; \
        check_condition(x, ___new___string___.str(), dstr.str()); \
     }

#define CHECK_ERROR_ABORT(x,y) \
     {  stringstream ___new___string___, dstr; \
     	 ___new___string___ << y; \
     	 dstr << " (Condition at " << __LINE__ <<  ", file " << __FILE__ << ")"; \
        check_condition_abort(x, ___new___string___.str(), dstr.str()); \
     }

static void check_condition(bool condition, string error_message, string debugstr) {
	if (!condition) {
		cout << error_message;
		if(debug)
			cout<<debugstr;
		cout<<endl;
	}
}

static void check_condition_abort(bool condition, string error_message, string debugstr) {
	if (!condition) {
		cout << error_message;
		if(debug)
			cout<<debugstr;
		cout<<endl;
		exit(0);
	}
}


#endif
