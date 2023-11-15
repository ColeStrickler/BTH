#pragma once
#include "..\pe\pe.h"
#include "../Dependencies/python/include/Python.h"
#include "../Dependencies/python/pybind11/pybind11.h"
#include "../Dependencies/python/pybind11/embed.h"
#include "../Dependencies/python/pybind11/stl.h"
#include <Windows.h>
#include <string>

// for some reason not adding this caused errors. gotta love linkers
struct fh_LibraryImport;

namespace BIND
{
	pybind11::list SetLibImports(const fh_LibraryImport& lib);
	pybind11::list SetDosRes1(const IMAGE_DOS_HEADER& dosHeader);
	pybind11::list SetDosRes2(const IMAGE_DOS_HEADER& dosHeader);

	std::string RecoverSectionHeaderName(const IMAGE_SECTION_HEADER& sect);


	struct RichHeaderEntryBinding
	{
		std::string ProductId;
		std::string VsVersion;
		int UseCount;
	};

};
