#include "qcatdatasource.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#define LIMITING_CONDITION " TRUE "
#define LOG_SQL 

#ifdef LOG_SQL
#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
namespace logging = boost::log;
#endif

boost::mutex db_mutex;

// we want to replace the default postgres notice handler to redirect to the log if necessary
static void CC_handle_notice(void *arg, const char *msg) 
{ 
	#ifdef LOG_SQL
		BOOST_LOG_TRIVIAL(info) << std::string(msg) << std::endl;
	#else
		std::cout << msg << std::endl;
	#endif
}

QCATDataSource::QCATDataSource(std::string connStr, std::string table)
    :m_table(table), m_goodConnection(false)
{
	m_client = PQconnectdb(connStr.c_str());
	if (PQstatus(m_client) == CONNECTION_BAD) {
		std::cerr << "*** QCATDataSource was unable to connect to the database with connection string " << connStr << std::endl;
		std::cerr.flush();
		return;
	}

#ifdef LOG_SQL
	logging::add_file_log(
		logging::keywords::file_name = "libqcat.log",
		logging::keywords::auto_flush = true,
		logging::keywords::format = "[%TimeStamp%]: %Message%"
		);
	PQsetNoticeProcessor(m_client, CC_handle_notice, NULL);
#endif

	m_fields = shared_ptr<QCATFieldManager>(new QCATFieldManager(this));
	m_goodConnection = m_fields->vector().size() != 0;	

	if(!m_goodConnection) {
		std::cerr << "*** QCATDataSource: Unable to obtain any fields from table " << table << std::endl;
		return;
	}
}

QCATDataSource::~QCATDataSource()
{
	PQfinish(m_client);
}

bool QCATDataSource::goodConnection() const
{
	return m_goodConnection;
}

std::string QCATDataSource::table() const
{
    return m_table;
}

std::string QCATDataSource::tableSafe() const
{
	return "\"" + m_table + "\"";
}

std::string QCATDataSource::db() const
{
	return m_db;
}

std::string QCATDataSource::executeSQLSingleShot(std::string sql, bool wrap) const
{
	bool success = false;
	if(wrap)
		sql = "SELECT (" + sql + ") FROM " + table();
	auto result = this->executeSQL(sql, &success);

	if(success)
		return std::string(result->get(0,0));
	else {
		std::cerr << "*** Problem with executing SQL as single shot" << std::endl;
		return "";
	}
}

QCATDBResult QCATDataSource::executeSQL(std::string sql, bool* success) const
{
    boost::mutex::scoped_lock lock(db_mutex);
	PGresult* r;
    try {
#ifdef LOG_SQL
		BOOST_LOG_TRIVIAL(info) << "QCATDataSource::executeSQL running SQL:" << endl;
		BOOST_LOG_TRIVIAL(info) << "\t" << sql << endl;
#endif
        r = PQexec(m_client, sql.c_str());
		auto rs = PQresultStatus(r);

        if(success != NULL)
            *success = rs == PGRES_TUPLES_OK 
			   || rs == PGRES_EMPTY_QUERY
			   || rs == PGRES_COMMAND_OK;
    }
    catch(exception e) {
#ifdef LOG_SQL
		BOOST_LOG_TRIVIAL(error) << "*******" << endl
			<< "ERROR Failed to execute SQL: " << endl << sql << endl 
			<< "*******" << endl;
#endif
        if(success != NULL)
            *success = false;
    }
    return shared_ptr<QCATPQResult>(new QCATPQResult(r));
}

shared_ptr<QCATField> QCATDataSource::fieldForName(std::string name)
{
    try {
        return m_fields->named(name);
    }
    catch(exception e) {
        std::cerr << "*** QCATDataSource::fieldForName: No such field name " << name << endl;
        return shared_ptr<QCATField>();
    }
}

shared_ptr<QCATFieldManager> QCATDataSource::fields() const
{
    return m_fields;
}

std::vector<shared_ptr<QCATFieldStats> > QCATDataSource::fieldStats() const
{
	ensureFieldStatTable();

	std::vector<shared_ptr<QCATFieldStats> > stats;
	for(auto field: m_fields->vector()) {
		stats.push_back(shared_ptr<QCATFieldStats>(new QCATFieldStats(field.get(), this)));
	}
	return stats;	
}

void QCATDataSource::ensureFieldStatTable() const
{
	std::string sql = "CREATE TABLE " + table() + "_stats ";
	sql += "(  _unique double precision,	\
		  special character varying,		\
		  last_compiled date,				\
		  field character varying NOT NULL,	\
		  avg text,							\
		  min text,							\
		  max text,							\
		  stddev text							\
		)	\
		WITH (			\
		  OIDS=FALSE	\
		);	\
		ALTER TABLE " + table() + "_stats		\
		  OWNER TO postgres;		\
		  ";
	executeSQL(sql);
}

int QCATDataSource::totalRecords()
{
    int result = -1;
    PGresult* r = PQexec(m_client, std::string("SELECT COUNT(*) FROM " + m_table).c_str());
	char* val = PQgetvalue(r,0,0);
    return boost::lexical_cast<int>(val);
}

QCATDBResult QCATDataSource::unique(std::string field, int limit)
{
    PGresult* r = PQexec(m_client, std::string("SELECT DISTINCT(" + field + ") FROM " + m_table + " WHERE " + LIMITING_CONDITION + " ORDER BY " + field + (limit == -1 ? "" : " LIMIT " + boost::lexical_cast<std::string>(limit))).c_str());
	return shared_ptr<QCATPQResult>(new QCATPQResult(r));
}

std::list<std::string> QCATDataSource::resultToList(QCATDBResult result, std::string field)
{
    std::list<std::string> resultList;
    for(int i=0;i<result->nrows();i++)
        resultList.push_back(std::string(result->get(i,field)));
    return resultList;
}


