//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "process_stack.h"

process_stack::process_stack(unsigned long id, unsigned long long base, process_stack_type type) : _id(id), _base(base), _type(type)
{
}

process_stack::~process_stack(void)
{
}
