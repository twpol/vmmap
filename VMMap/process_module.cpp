//------------------------------------------------------------------------------
// http://twpol.dyndns.org/projects/vmmap
// License: New BSD License (BSD).
//------------------------------------------------------------------------------

#include "StdAfx.h"
#include "process_module.h"

process_module::process_module(unsigned long long base, unsigned long long size, const std::tstring& image_filepath) : _base(base), _size(size), _image_filepath(image_filepath)
{
}

process_module::~process_module(void)
{
}
