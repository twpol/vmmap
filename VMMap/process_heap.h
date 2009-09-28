//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: Microsoft Public License (Ms-PL).
//------------------------------------------------------------------------------

#pragma once

class process_heap
{
private:
	unsigned long _id;
	unsigned long long _base;
	bool _default;
	std::list<const std::pair<const unsigned long long, const unsigned long long> > _blocks;
public:
	process_heap(unsigned long id, unsigned long long base, bool default);
	~process_heap(void);
	unsigned long id(void) const { return _id; }
	unsigned long long base(void) const { return _base; }
	unsigned long long default(void) const { return _default; }
	operator unsigned long long() const { return _base; }
};
