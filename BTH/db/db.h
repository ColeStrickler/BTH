#pragma once
#include "..\Dependencies\sqlite\sqlite3.h"
#include "settings.h"
#include <fstream>
#include <iostream>
#include <string>
#include "defaultstructs.h"
#include <format>


#define BTH_DBSTRING ".\\bth.db"


/*
	if we are to add a new color setting we need to edit the following:
	1. COLORSETTINGS_INITVEC
	2. COLORSETTINGS_QUERYSTRING
	3. VISUALS_INDEX

	[IMPORTANT!]
	Make sure that we put the item into COLORSETTINGS_INITVEC and COLORSETTINGS_QUERYSTRING
	at the index that that the VISUALS_INDEX::[name] enum is as we will use this to index
	into these vectors in other methods
*/



// [COLOR SETTINGS SCHEMA DEFAULTS]
#define COLOR_SETTINGS_SCHEMA					"(name, R, G, B, A)"

#define BACKGROUND_COLOR_DEFAULT				"('BACKGROUND_COLOR', 0.19, 0.36, 0.51, 1.00)"
#define BUTTON_COLOR_DEFAULT					"('BUTTON_COLOR', 0.11, 0.12, 0.52, 1.00)"
#define BUTTON_TEXT_COLOR_DEFAULT				"('BUTTON_TEXTCOLOR', 0.76, 0.76, 0.90, 1.00)"
#define TEXT_COLOR_DEFAULT						"('TEXT_COLOR', 0.76, 0.76, 0.90, 1.00)"

#define HEXDUMP_BACKGROUNDCOLOR_DEFAULT			"('HEXDUMP_BACKGROUND_COLOR', 0.00, 0.00, 0.00, 1.00)"
#define HEXDUMP_BUTTONCOLOR_DEFAULT				"('HEXDUMP_BUTTONCOLOR', 0.11, 0.12, 0.52, 1.00)"
#define HEXDUMP_TEXTCOLOR_DEFAULT				"('HEXDUMP_TEXTCOLOR', 0.76, 0.76, 0.90, 1.00)"

#define DISASSEMBLY_INST1_COLOR_DEFAULT			"('DISASSEMBLY_INST1_COLOR', 1.00, 0.00, 0.00, 1.00)"
#define DISASSEMBLY_INST2_COLOR_DEFAULT			"('DISASSEMBLY_INST2_COLOR', 0.00, 1.00, 0.00, 1.00)"
#define DISASSEMBLY_INST3_COLOR_DEFAULT			"('DISASSEMBLY_INST3_COLOR', 0.00, 1.00, 1.00, 1.00)"
#define DISASSEMBLY_INST4_COLOR_DEFAULT			"('DISASSEMBLY_INST4_COLOR', 1.00, 1.00, 0.20, 1.00)"


#define PEPARSER_BUTTON_COLOR_DEFAULT			"('PEPARSER_BUTTONCOLOR', 0.11, 0.12, 0.52, 1.00)"
#define PEPARSER_TEXT_COLOR_DEFAULT				"('PEPARSER_TEXTCOLOR', 0.76, 0.76, 0.90, 1.00)"
#define PEPARSER_BUTTONTEXT_COLOR_DEFAULT		"('PEPARSER_BUTTONTEXT_COLOR', 0.76, 0.76, 0.90, 1.00)"
#define PEPARSER_COLHEADER_COLOR_DEFAULT		"('PEPARSER_COLHEADER_COLOR', 0.26, 0.28, 0.30, 1.00)"
#define	PEPARSER_COLHEADERTEXT_COLOR_DEFAULT	"('PEPARSER_COLHEADERTEXT_COLOR', 0.76, 0.76, 0.90, 1.00)"




/*
	This vector is used in CreateDatabase() to initialize the initial RGB values for various components
*/
static const std::vector<std::string> COLORSETTINGS_INITVEC = {

	// GENERAL COLOR SETTINGS
	BACKGROUND_COLOR_DEFAULT,
	BUTTON_COLOR_DEFAULT,
	BUTTON_TEXT_COLOR_DEFAULT,
	TEXT_COLOR_DEFAULT,

	// HEX DUMP COLOR SETTINGS
	HEXDUMP_BACKGROUNDCOLOR_DEFAULT,
	HEXDUMP_BUTTONCOLOR_DEFAULT,
	HEXDUMP_TEXTCOLOR_DEFAULT,

	// DISASSEMBLY VISUALS
	DISASSEMBLY_INST1_COLOR_DEFAULT,
	DISASSEMBLY_INST2_COLOR_DEFAULT,
	DISASSEMBLY_INST3_COLOR_DEFAULT,
	DISASSEMBLY_INST4_COLOR_DEFAULT,
	// MEMORY DUMP VISUALS


	// PE DUMP VISUALS
	PEPARSER_BUTTON_COLOR_DEFAULT,
	PEPARSER_TEXT_COLOR_DEFAULT,
	PEPARSER_BUTTONTEXT_COLOR_DEFAULT,
	PEPARSER_COLHEADER_COLOR_DEFAULT,
	PEPARSER_COLHEADERTEXT_COLOR_DEFAULT

};


/*
	This vector will be used in querying the saved ColorSettings from the database in LoadState(),
	and also used in modification of ColorSettings
*/
static const std::vector<std::string> COLORSETTINGS_QUERYSTRING = {
	"'BACKGROUND_COLOR'",
	"'BUTTON_COLOR'",
	"'BUTTON_TEXTCOLOR'",
	"'TEXT_COLOR'",


	"'HEXDUMP_BACKGROUND_COLOR'",
	"'HEXDUMP_BUTTONCOLOR'",
	"'HEXDUMP_TEXTCOLOR'",


	"'DISASSEMBLY_INST1_COLOR'",
	"'DISASSEMBLY_INST2_COLOR'",
	"'DISASSEMBLY_INST3_COLOR'",
	"'DISASSEMBLY_INST4_COLOR'",


	"'PEPARSER_BUTTONCOLOR'",
	"'PEPARSER_TEXTCOLOR'",
	"'PEPARSER_BUTTONTEXT_COLOR'",
	"'PEPARSER_COLHEADER_COLOR'",
	"'PEPARSER_COLHEADERTEXT_COLOR'",
};

struct saved_struct
{
	std::string m_Name;
	int m_MemberCount;
};


class db_mgr
{
public:
	db_mgr();
	~db_mgr();

	int m_ErrorCode;
	
	int InsertData(const std::string& query);
	

	// UPDATE FUNCTIONS
	void UpdateDataItem(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria,
						const std::string& update_attribute, const std::string& update_value);
	void UpdateColorSetting(const std::string& name, const ImVec4& Color);
	void SaveStructure(const MemDumpStructure& structure);


	
	// RETRIEVAL FUNCTIONS
	float RetrieveFloat(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria, int data_column);
	int RetrieveInt(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria, int data_column);
	std::string RetrieveString(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria, int data_column);
	ImVec4 GetColorSetting(VISUALS_INDEX index) const { return m_SavedSettings->GetColorSettings(index); };
	std::vector<saved_struct> RetrieveSavedStructures();
	MemDumpStructEntry RetrieveStructMember(const std::string& parent_structure, const std::string& member_index);

	std::vector<default_struct> m_DefaultStructs;

private:
	void LoadState();
	void CreateDatabase();

	




private:
	sqlite3* m_DB;
	ManagerSettings* m_SavedSettings;


};

/*
	[QUERY FORMAT FUNCTIONS]
*/










