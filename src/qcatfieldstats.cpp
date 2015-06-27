#include "qcatfieldstats.h"
#include "qcatfield.h"
#include "qcatdatasource.h"
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#define ACCEPTABLE_AGE_DAYS 30
#define ENABLE_CACHE true

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>

QCATFieldStatResult::QCATFieldStatResult(std::string text)
    :text(text) {
    try {
        double n = boost::lexical_cast<double>(text);
        this->text = boost::str(boost::format("%.2f") % n);
        this->numeric = n;
        this->reliable_numeric = true;
    }
    catch(boost::bad_lexical_cast e) {
        this->numeric = 0;
        this->reliable_numeric = false;
    }
}

QCATFieldStats::QCATFieldStats()
{
}

QCATFieldStats::QCATFieldStats(const QCATField* field, const QCATDataSource* db)
	:m_field((QCATField*)field)
{
	if(ENABLE_CACHE) {
		if(!cacheAcceptable(field,db)) {
			compileStats(field, db);
			ensureCache(field, db);
			saveToCache(field, db);
		}
		else {
			compileFromCache(field, db);
		}
	}
	else {
		compileStats(field, db);
		saveToCache(field, db);
	}
}

void QCATFieldStats::ensureCache(const QCATField* f, const QCATDataSource* db)
{
    bool success;
    db->executeSQL("INSERT INTO " + db->table() + "_stats(field,last_compiled) VALUES('" + f->name() + "','01-01-3000')",&success);
}

bool QCATFieldStats::cacheAcceptable(const QCATField* f, const QCATDataSource* db) const
{
    bool success;
	BOOST_LOG_TRIVIAL(info) << "Is cache acceptable" << endl;
    double age = executeSingleShot<double>("select extract(epoch from last_compiled - now()) / 86400 from " + db->table() + "_stats WHERE field = '" + f->name() + "'", db, &success);
	BOOST_LOG_TRIVIAL(info) << "Age " << age;
	BOOST_LOG_TRIVIAL(info) << "Success " << success;
    return (age <= ACCEPTABLE_AGE_DAYS) && success;
}

void QCATFieldStats::compileFromCache(const QCATField* f, const QCATDataSource* db)
{
    bool success;
    auto result = db->executeSQL("SELECT avg,min,max,stddev,_unique,special FROM " + db->table() + "_stats WHERE field = '" + f->name() + "'", &success);

#define getr(name) result->hasCol(name) ? std::string(result->get(0,name)) : "-1"

    m_avg = getr("avg");
    m_min = getr("min");
    m_max = getr("max");
    m_stddev = getr("stddev");
	m_unique = result->getInt(0,"_unique");
    m_special = getr("special");
}

void QCATFieldStats::saveToCache(const QCATField* f, const QCATDataSource* db)
{
    bool success;
    db->executeSQL("UPDATE " + db->table() + "_stats SET(avg,min,max,stddev,_unique,special,last_compiled) = ('" +
                   m_avg.text + "','" + m_min.text + "','" + m_max.text + "','" + m_stddev.text + "'," +
                   boost::lexical_cast<std::string>(m_unique) + ",'" + m_special + "',now()) WHERE field = '" + f->name() + "'");
}

void QCATFieldStats::compileStats(const QCATField* field, const QCATDataSource *db)
{
    const std::string fn = field->name();
    const std::string tn = db->table();
    bool success;
    m_unique = executeSingleShot<long>("SELECT COUNT(DISTINCT " + fn + ") AS result FROM " + tn, db, &success);
    m_min = runStat("SELECT MIN(" + fn + ") AS result FROM " + tn, db);
    m_max = runStat("SELECT MAX(" + fn + ") AS result FROM " + tn, db);
    m_stddev = runStat("SELECT STDDEV(" + fn + ") AS result FROM " + tn, db);
    m_avg = runStat("SELECT AVG(" + fn + ") AS result FROM " + tn, db);

    switch(field->type()) {
        case fft_string:
            m_special = "Average string length: " + boost::lexical_cast<std::string>(executeSingleShot<double>("SELECT AVG(LENGTH(" + fn +")) FROM " + tn,db,&success));
            m_special += "; Examples: " + executeToCommaSepList("SELECT DISTINCT " + fn + " FROM " + tn + " ORDER BY " + fn + " LIMIT 5",db);
            break;
        case fft_boolean:
			{
            long t = boost::lexical_cast<long>(executeSingleShot<long>("SELECT COUNT(" + fn + ") FROM " + tn + " WHERE " + fn + " = true",db,&success));
            long f = boost::lexical_cast<long>(executeSingleShot<long>("SELECT COUNT(" + fn + ")) FROM " + tn + " WHERE " + fn + " = false",db,&success));
            double ratio = t/(double)f;
            m_special = std::string("True/False ratio: ") + (boost::str(boost::format("%.2f") % ratio));
            break;
			}
		default:
			m_special = "";
    }
}

std::string QCATFieldStats::executeToCommaSepList(std::string str, const QCATDataSource* db) const
{
    bool success;
    auto result = db->executeSQL(str, &success);
    if(success) {
        std::string resultStr;
        for(int i=0;i<result->ncols();i++)
            resultStr += std::string(result->get(0,i)) + ", ";
        return resultStr.substr(0,resultStr.size()-2);
    }
    else return "";
}

QCATFieldStatResult QCATFieldStats::runStat(std::string sql, const QCATDataSource *db) const
{
    bool success;
    return QCATFieldStatResult(executeSingleShot<std::string>(sql,db,&success));
}

template<class T>
T QCATFieldStats::executeSingleShot(std::string str, const QCATDataSource *db, bool *success) const
{
    T val;
    auto result = db->executeSQL(str,success);
	if(success) *success = true;
    try {
		if(result->ncols() > 0 && result->nrows() > 0)
			val = boost::lexical_cast<T>(result->get(0,0));
		else {
			if(success) *success = false;
			return 0;
		}
    }
    catch(exception e) {
		if(success) *success = false;
        return 0;
    }
	return val;
}

template<>
std::string QCATFieldStats::executeSingleShot<std::string>(std::string str, const QCATDataSource *db, bool *success) const
{
    std::string val;
    auto result = db->executeSQL(str,success);
    if(*success) {
        return std::string(result->get(0,"result"));
    }
    else return "";
}
