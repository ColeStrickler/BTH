#include "db.h"



/*
	[QUERY FORMAT FUNCTIONS]
*/
static std::string FormatInsertQuery(const std::string& table, const std::string& attributes, const std::string& values)
{
	std::string insertion_query = "INSERT INTO ";
	insertion_query += table + " ";
	insertion_query += attributes + " ";
	insertion_query += "VALUES ";
	insertion_query += values + ";";
	return insertion_query;
}

static std::string FormatRetrievalQuery(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria)
{
	std::string retrieval_query = "SELECT * FROM ";
	retrieval_query += table + " WHERE ";
	retrieval_query += selection_attribute + " = ";
	retrieval_query += selection_criteria + ";";
	return retrieval_query;
}

static std::string FormatStructMemberRetrievalQuery(const std::string& selection_attribute, const std::string& selection_criteria, const std::string& member_index)
{
	std::string retrieval_query = "SELECT * FROM ";
	retrieval_query += "StructureMembers WHERE ";
	retrieval_query += selection_attribute + " = ";
	retrieval_query += selection_criteria;
	retrieval_query += " AND member_index = ";
	retrieval_query += member_index + ";";
	return retrieval_query;
}



static std::string FormatUpdateQuery(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria,
									 const std::string& update_attribute, const std::string& update_value)
{
	std::string update_query = "UPDATE ";
	update_query += table;
	update_query += " SET ";
	update_query += update_attribute;
	update_query += " = ";
	update_query += update_value;
	update_query += " WHERE ";
	update_query += selection_attribute;
	update_query += " = ";
	update_query += selection_criteria;
	update_query += ";";
	return update_query;
}






db_mgr::db_mgr() 
{
	std::ifstream file(BTH_DBSTRING);


	int err = sqlite3_open(BTH_DBSTRING, &m_DB);
	if (err)
	{
		m_ErrorCode = err;
		return;
	}
		
	if (file.fail() || !file.is_open())
	{
		CreateDatabase();
	}
	else
	{
		file.close();
	}
		
	LoadState();
}

db_mgr::~db_mgr()
{
	sqlite3_close(m_DB);
	delete m_SavedSettings;
}

int db_mgr::InsertData(const std::string& query)
{
	int err;
	err = sqlite3_exec(m_DB, query.c_str(), 0, 0, 0);
	return err;
}


void db_mgr::LoadState()
{
	if (m_SavedSettings)
		delete m_SavedSettings;
	std::vector<ImVec4> SettingsInitVec;
	for (auto& query_string : COLORSETTINGS_QUERYSTRING)
	{
		auto R = RetrieveFloat("ColorSettings", "name", query_string, 2);
		auto G = RetrieveFloat("ColorSettings", "name", query_string, 3);
		auto B = RetrieveFloat("ColorSettings", "name", query_string, 4);
		auto A = RetrieveFloat("ColorSettings", "name", query_string, 5);
		SettingsInitVec.push_back(ImVec4(R, G, B, A));
	}
	m_SavedSettings = new ManagerSettings(SettingsInitVec);


	auto saved_structs = RetrieveSavedStructures();
	for (auto& def_struct : saved_structs)
	{
		auto format_name = std::format("'{}'", def_struct.m_Name);
		auto member_count = def_struct.m_MemberCount;
		default_struct ds;
		ds.m_Name = def_struct.m_Name;

		for (int i = 0; i < member_count; i++)
		{
			ds.m_Entry.push_back(RetrieveStructMember(ds.m_Name, std::to_string(i)));
		}
		m_DefaultStructs.push_back(ds);
	}


}

void db_mgr::CreateDatabase()
{
	int err;
	const char* CREATE_COLOR_TABLE = "CREATE TABLE IF NOT EXISTS ColorSettings (\
		id INTEGER PRIMARY KEY AUTOINCREMENT,\
		name TEXT,\
		R NUMERIC(3, 2),\
		G NUMERIC(3, 2),\
		B NUMERIC(3, 2),\
		A NUMERIC(3, 2));";


	const char* CREATE_STRUCT_TABLE = "CREATE TABLE IF NOT EXISTS Structures (\
		id INTEGER PRIMARY KEY AUTOINCREMENT,\
		name TEXT,\
		member_count INTEGER);";

	const char* CREATE_STRUCTMEMBER_TABLE = "CREATE TABLE IF NOT EXISTS StructureMembers (\
		id INTEGER PRIMARY KEY AUTOINCREMENT,\
		member_name TEXT,\
		parent_structure_name TEXT,\
		member_index INTEGER,\
		display_type INTEGER,\
		size INTEGER\);";




	// CREATE TABLES -> ColorSettings, Structures, StructureMembers
	if ((err = sqlite3_exec(m_DB, CREATE_COLOR_TABLE, 0, 0, 0)) != SQLITE_OK)
	{
		m_ErrorCode = err;
		return;
	}

	if ((err = sqlite3_exec(m_DB, CREATE_STRUCT_TABLE, 0, 0, 0)) != SQLITE_OK)
	{
		m_ErrorCode = err;
		return;
	}
	if ((err = sqlite3_exec(m_DB, CREATE_STRUCTMEMBER_TABLE, 0, 0, 0)) != SQLITE_OK)
	{
		m_ErrorCode = err;
		return;
	}

	// INSERT DEFAULT COLOR SETTINGS INTO DATABASE
	for (auto& default_setting : COLORSETTINGS_INITVEC)
	{
		auto INSERT_QUERY = FormatInsertQuery("ColorSettings", COLOR_SETTINGS_SCHEMA, default_setting);
		if ((err = sqlite3_exec(m_DB, INSERT_QUERY.c_str(), 0, 0, 0)) != SQLITE_OK)
		{
			m_ErrorCode = err;
			return;
		}
	}

	// INSERT DEFAULT STRUCTURES INTO DATABASE 
	for (auto& def_struct : DEFAULT_STRUCTS)
	{
		auto attribute_value = std::format("('{}', {})", def_struct.m_Name, def_struct.m_Entry.size());
		auto INSERT_QUERY = FormatInsertQuery("Structures", STRUCTURE_SCHEMA, attribute_value);
		if ((err = sqlite3_exec(m_DB, INSERT_QUERY.c_str(), 0, 0, 0)) != SQLITE_OK)
		{
			m_ErrorCode = err;
			return;
		}

		auto& members = def_struct.m_Entry;
		auto member_count = members.size();

		// FOR EACH STRUCTURE, ALSO INSERT DATA ABOUT ITS MEMBER VARIABLES INTO THE DATABSE IN THE "StructureMembers" table
		for (int i = 0; i < member_count; i++)
		{
			auto& name = members[i].m_GivenName;
			auto& parent_name = def_struct.m_Name;
			auto& display_type = members[i].m_Display;
			auto& size = members[i].m_Size;
			auto attrib_val = std::format("('{}', '{}', {}, {}, {})", name, parent_name, i, (int)display_type, (int)size);
			auto INSERT_MEMBER_QUERY = FormatInsertQuery("StructureMembers", STRUCTURE_MEMBER_SCHEMA, attrib_val);

			if ((err = sqlite3_exec(m_DB, INSERT_MEMBER_QUERY.c_str(), 0, 0, 0)) != SQLITE_OK)
			{
				m_ErrorCode = err;
				return;
			}
		}

	}

}



/*
	[UPDATE FUNCTIONS]
*/

void db_mgr::UpdateDataItem(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria,
							const std::string& update_attribute, const std::string& update_value)
{
	int err;
	auto update_query = FormatUpdateQuery(table, selection_attribute, selection_criteria, update_attribute, update_value);
	if ((err = sqlite3_exec(m_DB, update_query.c_str(), 0, 0, 0)) != SQLITE_OK)
	{
		m_ErrorCode = err;
		return;
	}

}

void db_mgr::UpdateColorSetting(const std::string& name, const ImVec4& Color)
{
	auto R_VAL = std::to_string(Color.x);
	auto G_VAL = std::to_string(Color.y);
	auto B_VAL = std::to_string(Color.z);
	auto A_VAL = std::to_string(Color.w);

	UpdateDataItem("ColorSettings", "name", name, "R", R_VAL);
	UpdateDataItem("ColorSettings", "name", name, "G", G_VAL);
	UpdateDataItem("ColorSettings", "name", name, "B", B_VAL);
	UpdateDataItem("ColorSettings", "name", name, "A", A_VAL);

	// Reload State
	LoadState();
}

void db_mgr::SaveStructure(const MemDumpStructure& structure)
{
	int err;
	auto entries = structure.GetAllEntries();


	auto attribute_value = std::format("('{}', {})", structure.m_Name, entries.size());
	auto INSERT_QUERY = FormatInsertQuery("Structures", STRUCTURE_SCHEMA, attribute_value);
	if ((err = sqlite3_exec(m_DB, INSERT_QUERY.c_str(), 0, 0, 0)) != SQLITE_OK)
	{
		m_ErrorCode = err;
		return;
	}

	auto& members = entries;
	auto member_count = members.size();

	// FOR EACH STRUCTURE, ALSO INSERT DATA ABOUT ITS MEMBER VARIABLES INTO THE DATABSE IN THE "StructureMembers" table
	for (int i = 0; i < member_count; i++)
	{
		auto& name = members[i].m_GivenName;
		auto& parent_name = structure.m_Name;
		auto& display_type = members[i].m_Display;
		auto& size = members[i].m_Size;
		auto attrib_val = std::format("('{}', '{}', {}, {}, {})", name, parent_name, i, (int)display_type, (int)size);
		auto INSERT_MEMBER_QUERY = FormatInsertQuery("StructureMembers", STRUCTURE_MEMBER_SCHEMA, attrib_val);

		if ((err = sqlite3_exec(m_DB, INSERT_MEMBER_QUERY.c_str(), 0, 0, 0)) != SQLITE_OK)
		{
			m_ErrorCode = err;
			return;
		}
	}
	return;;
}







/*
	[RETRIEVAL FUNCTIONS]
*/


/*
	1. Query will look like "SELECT * FROM table WHERE selection_attribute = selection_criteria;"
	2. The the first row that is returned is then accessed at data_column and returns a value of type T
*/
float db_mgr::RetrieveFloat(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria, int data_column)
{
	sqlite3_stmt* stmt;
	int err;
	auto retrieval_query = FormatRetrievalQuery(table, selection_attribute, selection_criteria);
	if ((err = sqlite3_prepare_v2(m_DB, retrieval_query.c_str(), -1, &stmt, 0)) == -1)
	{
		m_ErrorCode = err;
		return 0.0f;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		float data_item = static_cast<float>(sqlite3_column_double(stmt, data_column));
		sqlite3_finalize(stmt);
		return data_item;
	}
	sqlite3_finalize(stmt);
	return 0.0f;
}

int db_mgr::RetrieveInt(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria, int data_column)
{
	sqlite3_stmt* stmt;
	int err;
	auto retrieval_query = FormatRetrievalQuery(table, selection_attribute, selection_criteria);
	if ((err = sqlite3_prepare_v2(m_DB, retrieval_query.c_str(), -1, &stmt, 0)) == -1)
	{
		m_ErrorCode = err;
		return 0;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int data_item = sqlite3_column_int(stmt, data_column);
		sqlite3_finalize(stmt);
		return data_item;
	}
	sqlite3_finalize(stmt);
	return 0;
}

std::string db_mgr::RetrieveString(const std::string& table, const std::string& selection_attribute, const std::string& selection_criteria, int data_column)
{
	sqlite3_stmt* stmt;
	int err;
	auto retrieval_query = FormatRetrievalQuery(table, selection_attribute, selection_criteria);
	if ((err = sqlite3_prepare_v2(m_DB, retrieval_query.c_str(), -1, &stmt, 0)) == -1)
	{
		m_ErrorCode = err;
		return "";
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		std::string data_item = (const char*)sqlite3_column_text(stmt, data_column);
		sqlite3_finalize(stmt);
		return data_item;
	}
	sqlite3_finalize(stmt);
	return "";
}

std::vector<saved_struct> db_mgr::RetrieveSavedStructures()
{
	int err;
	std::vector<saved_struct> ret;
	const char* QUERY = "SELECT * FROM Structures;";
	sqlite3_stmt* stmt;
	err = sqlite3_prepare_v2(m_DB, QUERY, -1, &stmt, nullptr);

	if (err != SQLITE_OK) {
		return {};
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		// Assuming you have a single TEXT column in your table
		saved_struct ss;
		ss.m_Name = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
		ss.m_MemberCount = sqlite3_column_int(stmt, 2);
		ret.push_back(ss);
	}

	return ret;
}
MemDumpStructEntry db_mgr::RetrieveStructMember(const std::string& parent_structure, const std::string& member_index)
{
	int err;
	MemDumpStructEntry ret;
	auto format_parent_structure = std::format("'{}'", parent_structure);
	auto RETRIEVAL_QUERY = FormatStructMemberRetrievalQuery("parent_structure_name", format_parent_structure, member_index);
	sqlite3_stmt* stmt;
	err = sqlite3_prepare_v2(m_DB, RETRIEVAL_QUERY.c_str(), -1, &stmt, nullptr);

	if (err != SQLITE_OK) {
		return {};
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		// Assuming you have a single TEXT column in your table
		
		
		ret.m_GivenName = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
		ret.m_Display = (MEMDUMPDISPLAY)sqlite3_column_int(stmt, 4);
		ret.m_Size = sqlite3_column_int(stmt, 5);
		break;
	}

	return ret;
}





