//! \file comm_routing.c
//! \addtogroup Communications
//! @{


#include "comm.h"
#include "serial.h"


//TODO: consider the situation where too many nodes are added to the network (may only be noticed by an ancestor node)!

//! \var uiNumEdges
//! \brief The number of edges in the edge list.  THis includes all descendants
uint uiNumEdges;
//! \struct S_nextEdge
//! \brief A structure pointer to indices in the edge list
S_Edge *S_nextEdge;
//! \struct S_edgeList[MAX_EDGES]
//! \brief The edge list
S_Edge S_edgeList[MAX_EDGES];
//! \var uiSelf
//! \brief Address of this node
uint uiSelf;


// Updates to the edge list are stored in the following structure.  All nodes joining the network are added to the
// update edge list, but since unjoining a node removes all descendants there is no need to add all dropped edges to
// the update edge list.
//! \struct S_Update
//! \brief Contains all the pertinent information to temporarily store network updates and delete them as needed.
struct {
		S_Edge  m_ucaEdges[MAX_UPDATES];		//!< Edge list containing IDs of added or dropped nodes
		uchar 	m_ucaFlags[MAX_UPDATES];
		uchar 	ucIndex;								//!< Update index
}S_RtUpdate;

void vRouteClrAllUpdates(void);

/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Clears the edge list and sets the next edge pointers to the
//!				 the edge list.
//!
//! \param uiAddress, the address of this node
//!	\return 0
////////////////////////////////////////////////////////////////////////////////
uchar ucRoute_Init(uint uiAddress)
{
	int i;

	//Save the address of this node
	uiSelf = uiAddress;

	//Clear values and edge list
	uiNumEdges = 0;
	for (i = 0; i < MAX_EDGES; i++)
		S_edgeList[i].m_uiSrc = S_edgeList[i].m_uiDest = 0;

	// Clear the arrays holding updates to the edge list
	vRouteClrAllUpdates();

	//Initialize non-zero values
	S_nextEdge = S_edgeList;

	return (0);
}

/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Adds addresses to the next available locations in the edge list
//!
//!
//! \param uiSrc, uiDest; source and destination node addresses
//! \return 0 for success, else error code
////////////////////////////////////////////////////////////////////////////////
uchar ucRoute_AddEdge(uint uiSrc, uint uiDest)
{
	if (uiNumEdges >= MAX_EDGES)
		return ROUTE_ERROR_TABLE_FULL;

	//Save the edge nodes and increment the pointer.  Also add the edges to the update list.
	S_nextEdge->m_uiSrc = S_RtUpdate.m_ucaEdges[S_RtUpdate.ucIndex].m_uiSrc = uiSrc;
	S_nextEdge->m_uiDest = S_RtUpdate.m_ucaEdges[S_RtUpdate.ucIndex].m_uiDest = uiDest;

	// Set the join flag in the update list
	S_RtUpdate.m_ucaFlags[S_RtUpdate.ucIndex] = F_JOIN;

	S_nextEdge++;					// Increment edge list pointer
	uiNumEdges++;					// Increment the total number of edges

	// Increment update index, prevent overflow
	if(S_RtUpdate.ucIndex<(MAX_UPDATES-1))
		S_RtUpdate.ucIndex++;

	return (0);
}

/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Searches through the edge list and removes the specified edge
//!
//!
//! \param
//! \return
////////////////////////////////////////////////////////////////////////////////
uchar ucRoute_RemoveEdge(uint uiSrc, uint uiDest)
{
	int i, j, uiFoundEdge;
	S_Edge *S_ptr;

	//Fail if there are no edges to remove
	if (uiNumEdges == 0)
		return ROUTE_ERROR_TABLE_EMPTY;

	// Assume we will not find the edge
	uiFoundEdge = 0;

	//Find the edge
	for (i = 0, S_ptr = S_edgeList; i < uiNumEdges; i++, S_ptr++)
	{
		if (S_ptr->m_uiSrc == uiSrc && S_ptr->m_uiDest == uiDest)
		{
			//Remove the edge from the list and shift the proceeding edges over to fill in the gap
			S_ptr->m_uiSrc = S_ptr->m_uiDest = 0;
			uiFoundEdge = 1;
			for (j = i + 1; j < uiNumEdges; j++)
			{
				//Bounds check the index before shifting
				if(j > 0 && j < uiNumEdges)
					S_edgeList[j - 1] = S_edgeList[j];
				else
					return ROUTE_ERROR_INVALID_EDGE;
			}
			uiNumEdges--;
			S_nextEdge--;
			S_edgeList[uiNumEdges].m_uiSrc = 0;
			S_edgeList[uiNumEdges].m_uiDest = 0;

			//We found the edge so exit the loop
			break;
		}
	}

	//Fail if the specified edge wasn't found in the graph
	if (uiFoundEdge == 0)
		return ROUTE_ERROR_DOES_NOT_EXIST;

	return (0);
}

/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Gets the next destination address for a node that may be several hops away
//!
//! In the event that the destination address is a child of self then the function returns
//! the self address.
//!
//! \param uiDest
//! \return uiSrc
////////////////////////////////////////////////////////////////////////////////
uint uiRoute_GetNextHop(uint uiDest)
{
	int i, j;
	S_Edge *S_ptr, *S_ptr2;
	uint uiSrc;

	//Search for edge with the matching destination (if it exists)
	for (i = 0, S_ptr = S_edgeList; i < uiNumEdges; i++, S_ptr++)
	{
		if (S_ptr->m_uiDest == uiDest)
		{
			// If the child is a direct child of this node then return the destination address
			if(S_ptr->m_uiSrc == uiSelf)
				return uiDest;

			//If the src is not the self then save the parent (source) of the edge
			uiSrc = S_ptr->m_uiSrc;
			break;
		}
	}

	//Fail if there isn't an edge with a matching destination
	if (i == uiNumEdges)
		return 0; //Error - 0 is an invalid address

//TODO: add in some logic to prevent infinite loops (circular graph connections) in the while loop below

	//Backtrack the src node address to one of the child nodes of this address
	while (1)
	{
		//Look through each edge
		for (j = 0, S_ptr2 = S_edgeList; j < uiNumEdges; j++, S_ptr2++)
		{
			//If the current edge leads to src then src is a child of this node (and therefore it is the next hop)
			if (S_ptr2->m_uiDest == uiSrc && S_ptr2->m_uiSrc == uiSelf)
			{
				return uiSrc;
			}

			//Otherwise continue backtracking from src
			else if (S_ptr2->m_uiDest == uiSrc)
			{
				uiSrc = S_ptr2->m_uiSrc;
				break;
			}
		}

		//If the backtracking failed then return fail
		if (j == uiNumEdges)
			break;
	}

	//Fail because the next hop wasn't found
	return 0; //Error - 0 is an invalid address
}

/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Adds a node to the list along with its edge list
//!
//!
//! \param
//! \return
////////////////////////////////////////////////////////////////////////////////
uchar ucRoute_NodeJoin(uint uiParent, uint uiChild, S_Edge* S_edges, int iNumEdges)
{
	int i;
	uchar ucReturnValue;
	S_Edge *S_ptr;

	//Add all of the edges in the subtree of this node
	for (i = 0, S_ptr = S_edges; i < iNumEdges; i++, S_ptr++)
	{
		ucReturnValue = ucRoute_AddEdge(S_ptr->m_uiSrc, S_ptr->m_uiDest);
		if (ucReturnValue != 0)
			return ucReturnValue;
	}

    //Add the new edge from the 'mount point' node to the new node
    if(uiParent == 0) return ucRoute_AddEdge(uiSelf, uiChild);      //The node is joining as a direct child
    else return ucRoute_AddEdge(uiParent, uiChild);               //The node is joining as a non-child descendant
}


/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Unjoin the specified node (and therefore all of its descendants) from the edge list
//!
//!
//! \param uiChild
//! \return 0
////////////////////////////////////////////////////////////////////////////////
uchar ucRoute_NodeUnjoin(uint uiChild)
{
	int i;
	S_Edge *S_ptr;
	uint uiTmp;
	uchar ucRet;

	//Allocate a remove stack on the system stack
	uint removeStack[MAX_NODES];
	int iStackSize = 0;

	//Fail if there are no edges to remove
	if (uiNumEdges == 0)
		return ROUTE_ERROR_TABLE_EMPTY;

	//Find the node that is being unjoined and then:
	//   1 - remove the edge that connects the unjoining node
	//   2 - add the node to the remove stack so that its descendants will also be removed below
	for (i = 0, S_ptr = S_edgeList; i < uiNumEdges; i++, S_ptr++)
	{
		//Remove the edge that connects the unjoining node (return on any error)
		if (S_ptr->m_uiDest == uiChild)
		{
			// Add the unjoining node to the edge list and set the drop flag
			S_RtUpdate.m_ucaEdges[S_RtUpdate.ucIndex].m_uiSrc = S_ptr->m_uiSrc;
			S_RtUpdate.m_ucaEdges[S_RtUpdate.ucIndex].m_uiDest = S_ptr->m_uiDest;
			S_RtUpdate.m_ucaFlags[S_RtUpdate.ucIndex] = F_DROP;
			if(S_RtUpdate.ucIndex<(MAX_UPDATES-1))
				S_RtUpdate.ucIndex++;

			ucRet = ucRoute_RemoveEdge(S_ptr->m_uiSrc, S_ptr->m_uiDest);
			if(ucRet)
				return ucRet;

			//Add the unjoining node to the remove stack and exit the loop
			removeStack[iStackSize++] = uiChild;
			break;
		}
	}

	//While there are nodes to remove from the edge list...
	while (iStackSize > 0)
	{
		//Pop the next node to be removed off of the remove stack
		uiTmp = removeStack[--iStackSize];

		//Look through the edge list for the all edges originating at node that is being removed
		for (i = 0, S_ptr = S_edgeList; i < uiNumEdges; )
		{
			//If this edge originates at the node being removed then...
			if (S_ptr->m_uiSrc == uiTmp)
			{
				//Push the destination node of the edge to the remove stack so it will also be removed
				removeStack[iStackSize++] = S_ptr->m_uiDest;

				//Remove this edge from the edge list (return any error)
				ucRet = ucRoute_RemoveEdge(S_ptr->m_uiSrc, S_ptr->m_uiDest);
				if (ucRet)
					return ucRet;
			}

			//Only increment these variables on iterations where an edge wasn't removed, since
			//that shifts the proceeding edge back to the current index and decrements uiNumEdges
			else
			{
				 i++;
				 S_ptr++;
			}
		}
	}

	return (0);
}

/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Returns either the number of added or dropped
//!
//! \param ucIndex
//! \return ucCount, the number of update records
////////////////////////////////////////////////////////////////////////////////
uchar ucRoute_GetUpdateCount(uchar ucAddOrDrop)
{
	uchar ucCount;
	uchar ucIndex;

	// Start at zero
	ucCount = 0;

	for (ucIndex = 0; ucIndex < S_RtUpdate.ucIndex; ucIndex++)
	{
		if (S_RtUpdate.m_ucaFlags[ucIndex] & ucAddOrDrop)
			ucCount++;
	}

	// Return
	return ucCount;
}

/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Returns either the number of added or dropped
//!
//! \param ucIndex
//! \return S_edgeList[ucIndex].m_uiSrc
////////////////////////////////////////////////////////////////////////////////
uchar ucRoute_GetUpdateCountBytes(uchar ucAddOrDrop)
{
	uchar ucJoinVal;
	uchar ucDropVal;
	uchar ucRetVal;

	ucRetVal = 0;

	if (ucAddOrDrop & F_JOIN)
	{
		ucJoinVal = ucRoute_GetUpdateCount(F_JOIN);
		ucJoinVal *= 4;
		ucRetVal += ucJoinVal;
	}
	if (ucAddOrDrop & F_DROP)
	{
		ucDropVal = ucRoute_GetUpdateCount(F_DROP);
		ucDropVal *= 2;
		ucRetVal += ucDropVal;
	}

	return ucRetVal;
}

/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Loads the update table into a buffer, typically the message buffer.
//!
//! This function first loads the joins then the drops.  The joins and drops are
//! preceded by a byte indicating the length of the join or drop section.
//!
//!
//! \param *ucaBuff, the message buffer; ucSpaceAvail, space in bytes,
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vRoute_GetUpdates(volatile uchar *ucaBuff, uchar ucSpaceAvail)
{
	uchar ucRtCount;
	uchar ucJoinCount;
	uchar ucDropCount;
	uint uiNewSrcNodeId;
	uint uiNewDestNodeId;

	// Get join count
	ucJoinCount = ucRoute_GetUpdateCountBytes(F_JOIN);

	// Fill join count field
	if (ucJoinCount < ucSpaceAvail)
		*ucaBuff++ = ucJoinCount;
	else
		*ucaBuff++ = ucSpaceAvail;

	// If there are no joins then don't bother iterating through the list
	if (ucJoinCount != 0)
	{
		// Fill join edge list
		for (ucRtCount = 0; ucRtCount < S_RtUpdate.ucIndex; ucRtCount++)
		{
			if (S_RtUpdate.m_ucaFlags[ucRtCount] & F_JOIN)
			{
				// Get the src and dest node addresses from the edge list
				uiNewSrcNodeId = S_RtUpdate.m_ucaEdges[ucRtCount].m_uiSrc;
				uiNewDestNodeId = S_RtUpdate.m_ucaEdges[ucRtCount].m_uiDest;

				// Flag the element for deletion
				S_RtUpdate.m_ucaFlags[ucRtCount] |= F_DELETE;

				// Add the addresses to the message
				*ucaBuff++ = (uchar) (uiNewSrcNodeId >> 8);
				*ucaBuff++ = (uchar) uiNewSrcNodeId;
				*ucaBuff++ = (uchar) (uiNewDestNodeId >> 8);
				*ucaBuff++ = (uchar) uiNewDestNodeId;

				// Decrement the available space by the length of 2 addresses
				ucSpaceAvail -= 4;

				// If there isn't enough room for the next join edge then break out
				if (ucSpaceAvail < 4)
					break;
			} //END if(F_JOIN)
		}//END: for(ucRtCount)
	}//END: if(ucJoinCount)

	// Get drop count
	ucDropCount = ucRoute_GetUpdateCountBytes(F_DROP);

	// Only append drops if there are any and if there is enough room for the length and one address
	if (ucDropCount != 0 && ucSpaceAvail > 3)
	{
		// Fill drop count field
		if (ucDropCount < ucSpaceAvail)
			*ucaBuff++ = ucDropCount;
		else
			*ucaBuff++ = ucSpaceAvail;

		// Fill drop node addresses
		for (ucRtCount = 0; ucRtCount < S_RtUpdate.ucIndex; ucRtCount++)
		{
			// For drops we only load the source node.  No point in transmitting the destination
			if (S_RtUpdate.m_ucaFlags[ucRtCount] & F_DROP)
			{
				// Get the src and dest node addresses from the edge list
				uiNewDestNodeId = S_RtUpdate.m_ucaEdges[ucRtCount].m_uiDest;

				// Flag the element for deletion
				S_RtUpdate.m_ucaFlags[ucRtCount] |= F_DELETE;

				// Add the addresses to the message
				*ucaBuff++ = (uchar) (uiNewDestNodeId >> 8);
				*ucaBuff++ = (uchar) uiNewDestNodeId;

				ucSpaceAvail -= 2;
			// If there isn't enough room for the drop edge then break out
			if (ucSpaceAvail < 2)
				break;

			} // END: if(F_DROP)
		} // END: for(ucRtCount)
	} // END: if( ucDropCount && ucSpaceAvail)

}//END: vRoute_GetUpdates();


/////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Loads the update table into a buffer, typically the message buffer.
//!
//! This function first loads the joins then the drops.  The joins and drops are
//! preceded by a byte indicating the length of the join or drop section.
//!
//!
//! \param *ucaBuff, the message buffer; ucSpaceAvail, space in bytes,
//! \return none
////////////////////////////////////////////////////////////////////////////////
void vRoute_SetUpdates(volatile uchar *ucaBuff)
{
	S_Edge S_EdgeLocal[MAX_UPDATES];
	uchar ucJoins;
	uchar ucDrops;
	uint uiDropAddr;
	uchar ucCount;

	// Convert the join field from length for the following loop
	ucJoins = (*ucaBuff++)/4;

	for(ucCount = 0; ucCount < ucJoins; ucCount++)
	{
		S_EdgeLocal[ucCount].m_uiSrc = (uint) (*ucaBuff++<<8);
		S_EdgeLocal[ucCount].m_uiSrc |= (uint) (*ucaBuff++);
		S_EdgeLocal[ucCount].m_uiDest = (uint) (*ucaBuff++<<8);
		S_EdgeLocal[ucCount].m_uiDest |= (uint) (*ucaBuff++);

		ucRoute_AddEdge(S_EdgeLocal[ucCount].m_uiSrc, S_EdgeLocal[ucCount].m_uiDest);

	}


	// Get the number of drops in terms of nodes instead of bytes
	ucDrops = (*ucaBuff++)/2;

		for(ucCount = 0; ucCount < ucDrops; ucCount++)
		{
			uiDropAddr = (uint) (*ucaBuff++);
			uiDropAddr = uiDropAddr << 8;
			uiDropAddr |= (uint) (*ucaBuff++);

			ucRoute_NodeUnjoin(uiDropAddr);

		}


}

///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Clears the joins update table
//!
//! \param none
//! \return none
///////////////////////////////////////////////////////////////////////////////
void vRouteClrAllUpdates(void)
{
	uchar ucIndex;

	// Empty the update list
	for (ucIndex = 0; ucIndex < MAX_UPDATES; ucIndex++)
	{
		S_RtUpdate.m_ucaEdges[ucIndex].m_uiDest = 0;
		S_RtUpdate.m_ucaEdges[ucIndex].m_uiSrc = 0;
		S_RtUpdate.m_ucaFlags[ucIndex] = 0;
	}

	// Set the route update counts to 0
	S_RtUpdate.ucIndex = 0;

}



///////////////////////////////////////////////////////////////////////////////
//!
//! \brief Clears flagged elements from the update table
//!
//! \param none
//! \return none
///////////////////////////////////////////////////////////////////////////////
void vRouteClrFlaggedUpdates(void)
{
	uchar ucIndex;
	uchar ucTmpCount;
	uchar ucTmpIndex;

	ucTmpCount = 0;

	// Iterate through the Join List
	ucIndex = S_RtUpdate.ucIndex;

	while(ucIndex > 0)
	{
		ucIndex--;

		// If the delete flag is set and the add or drop flag matches the passed parameter.
	if((S_RtUpdate.m_ucaFlags[ucIndex] & F_DELETE))
		{
			// Clear flag and the contents of the element in question
			S_RtUpdate.m_ucaFlags[ucIndex] = 0;
			S_RtUpdate.m_ucaEdges[ucIndex].m_uiSrc = 0;
			S_RtUpdate.m_ucaEdges[ucIndex].m_uiDest = 0;

			// Decrement the update count
			ucTmpCount++;

			// Store the empty location in a temp variable for the shifting algorithm
			ucTmpIndex=ucIndex;
		}
	}


	if (ucTmpCount > 0)
	{
		// Shift the contents of the array over
		for (ucIndex = 0; ucIndex < S_RtUpdate.ucIndex; ucIndex++)
		{
			if (S_RtUpdate.m_ucaEdges[ucIndex].m_uiDest != 0)
			{
				S_RtUpdate.m_ucaEdges[ucTmpIndex] = S_RtUpdate.m_ucaEdges[ucIndex];
				S_RtUpdate.m_ucaFlags[ucTmpIndex] = S_RtUpdate.m_ucaFlags[ucIndex];
				// Zero the last edge in the list to avoid duplicates
				S_RtUpdate.m_ucaEdges[ucIndex].m_uiSrc = 0;
				S_RtUpdate.m_ucaEdges[ucIndex].m_uiDest = 0;
				S_RtUpdate.m_ucaFlags[ucIndex] = 0;
				ucTmpIndex++;
			}
		}
	}

	S_RtUpdate.ucIndex -= ucTmpCount;

}


void vRoute_DisplayEdges(void)
{
	uint uiEdgeCount;

	vSERIAL_sout("SOURCE    DESTINATION", 21);
	vSERIAL_crlf();
	for(uiEdgeCount = 0; uiEdgeCount < uiNumEdges; uiEdgeCount++)
	{
		vSERIAL_HB16out(S_edgeList[uiEdgeCount].m_uiSrc);
		vSERIAL_sout("      ", 6);
		vSERIAL_HB16out(S_edgeList[uiEdgeCount].m_uiDest);
		vSERIAL_crlf();
	}

}
//! @}
