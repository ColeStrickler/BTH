#include "bind_helpers.h"

pybind11::list BIND::SetDosRes1(const IMAGE_DOS_HEADER& dosHeader)
{
	pybind11::list ret;
	for (int i = 0; i < 4; i++)
	{
		ret.append(static_cast<int>(dosHeader.e_res[i]));
	}
	return ret;
};


pybind11::list BIND::SetDosRes2(const IMAGE_DOS_HEADER& dosHeader)
{
	pybind11::list ret;
	for (int i = 0; i < 10; i++)
	{
		ret.append(static_cast<int>(dosHeader.e_res2[i]));
	}
	return ret;
}

std::string BIND::RecoverSectionHeaderName(const IMAGE_SECTION_HEADER& sect)
{
	return std::string((char*)sect.Name);
}

pybind11::list BIND::SetLibImports(const fh_LibraryImport& lib)
{
	pybind11::list ret;

	for (auto& f : lib.m_FunctionImports)
	{
		pybind11::list func;
		func.append(f.m_FunctionName);
		func.append(f.m_RawThunk);
		ret.append(func);
	}
	return ret;
}
