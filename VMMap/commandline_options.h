//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#pragma once

class commandline_options
{
private:
	std::map<std::tstring, std::list<std::tstring> > _arguments;
	std::list<std::tstring> _files;
public:
	static const std::tstring nopt;
	// Creates a new commandline_options from the process' command line and
	// the given settings. The settings have the following syntax:
	//   [:]NAME1[ ALIAS1[ ALIAS2...]],[:]NAME2[ ALIAS3[ ALIAS4...]],...
	//
	// NAME1, NAME2 specify the canonical names for the options.
	// : indicates the option takes a value, which is taken automatically.
	// ALIAS1, ALIAS2, ... give NAME1 aliases which can be used instead.
	commandline_options(const std::tstring);
	~commandline_options(void) {}
	// Checks whether a specific option was specified.
	const bool has(const std::tstring) const;
	// Returns the value of a specified option.
	const std::tstring& get(const std::tstring) const;
	// Returns a list of values for a multiply-specified option.
	const std::list<std::tstring>& gets(const std::tstring) const;
	// Returns a list of non-argument values.
	const std::list<std::tstring>& files(void) const { return _files; }
};
