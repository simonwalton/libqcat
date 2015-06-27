#include "qcatbinstrategy.h"
#include "qcatdatasource.h"
#include "qcatfieldstats.h"
#include "qcatattribute.h"
#include "qcatbin.h"

QCATBinStrategy::QCATBinStrategy() 
{

}

double QCATBinStrategyExact::width(QCATAttribute* attr) const
{
	return m_width;
}

double QCATBinStrategyDivide::width(QCATAttribute* attr) const
{
	if(!attr->bin()->isQuantitative())
		return 1.0;

	auto fs = attr->field()->stats();
	auto rangeSQL = "(" + fs.max().text + " - " + fs.min().text + ")"; 
	auto divSQL = rangeSQL + " / " +  boost::lexical_cast<std::string>(m_by);
	return boost::lexical_cast<double>(attr->db()->executeSQLSingleShot(divSQL, false));
}

double QCATBinStrategyDivideIfMore::width(QCATAttribute* attr) const
{
	if(!attr->bin()->isQuantitative())
		return 1.0;

	auto fs = attr->field()->stats();
	auto rangeSQL = "(" + fs.max().text + " - " + fs.min().text + ")"; 
	auto divSQL = "(" + rangeSQL + " / " +  boost::lexical_cast<std::string>(m_by) + ")";
	auto caseSQL = "(CASE WHEN " + rangeSQL + " >= "
		+ boost::lexical_cast<std::string>(m_moreThanWhat)
		+ " THEN " + divSQL 
		+ " ELSE " + boost::lexical_cast<std::string>(attr->bin()->binWidth()) + " END)";
	return boost::lexical_cast<double>(attr->db()->executeSQLSingleShot(caseSQL, false));
}

