//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#pragma once

enum process_stack_type
{
	PST__FIRST = 0,
	PST_32BIT = PST__FIRST,
	PST_64BIT,
	PST__LAST
};

class process_stack
{
private:
	unsigned long _id;
	unsigned long long _base;
	process_stack_type _type;
public:
	process_stack(const unsigned long id, const unsigned long long base, const process_stack_type type);
	~process_stack(void);
	const unsigned long id(void) const { return _id; }
	const unsigned long long base(void) const { return _base; }
	const process_stack_type type(void) const { return _type; }
	operator unsigned long long() const { return _base; }
};
