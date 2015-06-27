
#include "qcatattribute.h"
#include "qcatdatasource.h"
#include "qcatbinnumeric.h"
#include "qcatbintimestamp.h"
#include "qcatbinpassthrough.h"
#include "qcatfieldstats.h"
#include "qcatbinstats.h"

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>

QCATAttribute::QCATAttribute(shared_ptr<QCATField> field)
{
	m_field = field;
	if(m_field)
		setDefaultBin();
	else
		std::cerr << "*** QCATAttribute::QCATAttribute: got a NULL field" << std::endl;
}

QCATBinStats QCATAttribute::binStats()
{
    return QCATBinStats(this);
}

bool QCATAttribute::OK() const
{
	return (bool)m_field;
}

std::vector<std::string> QCATAttribute::uniques()
{
	std::string sql = "SELECT UNIQUE " + m_bin->sqlAttrToBin(m_field->name()) + " FROM " + m_field->db()->table();
	auto rows = m_field->db()->executeSQL(sql);

	auto result = std::vector<std::string>(rows->nrows());
	for(int i=0;rows->nrows();i++) {
		result[i] = rows->get(i,0);
	}

	return result;
}

void QCATAttribute::setDefaultBin()
{
	switch(m_field->type()) {
		case fft_integer:
	        m_bin = unique_ptr<QCATBin>(new QCATBinInteger()); break;
		case fft_double:
			m_bin = unique_ptr<QCATBin>(new QCATBinDouble()); break;
		case fft_date:
		case fft_time:
			m_bin = unique_ptr<QCATBin>(new QCATBinTimestamp()); break;
		default:
			m_bin = unique_ptr<QCATBin>(new QCATBinPassthrough(m_field->type()));
	}

	BOOST_LOG_TRIVIAL(info) << "Set default bin of type " << m_bin->unit() 
		<< " for attribute " << m_field->name() << " and isPassthrough: " 
		<< m_bin->isPassthrough() << std::endl;
}

void QCATAttribute::applyBinStrategy()
{
	m_bin->setBinWidth(m_binStrategy->width(this));
}

void QCATAttribute::setBinStrategy(shared_ptr<QCATBinStrategy> bs)
{
	m_binStrategy = bs;
	applyBinStrategy();
}

shared_ptr<QCATBinStrategy> QCATAttribute::binStrategy() const
{
	return m_binStrategy;
}

bool QCATAttribute::hasBinStrategy() const
{
   return m_binStrategy == make_shared<QCATBinStrategy>();
}

std::string QCATAttribute::sqlSelect() const
{
    return sql(true);
}

std::string QCATAttribute::sqlHash() const
{
    return sql(false);
}

std::string QCATAttribute::sqlWhere() const
{
    return sql(false);
}

std::string QCATAttribute::sqlNoAS() const
{
	return sql(false);
}

std::string QCATAttribute::sqlUnbinned() const
{
	return m_field->name();
}

std::string QCATAttribute::sql(bool withAS) const
{
    return m_bin->sqlAttrToBin(m_field->name()) + (withAS ? m_field->name() : "");
}

std::string QCATAttribute::sqlBinExpr(std::string str, std::string AS) const
{
    return m_bin->sqlAttrToBin(str) + (!AS.empty() ? " AS " + AS : " ");
}

std::string QCATAttribute::sqlBinForValue(std::string constant) const
{
    return m_bin->sqlValToBin(constant);
}

std::string QCATAttribute::sqlForVal(std::string value) const
{
    return m_bin->sqlValToBin(value);
}

/*void QCATAttribute::ensureBinCacheColumn() const
{
    if(m_bin->isPassthrough())
        return;

    try {
        std::string sqlCreate = "ALTER TABLE " + m_db->table() + " ADD COLUMN " + binCacheColumnName() + " smallint";
        bool success;
        QCATDBResult result = m_db->executeSQL(sqlCreate,&success);

        if(success) {
#ifdef QCAT_VERBOSE
            BOOST_LOG_TRIVIAL(info) << "Creating bin cache " << binCacheColumnName() << std::endl;
#endif
            // if executing the above doesn't throw, then we can go ahead and fill the new column in
            std::string sqlFill = "UPDATE " + m_db->table() + " SET " + binCacheColumnName() + " = " + sqlBinExpr(name());
            m_db->executeSQL(sqlFill);
        }
    }
    catch(exception e) {

    }
}*/


