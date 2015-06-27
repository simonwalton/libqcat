#include "qcatfield.h"
#include "qcatbin.h"
#include <boost/lexical_cast.hpp>

#include "qcatdatasource.h"
#include "qcatbinnumeric.h"
#include "qcatbintimestamp.h"
#include "qcatbinpassthrough.h"
#include "qcatfieldstats.h"
#include "qcatbinstats.h"

#ifdef QCAT_VERBOSE
#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>
#endif

QCATField::QCATField(QCATDataSource* db, std::string name, int index, QCATFieldType type)
{
    m_db = db;
    m_table = m_db->table();
    setName(name);
    setIndex(index);
    setType(type);

    initialise();
}

void QCATField::initialise()
{

}

/*void QCATField::executeBinStrategy()
{
	m_bin->setBinWidth(m_binStrategy->width(this));
}
*/


QCATFieldStats QCATField::stats()
{
    if(m_cachedStats.get() == NULL)
        m_cachedStats = shared_ptr<QCATFieldStats>(new QCATFieldStats(this, m_db));

    return *m_cachedStats;
}



/*void QCATField::ensureBinIndex() const
{
    if(m_bin->isPassthrough())
        return;

    try {
        std::string sqlCreate = "CREATE INDEX " + indexName() + " ON " + m_db->table() + "(" + sqlBinExpr(name()) + ")";
        bool success;
        m_db->executeSQL(sqlCreate, &success);
#ifdef QCAT_VERBOSE
        if(success)
			BOOST_LOG_TRIVIAL(info) << "Created bin index" << indexName() << std::endl;
#endif
    }
    catch(exception e) {
    }
}


bool QCATField::isBinCache() const
{
    return name().find("_bincache_") != std::string::npos;
}

bool QCATField::isOriginalField() const
{
    return !isBinCache();
}

std::string QCATField::binCacheColumnName() const
{
    return "_bincache_" + name();// + "_" + boost::lexical_cast<std::string>(m_binSize);
}*/

std::string QCATField::indexName() const
{
    return "idx_" + name() + "_binned";
}

/*std::string QCATField::nameBinned() const
{
    return name() + m_binPostfix;
}*/

std::string QCATField::safePad(std::string str)
{
    return " " + str + " ";
}

void QCATField::setName(std::string name)
{
    m_name = name;
}

std::string QCATField::name(bool quoted) const
{
    return quoted ? "'" + m_name + "'" : m_name;
}

void QCATField::setIndex(int idx)
{
    m_index = idx;
}

int QCATField::index() const
{
    return m_index;
}

void QCATField::setType(QCATFieldType type)
{
    m_type = type;
}

QCATFieldType QCATField::type() const
{
    return m_type;
}
