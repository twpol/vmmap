//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#pragma once

class process_heap
{
private:
	unsigned long _id;
	unsigned long long _base;
	bool _default;
	std::list<std::pair<unsigned long long, unsigned long long> > _blocks;
public:
	process_heap(const unsigned long id, const unsigned long long base, const bool default);
	~process_heap(void);
	const unsigned long id(void) const { return _id; }
	const unsigned long long base(void) const { return _base; }
	const unsigned long long default(void) const { return _default; }
	operator unsigned long long() const { return _base; }
};
