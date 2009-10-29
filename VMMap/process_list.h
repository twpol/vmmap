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
	// Create a process list for this user.
	process_list(void);
	~process_list(void);
	// Returns a list of processes found.
	const std::list<const process>& get() const { return _processes; }
};
