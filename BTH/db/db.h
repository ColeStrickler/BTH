#pragma once
#include "..\Dependencies\sqlite\sqlite3.h"
#include "settings.h"
#include <fstream>
#include <iostream>
#include <string>
#define BTH_DBSTRING ".\\bth.db"


/*
	if we are to add a new color setting we need to edit the following:
	1. COLORSETTINGS_INITVEC -->
*/




// COLOR SETTINGS SCHEMA DEFAULTS
#define COLOR_SETTINGS_SCHEMA				"(name, R, G, B, A)"
#define BACKGROUND_COLOR_DEFAULT			"('BACKGROUND_COLOR', 0.19, 0.36, 0.51, 1.00)"
#define HEXDUMP_BACKGROUNDCOLOR_DEFAULT		"('HEXDUMP_BACKGROUND_COLOR', 0.00, 0.00, 0.00, 1.00)"
#define HEXDUMP_BUTTONCOLOR_DEFAULT			"('HEXDUMP_BUTTONCOLOR', 0.11, 0.12, 0.52, 1.00)"
#define HEXDUMP_TEXTCOLOR_DEFAULT			"('HEXDUMP_TEXTCOLOR', 0.76, 0.76, 0.90, 1.00)"



/*
	This vector is used in CreateDatabase() to initialize the initial RGB values for various components
*/
static const std::vector<std::string> COLORSETTINGS_INITVEC = {
	BACKGROUND_COLOR_DEFAULT,
	HEXDUMP_BACKGROUNDCOLOR_DEFAULT,
	HEXDUMP_BUTTONCOLOR_DEFAULT,
	HEXDUMP_TEXTCOLOR_DEFAULT,
};


/*
	This vector will be used in querying the saved ColorSettings from the database in LoadState(),
	and also used in modification of ColorSettings
*/
static const std::vector<std::string> COLORSETTINGS_QUERYSTRING = {
	"'BACKGROUND_COLOR'",
	"'HEXDUMP_BACKGROUND_COLOR'",
	"'HEXDUMP_BUTTONCOLOR'",
	"'HEXDUMP_TEXTCOLOR'"
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



	
	// RETRIEVAL FUNCTIONS
	float RetrieveFloat(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria, int data_column);
	int RetrieveInt(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria, int data_column);
	std::string RetrieveString(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria, int data_column);


	ImVec4 GetColorSetting(VISUALS_INDEX index) const { return m_SavedSettings->GetColorSettings(index); };

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










