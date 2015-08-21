#include <iostream>
#include <fstream>

using namespace std;

#include "vsdk-ctx.h"
#include "link.h"
#include "parse.tab.hh"
#include "options.h"
#include "usecase.h"
#include "error.h"

Usecase mainObj;
Processor proc;

extern Options cmd_options;
bool debug = false;
extern void init_Scanner();

vsdk_ctx::vsdk_ctx() {
	init_Scanner();
}

int main(int argc, char * argv[]) {

	int v;

	//process the command line option
	v = cmd_options.process_Options(argc, argv);
	if(v == -1)
		return 0;

	// Create the parser object and parse the input program
	vsdk_ctx ctx;
	yy::vsdk parser(ctx);
	v = parser.parse();
	CHECK_ERROR_ABORT(v == 0, "Error: Cannot parse the input program !!!");

	//initialize, print summary of links and generate files
	mainObj.initialize();
	mainObj.print();
	mainObj.genFiles();

	return 0;
}

namespace yy {
void vsdk::error(location const &loc, const std::string& s) {
	cerr << "Error: @ [" << loc << "] : [" << s << "] !!!\n";
}
}
