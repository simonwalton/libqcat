#include "qcatbinpassthrough.h"

std::string QCATBinPassthrough::sqlAttrToBin(std::string str) const
{
    return safePad(str);
}

std::string QCATBinPassthrough::sqlValToBin(std::string val) const
{
    return shouldQuoteConstants() ? safePad("'" + val+ "'") : safePad(val);
}

std::string QCATBinPassthrough::sqlBinToVal(std::string val) const
{
	return sqlValToBin(val); 
}

std::string QCATBinPassthrough::sqlFieldToBasicUnit(std::string field) const
{
	return safePad("CAST(" + boost::lexical_cast<std::string>(field) + " as int)");
}

std::string QCATBinPassthrough::sqlValToBasicUnit(std::string val) const
{
	return sqlFieldToBasicUnit(val); 
}
