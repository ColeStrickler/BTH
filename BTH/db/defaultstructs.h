#pragma once
#include <vector>
#include "..\memdump\memdump.h"

#define STRUCTURE_SCHEMA			"(name, member_count)"
#define STRUCTURE_MEMBER_SCHEMA		"(member_name, parent_structure_name, member_index, display_type, size)"




struct default_struct
{
	std::string m_Name;
	std::vector<MemDumpStructEntry> m_Entry;
};


const std::vector<MemDumpStructEntry> _IMAGE_DOS_HEADER
{
	{2, MEMDUMPDISPLAY::HEX, "e_magic"},
	{2, MEMDUMPDISPLAY::HEX, "e_cblp"},
	{2, MEMDUMPDISPLAY::HEX, "e_cp"},
	{2, MEMDUMPDISPLAY::HEX, "e_rlc"},
	{2, MEMDUMPDISPLAY::HEX, "e_cparhdr"},
	{2, MEMDUMPDISPLAY::HEX, "e_minalloc"},
	{2, MEMDUMPDISPLAY::HEX, "e_maxalloc"},
	{2, MEMDUMPDISPLAY::HEX, "e_ss"},
	{2, MEMDUMPDISPLAY::HEX, "e_sp"},
	{2, MEMDUMPDISPLAY::HEX, "e_csum"},
	{2, MEMDUMPDISPLAY::HEX, "e_ip"},
	{2, MEMDUMPDISPLAY::HEX, "e_cs"},
	{2, MEMDUMPDISPLAY::HEX, "e_lfarlc"},
	{2, MEMDUMPDISPLAY::HEX, "e_ovno"},
	{8, MEMDUMPDISPLAY::HEX, "e_res"},
	{2, MEMDUMPDISPLAY::HEX, "e_oemid"},
	{2, MEMDUMPDISPLAY::HEX, "e_oeminfo"},
	{20, MEMDUMPDISPLAY::HEX, "e_res2"},
	{4, MEMDUMPDISPLAY::UNSIGNED_INT, "e_lfanew"},
};


const std::vector<default_struct> DEFAULT_STRUCTS
{
	{"IMAGE_DOS_HEADER", _IMAGE_DOS_HEADER}

};









