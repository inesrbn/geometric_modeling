#include "myHalfedge.h"

myHalfedge::myHalfedge(void)
{
	source = NULL; 
	adjacent_face = NULL; 
	next = NULL;  
	prev = NULL;  
	twin = NULL;  
}

void myHalfedge::copy(myHalfedge* ie)
{
    if (!ie) return;

    source = ie->source;
    adjacent_face = ie->adjacent_face;
    next = ie->next;
    prev = ie->prev;
    twin = ie->twin;
}

myHalfedge::~myHalfedge(void)
{
}
