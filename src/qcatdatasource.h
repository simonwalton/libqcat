#ifndef QCATDataSource_H
#define QCATDataSource_H

#include "libpq-fe.h"
#include <memory>
#include <string>
#include <list>

using namespace std;

#include "qcatfield.h"
#include "qcatfieldmanager.h"
#include "qcatpqresult.h"
#include "qcatfieldstats.h"

typedef shared_ptr<QCATPQResult> QCATDBResult;

class QCATDataSource
{
public:
    QCATDataSource(std::string connStr, std::string table);
    ~QCATDataSource();

    QCATDBResult unique(std::string field, int limit = -1);
    int totalRecords();

    static list<std::string> resultToList(QCATDBResult result, std::string field);

    shared_ptr<QCATField> fieldForName(std::string);
    shared_ptr<QCATFieldManager> fields() const;
	std::vector<shared_ptr<QCATFieldStats> > fieldStats() const;

    QCATDBResult executeSQL(std::string sql, bool* success = NULL) const;

	/*!
	 * \brief Execute a command a return the first row, first col result
	 * \param sql SQL to execute
	 * \param wrap If true, wraps sql param in SELECT (sql) FROM <this_table>
	 * \return First col, first row result
	 */
	std::string executeSQLSingleShot(std::string sql, bool wrap = false) const;

    std::string table() const;
	std::string tableSafe() const;
	std::string db() const;

	bool goodConnection() const;

private:
	void ensureFieldStatTable() const;

	bool m_goodConnection;
    shared_ptr<QCATFieldManager> m_fields;
    std::string m_table, m_db;
    PGconn* m_client;
};

#endif // QCATDataSource_H
