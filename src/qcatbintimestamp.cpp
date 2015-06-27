#include "qcatbintimestamp.h"
#include <boost/lexical_cast.hpp>

const int removeSeconds = 250100000;

std::string QCATBinTimestamp::sqlAttrToBin(std::string str) const
{
    return safePad("CAST((((extract(epoch from " + str + ") - " + boost::lexical_cast<std::string>(removeSeconds) + ") / 60.0) / " + boost::lexical_cast<std::string>(binWidth()) + ") AS int)");
}

std::string QCATBinTimestamp::sqlValToBin(std::string val) const
{
	std::string totimestamp = "(to_timestamp('" + val + "','YYYY-MM-DD HH24:MI:SS'))";
    return safePad("CAST((((extract(epoch from " + totimestamp + ") - " + boost::lexical_cast<std::string>(removeSeconds) + ") / 60.0) / " + boost::lexical_cast<std::string>(binWidth()) + ") AS int)");
}

std::string QCATBinTimestamp::sqlBinToVal(std::string val) const
{
	std::string mult = safePad(boost::lexical_cast<std::string>(binWidth()) + " * " + val + " * 60");
	std::string add = boost::lexical_cast<std::string>(removeSeconds) + " (" + mult + ")";
	return "SELECT TIMESTAMP WITH TIME ZONE 'epoch' + (" + add + ") * INTERVAL '1 second'";
}

std::string QCATBinTimestamp::sqlFieldToBasicUnit(std::string field) const
{
	std::string totimestamp = "(to_timestamp(" + field + ",'YYYY-MM-DD HH24:MI:SS'))";
    return safePad("CAST((((extract(epoch from " + totimestamp + ") - " + boost::lexical_cast<std::string>(removeSeconds) + ") / 60.0)) AS int)");
}

std::string QCATBinTimestamp::sqlValToBasicUnit(std::string val) const
{
	return sqlFieldToBasicUnit("'" + val + "'");
}
