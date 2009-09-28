//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: Microsoft Public License (Ms-PL).
//------------------------------------------------------------------------------

#pragma once

#include "process.h"

class process_list
{
private:
	std::list<const process> _processes;
public:
	process_list(void);
	~process_list(void);
	const std::list<const process>& get() const { return _processes; }
};
