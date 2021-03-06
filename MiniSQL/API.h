#pragma once

#include <vector>
#include <string>
#include "Condition.h"
#include "ColumnAttribute.h"
#include "RecordManager.h"

class API
{
private:
	RecordManager record_manager;

	static Data * toData(const int attr_flag, const string & value);
	static void toWhere(vector<Where> & where_conds, const vector<Condition> & conds, Table * table_ptr);

public:
	API();
	virtual ~API();

	void select(
		const std::string & table,
		const std::vector<std::string> & columns,
		const std::vector<Condition> & conds
	);

	void deleteFrom(
		const std::string & table,
		const std::vector<Condition> & conds
	);

	void insert(
		const std::string & table,
		const std::vector<std::string> & values
	);

	void createTable(
		const std::string & table,
		const std::string & primary_key,
		const std::vector<ColumnAttribute> & attrs
	);

	void dropTable(
		const std::string & table
	);

	void createIndex(
		const std::string & table,
		const std::string & column,
		const std::string & index
	);

	void dropIndex(
		const std::string & index
	);
};

