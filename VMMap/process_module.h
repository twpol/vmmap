//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: Microsoft Public License (Ms-PL).
//------------------------------------------------------------------------------

#pragma once

class process_module
{
private:
	unsigned long long _base;
	unsigned long long _size;
	std::tstring _image_filepath;
public:
	process_module(unsigned long long base, unsigned long long size, const std::tstring& image_filepath);
	~process_module(void);
	unsigned long long base(void) const { return _base; }
	unsigned long long size(void) const { return _size; }
	std::tstring image_filepath(void) const { return _image_filepath; }
	operator unsigned long long() const { return _base; }
};
