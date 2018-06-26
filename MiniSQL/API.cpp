﻿#include "API.h"


#include <iostream>

#include <boost/lexical_cast.hpp>

#include "BufferBlock.h"
#include "IndexManager.h"
#include "CatalogManager.h"
#include "QueryException.h"
#include "Data.h"
#include "DataC.h"
#include "DataF.h"
#include "DataI.h"

API::API()
{
}


API::~API()
{
}


Data * API::toData(const int attr_flag, const string & value)
{
	Data * data = nullptr;
	if (attr_flag == -1) {
		// int
		int d;
		try {
			d = boost::lexical_cast<int>(value);
		}
		catch (...) {
			throw QueryException("the format of length is invalid.");
		}

		data = new DataI(d);
	}
	else if (attr_flag == 0) {
		// float
		float d;
		try {
			d = boost::lexical_cast<float>(value);
		}
		catch (...) {
			throw QueryException("the format of length is invalid.");
		}

		data = new DataF(d);
	}
	else {
		// char
		if (value.front() == '\'' && value.back() == '\'') {
			data = new DataC(value.substr(1, value.size() - 2));
		} else if (value.front() == '"' && value.back() == '"') {
			data = new DataC(value.substr(1, value.size() - 2));
		} else {
			data = new DataC(value);
		}
	}
	return data;
}


void API::toWhere(vector<Where> & where_conds, const vector<Condition> & conds, Table * table_ptr)
{
	for (const auto & cond : conds) {
		Where w;

		std::string op;
		switch (cond.type) {
		case Condition::Equal:
			w.flag = Where::eq;
			break;
		case Condition::NotEqual:
			w.flag = Where::neq;
			break;
		case Condition::Greater:
			w.flag = Where::g;
			break;
		case Condition::Less:
			w.flag = Where::l;
			break;
		case Condition::GreaterOrEqual:
			w.flag = Where::geq;
			break;
		case Condition::LessOrEqual:
			w.flag = Where::leq;
			break;
		}

		const auto & table_attr = table_ptr->getattribute();

		for (int i = 0; i < table_attr.num; i++) {
			const string & attr_name = table_attr.name[i];
			const int attr_flag = table_attr.flag[i];
			if (cond.column == attr_name) {
				w.d = API::toData(attr_flag, cond.value);
			}
		}

		where_conds.push_back(w);
	}
}

void API::select(
	const std::string & table,
	const std::vector<std::string> & columns,
	const std::vector<Condition> & conds
)
{
	CatalogManager catalog_manager;
	RecordManager record_manager;

	Table * table_ptr = catalog_manager.getTable(table);
	//Table * table_ptr = nullptr;

	//Table output = api.Select(*t, attrselect, attrwhere, w);

	//return rm.Select(tableIn, attrSelect, mask, w);

	vector<int> select_indices;

	vector<Where> where_conds;
	vector<int> where_indices;

	API::toWhere(where_conds, conds, table_ptr);

	const auto & table_attr = table_ptr->getattribute();
	
	for (int i = 0; i < table_attr.num; i++) {
		const string & attr_name = table_attr.name[i];
		for (const auto & cond : conds) {
			if (cond.column == attr_name) {
				where_indices.push_back(i);
			}
		}

		for (const auto & column : columns) {
			if (column == attr_name) {
				select_indices.push_back(i);
			}
		}
	}

	if (columns.size() == 1 && columns.back() == "*") {
		for (int i = 0; i < table_attr.num; i++) {
			select_indices.push_back(i);
		}
	}

	Table result = record_manager.Select(*table_ptr, select_indices, where_indices, where_conds);
	result.display();
}


void API::deleteFrom(
	const std::string & table,
	const std::vector<Condition> & conds
)
{
	CatalogManager catalog_manager;
	RecordManager record_manager;

	Table * table_ptr = catalog_manager.getTable(table);

	vector<Where> where_conds;
	vector<int> where_indices;

	API::toWhere(where_conds, conds, table_ptr);

	const auto & table_attr = table_ptr->getattribute();

	for (int i = 0; i < table_attr.num; i++) {
		const string & attr_name = table_attr.name[i];
		for (const auto & cond : conds) {
			if (cond.column == attr_name) {
				where_indices.push_back(i);
			}
		}
	}

	record_manager.Delete(*table_ptr, where_indices, where_conds);

	//std::cout << "----------" << std::endl;
	//std::cout << "Table: {" << table << "}" << std::endl;

	//std::cout << "Where: " << std::endl;
	//for (const auto & cond : conds) {
	//	std::cout << "    {" << cond.column << "} ";

	//	std::string op;
	//	switch (cond.type) {
	//	case Condition::Equal:
	//		op = "=";
	//		break;
	//	case Condition::NotEqual:
	//		op = "<>";
	//		break;
	//	case Condition::Greater:
	//		op = ">";
	//		break;
	//	case Condition::Less:
	//		op = "<";
	//		break;
	//	case Condition::GreaterOrEqual:
	//		op = ">=";
	//		break;
	//	case Condition::LessOrEqual:
	//		op = "<=";
	//		break;
	//	}

	//	std::cout << op << " {" << cond.value << "}" << std::endl;
	//}
	//std::cout << "----------" << std::endl;
}


void API::insert(
	const std::string & table,
	const std::vector<std::string> & values
)
{
	CatalogManager catalog_manager;
	RecordManager record_manager;

	Table * table_ptr = catalog_manager.getTable(table);

	const auto & table_attr = table_ptr->getattribute();

	Tuple tuple;
	for (size_t i = 0; i < values.size(); i++) {
		const std::string & value = values[i];
		const int attr_flag = table_attr.flag[i];
		Data * data = API::toData(attr_flag, value);
		tuple.addData(data);
	}

	record_manager.Insert(*table_ptr, tuple);
}


void API::createTable(
	const std::string & table,
	const std::string & primary_key,
	const std::vector<ColumnAttribute> & attrs
)
{
	Index index;
	index.num = 0;

	short primary = -1;
	Attribute atb;
	atb.num = attrs.size();
	for (int i = 0; i < attrs.size(); i++) {
		const ColumnAttribute & attr = attrs[i];

		int &type = atb.flag[i];
		atb.name[i] = attr.name;
		atb.unique[i] = attr.unique;

		switch (attr.type) {
		case ColumnAttribute::Character:
			type = attr.length;
			break;
		case ColumnAttribute::Integer:
			type = -1;
			break;
		case ColumnAttribute::Float:
			type = 0;
			break;
		}

		if (attr.name == primary_key) {
			primary = i;
			atb.unique[i] = true;
		}
	}

	BufferBlock buffer;
	CatalogManager catalog_manager;

	catalog_manager.create_table(table, atb, primary, index);

	if (primary != -1) {
		catalog_manager.create_index(table, primary_key, primary_key);
	}


	//IndexManager index_manager;
	////bool res;
	////int i;

	////getTable
	////res = record_manager.CreateTable(tableIn);
	//index_manager

	Table * table_ptr = catalog_manager.getTable(table);
	record_manager.CreateTable(*table_ptr, buffer);
	delete table_ptr;
}

void API::dropTable(
	const std::string & table
)
{
	std::cout << "----------" << std::endl;
	std::cout << "Table: {" << table << "}" << std::endl;
}

void API::createIndex(
	const std::string & table,
	const std::string & column,
	const std::string & index
)
{
	std::cout << "----------" << std::endl;
	std::cout << "Table: {" << table << "}" << std::endl;
	std::cout << "Column: {" << column << "}" << std::endl;
	std::cout << "Index: {" << index << "}" << std::endl;
}

void API::dropIndex(
	const std::string & index
)
{
	std::cout << "----------" << std::endl;
	std::cout << "Index: {" << index << "}" << std::endl;
}

