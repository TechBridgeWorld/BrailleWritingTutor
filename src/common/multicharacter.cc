
#include "multicharacter.h"
#include <assert.h>
#include <stdlib.h>
#include <string>


using namespace std;
 
/* Multicell character support
 * July 2013 -- Madeleine

 * README: This file is where multi-cell characters need to be added. 
 * As of now, they are hard coded in, as there are generally very few of them, 
 * and the timeline for iSTEP 2013 did not permit a deeper, more general solution. 
 * (ideally, you should be able to put in some delimeter in the mappings file
 * and parse that accordingly). To add a character, add it as you would noramlly
 * and then add it to this file, which will allow it to be flagged as multi-cell,
 * and will override whatever you've put in the mappings file. To add something
 * to this, first you need to create a multiCellChar struct like so:
 	static multiCellChar char2 = {"आ", 2, {0b000001, 0b111111}};
 * first you put the character in as string (as it might be more than one byte),
 * then how many cells it takes, and then finally, the bit mask of each cell in
 * an array. Then, you need to add it to the linked list by declaring a node for it
 * and hooking it up approriately in initializeMultiCell().
*/

 multiCellChar *retrieveChar(char);

static node *character_list;
static multiCellChar char1 = {"अ", 2, {0b100000, 0b001100}};
static multiCellChar char2 = {"आ", 2, {0b000001, 0b111111}};
static node test1, test2; 

void multi::initializeMultiCell(){
	printf("starting\n");
	
	//----------------------------------
	::test1.prev = NULL;
	::test1.next = &(::test2); // force it to look for the global
	::test1.content = &char1;
	
	//----------------------------------
	::test2.prev = &(::test1);
	::test2.next = NULL;
	::test2.content = &char2;

	//----------------------------------
	character_list = &(::test1);

}


multiCellChar *multi::retrieveChar( std::string target){
	node *ptr;
	for (ptr = character_list; ptr != NULL; ptr = ptr->next){
		assert(ptr != NULL);
		assert(ptr->content != NULL);
 		if ((ptr->content->c).compare(target) == 0) {
 			return ptr->content;
 		}
 	}
 	return NULL;
 }

bool multi::isMultiCell(const std::string target){
	if (retrieveChar(target) == NULL)
		return false;
	return true;
}

// assumes that it is multicell
int multi::numCells(const std::string target){
	multiCellChar *info = retrieveChar(target);
	assert(info != NULL);
	return info->num_cells;
}

int *multi::getPatterns(const std::string target){
	multiCellChar *info = retrieveChar(target);
	assert(info != NULL);
	return (info->dot_patterns);
}

