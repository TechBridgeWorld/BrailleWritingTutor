
/* Madeleine Clute July 2013
 * this file is meant to be able to handle multicell inputs with
 * the existing file parser. In order to add a multi cell character, 
 * add it to the list with 


 write isMultiCell macro which sees how far the bits have gone */

#ifndef MULTICHARACTER_H_
#define MULTICHARACTER_H_

#define MAX_CELLS 4
#include <stdint.h>
#include <string>
#include <iostream>

using namespace std; 

typedef struct multiCellChar {
	std::string c;
	int num_cells;
	int dot_patterns [MAX_CELLS];
} multiCellChar; 

typedef struct node {
	node *prev;
	node *next;
	multiCellChar *content;
} node;

class multi {

public:
 void initializeMultiCell();
 bool isMultiCell(const std::string);
 int numCells (const std::string);
 int *getPatterns(const std::string);

 //void initializeMultiCell(); // called to initialize the lis 
private:
	multiCellChar *retrieveChar(std::string);
	node test1, test2;
};

#endif /* MULTICHARACTER_H_ */