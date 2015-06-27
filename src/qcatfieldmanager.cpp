#include "qcatfieldmanager.h"
#include "qcatdatasource.h"
#include <iostream>

QCATFieldManager::QCATFieldManager(QCATDataSource* db)
{
    scan(db);
}

void QCATFieldManager::scan(QCATDataSource* db)
{
    std::string sql = "select * from information_schema.columns where table_name = '" + db->table() + "'";
    auto rows = db->executeSQL(sql);

    for(int i=0;i<rows->nrows();i++) {
        std::string name = std::string(rows->get(i,"column_name"));
        int idx = rows->getInt(i,"ordinal_position");
        std::string type = std::string(rows->get(i,"data_type"));

        auto field = shared_ptr<QCATField>(new QCATField(db,name,idx,fieldTypeFromStr(type)));
        m_fieldMap[field->name()] = field;
        m_fieldVector.push_back(field);
    }
}

int QCATFieldManager::size() const
{
    return m_fieldVector.size();
}

shared_ptr<QCATField> QCATFieldManager::at(int idx) const
{
    return m_fieldVector.at(idx);
}

shared_ptr<QCATField> QCATFieldManager::named(std::string name) const
{
    return m_fieldMap.at(name);
}

std::vector<shared_ptr<QCATField> > QCATFieldManager::vector() const
{
    return m_fieldVector;
}

std::map<std::string,shared_ptr<QCATField> > QCATFieldManager::map() const
{
    return m_fieldMap;
}

QCATFieldType QCATFieldManager::fieldTypeFromStr(std::string str)
{
    if(str == "integer" || str == "smallint" || str == "bigint" || str == "numeric")
        return fft_integer;
	else if(str == "single precision" || str == "double precision")
		return fft_double;
    else if(str.find("time") != string::npos)
        return fft_date;
    else if(str == "character" || str == "character varying")
        return fft_string;
    else if(str == "boolean")
        return fft_boolean;

    return fft_string;
}

std::map<QCATFieldType,vector<shared_ptr<QCATField> > > QCATFieldManager::groupedByDataType() const
{
	auto all = this->vector();
	auto grp = std::map<QCATFieldType,std::vector<shared_ptr<QCATField> > >();

	for(auto item: all) {
		grp[item->type()].push_back(item);
	}

	return grp;
}

/*std::map<std::string,vector<shared_ptr<QCATField> > > QCATFieldManager::groupedByBinType() const
{
	auto all = this->vector();
	auto grp = std::map<std::string,std::vector<shared_ptr<QCATField> > >();

	for(auto item: all) {
		grp[typeid(item->bin().get()).name()].push_back(item);
	}

	return grp;
}*/
