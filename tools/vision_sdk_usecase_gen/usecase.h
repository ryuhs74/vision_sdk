#ifndef USECASE_H
#define USECASE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "link.h"
#include "options.h"
#include "error.h"
#include "support.h"

#define OBJ_MAX 128 //Changeable TODO: put this in documentation

extern Processor proc;
extern Options cmd_options;
using namespace std;

class Usecase {

	string fileName;
	string structName;
	ostream* logFile;
	map<string, Link*> inst_object; //Map of link name and Link object
	vector<Link*> exec_seq;
	vector<Link*> temp_seq;
	bool matrix[OBJ_MAX][OBJ_MAX]; //Matrix of connections
	bool userMat[OBJ_MAX][OBJ_MAX]; //Matrix of connections according to user, i.e as mentioned in testcase
	int numIpc;	//Number of ipc Links generated, used to name ipc Link uniquely
	vector<vector<Link*> > connections;

	void createNewObj(string name, Link* &obj);

	//initialize Links
	void assignCPU();
	void assignLinkID();
	void assignSeq(int curr, bool* asgn, bool* visited, bool* done);
	void setSequence();
	void connect(Link* obj1, Link* obj2); //insert the object in map
	void createAllConn();

	//print Link summary
	void printFileName(ostream* out);
	void printMatrix(ostream* out);
	void printExecSeq(ostream* out);
	void printTable(ostream* out);

	//Generate files
	void genFile();
	void genImgFile();

	/**********************/
	void initFiles(ostream& fp, ostream& fph);
	void genIncludes(ostream& fp, ostream& fph);
	void genStruct(ostream& fp, ostream& fph);
	void genSetLinkID(ostream& fp, ostream& fph);
	void genResetLinkPrms(ostream& fp, ostream& fph);
	void genSetLinkPrms(ostream& fp, ostream& fph);
	void genConnectLinks(ostream& fp, ostream& fph);
	void genCreate(ostream& fp, ostream& fph);
	void genStart(ostream& fp, ostream& fph);
	void genStop(ostream& fp, ostream& fph);
	void genDelete(ostream& fp, ostream& fph);
	void genBufferStatistics(ostream& fp, ostream& fph);
	void genStatistics(ostream& fp, ostream& fph);
	void endFiles(ostream& fp, ostream& fph);

	/****************************/

public:

	Usecase();
	//~Usecase();

	void initialize();
	void print();
	void genFiles();

	void setFileName(string name);
	string getFileName();
	void setStructName(string name);
	string getStructName();

	Link* getObject(string name); //Return object from inst_object if present, else return NULL
	Link* createObject(string name); //Return object from inst_object if present, else create new Object

	void setNewConn(vector<Link*>* vec);
};
#endif
