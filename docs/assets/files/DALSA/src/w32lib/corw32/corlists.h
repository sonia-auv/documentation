/****************************************************************************** 
Copyright (c) 2003 Coreco inc / 2010 DALSA Corp.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer. 

  - Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the documentation 
    and/or other materials provided with the distribution. 

  - Neither the name of the DALSA nor the names of its contributors 
    may be used to endorse or promote products derived from this software 
    without specific prior written permission. 


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================

corlists.h			 											

Description:
   Generic list handling primitives

Platform:
	-All.

$Log: corlists.h $
Revision 1.2  2005/04/19 15:23:09  PARHUG
ListAppend now starts search from head (not head->next).
Revision 1.1  2004/08/19 12:26:05  parhug
Initial revision

*******************************************************************************/

#ifndef __CORLISTS_H
#define __CORLISTS_H

#include <corposix.h>

// Find an entry in the list by the address of the link pointer
// (return NULL if not found).
static inline LIST_ENTRY *ListFindEntry( LIST_ENTRY *head, LIST_ENTRY *entry)
{
	LIST_ENTRY *cur;
	off_t curVal, entryVal;

	if ( head != NULL)
	{
		cur = head;
		if ( cur->next != NULL)
		{
			// Set up the value to search for (pointer address).
			entryVal = (off_t) entry;

			// There are entries in the list. Find one.
			while ( cur->next != NULL)
			{
				cur = cur->next;
				curVal = (off_t)cur;
				if ( curVal == entryVal)
				{
					return cur;
				}
			}
		}
	}
	return NULL;
}

// Remove an entry (return pointer to removed entry).
static inline LIST_ENTRY *ListRemoveEntry( LIST_ENTRY *list)
{
	LIST_ENTRY *entry, *prev, *next;

	entry = NULL;
	if (list != NULL)
	{
		prev = list->prev;
		next = list->next;
		entry = list;
		if ( prev != NULL)
		{
			prev->next = next;
		}
		if ( next != NULL)
		{
			next->prev = prev;
		}
	}
	return entry;
}

// Insert between two entries.
static inline void ListInsertEntry( LIST_ENTRY *entry, LIST_ENTRY *prev, LIST_ENTRY *next)
{
	if (entry != NULL)
	{
		next->prev = entry;
		entry->next = next;
		entry->prev = prev;
		prev->next = entry;
	}
}

// Add to head end.
static inline void ListAdd( LIST_ENTRY *entry, LIST_ENTRY *head)
{
	if (entry != NULL)
	{
		LIST_ENTRY *next;

		next = head->next;
		head->next = entry;
		entry->next = next;
		if ( next != NULL)
		{
			entry->prev = next->prev;
			next->prev = entry;
		}
		else
		{
			entry->prev = head;
		}

	}
}

// Append to tail end.
static inline void ListAppend( LIST_ENTRY *entry, LIST_ENTRY *head)
{
	if (entry != NULL)
	{
		LIST_ENTRY *next;

		//next = head->next;
		next = head;
		while (next->next != NULL)
		{
			next = next->next;
		}
		entry->next = NULL;
		next->next = entry;
		entry->prev = next;
	}
}

#endif
