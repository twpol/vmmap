//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
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
