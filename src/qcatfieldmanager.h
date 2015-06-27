#ifndef FACASFIELDGROUP_H
#define FACASFIELDGROUP_H

#include <list>
#include <memory>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <typeinfo>
#include "qcatfield.h"

using namespace std;

class QCATFieldManager
{
public:
    QCATFieldManager(QCATDataSource* db);

    std::vector<shared_ptr<QCATField> > vector() const;
    std::map<std::string,shared_ptr<QCATField> > map() const;
//	std::map<std::string,std::vector<shared_ptr<QCATField> > > groupedByBinType() const;
	std::map<QCATFieldType,std::vector<shared_ptr<QCATField> > > groupedByDataType() const;

    int size() const;
    shared_ptr<QCATField> at(int idx) const;
    shared_ptr<QCATField> named(std::string name) const;

	static QCATFieldType fieldTypeFromStr(std::string str);

private:
    void scan(QCATDataSource* db);

    std::map<std::string,shared_ptr<QCATField> > m_fieldMap;
    std::vector<shared_ptr<QCATField> > m_fieldVector;
};

#endif // FACASFIELDGROUP_H
