#include "support.h"

string toString(int num) {
	char buffer[33];
	sprintf(buffer, "%d", num);
	return string(buffer);
}

string getRoot(string name) {
	int i = 0;
	char c = ' ';
	string root = "";
	c = name.at(0);
	while (i < name.length() && (c = name.at(i)) && (c != '_')) {
		root = root + string(1, c);
		i++;
	}
	return root;
}

string getSecRoot(string name) {
	int i = 0;
	char c = ' ';
	string root = "";
	c = name.at(0);
	while (i < name.length() && (c = name.at(i)) && (c != '_'))
		i++;

	i++;
	while (i < name.length() && (c = name.at(i)) && (c != '_')) {
		root = root + string(1, c);
		i++;
	}
	return root;
}
