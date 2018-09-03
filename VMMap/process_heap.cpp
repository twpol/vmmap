//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "process_heap.h"

process_heap::process_heap(unsigned long id, unsigned long long base, bool default) : _id(id), _base(base), _default(default)
{
}

process_heap::~process_heap(void)
{
}

const void process_heap::add_block(ULONG_PTR start_address, ULONG_PTR end_address){
	_blocks.push_back(std::make_pair(start_address, end_address));
}