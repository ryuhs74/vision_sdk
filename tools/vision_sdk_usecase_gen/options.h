#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>
#include <cstdio>
#include <fstream>
#include <ostream>
#include "error.h"

extern bool debug;
using namespace std;

class Options {
	bool show_tokens;
	bool write_file;
	bool write_image;
	bool verbose_mode;
	bool log_mode;
	bool output_path;
	/* Output streams */

	ostream* os_tokens;
	ostream* os_file;
	ostream* os_header;
	ostream* os_image;
	ostream* os_log;

	/* 
	 Privately, we also need objects of ofstream which can be opened
	 if required and their addresses set to the pointers above.
	 */

	ofstream ofs_tokens;
	ofstream ofs_file;
	ofstream ofs_header;
	ofstream ofs_image;
	ofstream ofs_log;

	string input_file_name;

public:
	string output_path_name;

	Options();
	~Options();

	bool show_Tokens();
	bool write_toFile();
	bool write_toImage();
	bool verboseMode();
	bool write_tologFile();

	int process_Options(int argc, char * argv[]);
	void setFileNames(string filestr);
	void close_Image();

	//Return output file with appropriate suffix attached, if no input file then cout
	ostream* tokens_File();
	ostream* write_File();
	ostream* write_Header();
	ostream* write_Image();
	ostream* log_File();

	string get_output_path_name();
};

extern Options cmd_options;
#endif
