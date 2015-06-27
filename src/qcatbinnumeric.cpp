#include "qcatbinnumeric.h"
#include <boost/lexical_cast.hpp>

template class QCATBinNumeric<int>;
template class QCATBinNumeric<double>;

// TODO: I'm worried about overflow for numeric types here... should ideally be using longs

template<class T>
std::string QCATBinNumeric<T>::sqlAttrToBin(std::string str) const
{
    return safePad("CAST(" + str + " / " + boost::lexical_cast<std::string>(binWidth())  + " AS int)");
}

template<class T>
std::string QCATBinNumeric<T>::sqlValToBin(std::string val) const
{
	return safePad("CAST(" + boost::lexical_cast<std::string>(val) + " / " +
		boost::lexical_cast<std::string>(binWidth()) + " as int)");

}

template<class T>
std::string QCATBinNumeric<T>::sqlBinToVal(std::string val) const
{
	return safePad("CAST(" + boost::lexical_cast<std::string>(val) + " * " +
		boost::lexical_cast<std::string>(binWidth()) + " as int)");
}

template<class T>
std::string QCATBinNumeric<T>::sqlFieldToBasicUnit(std::string field) const
{
	return safePad("CAST(" + boost::lexical_cast<std::string>(field) + " as int)");
}

template<class T>
std::string QCATBinNumeric<T>::sqlValToBasicUnit(std::string val) const
{
	return sqlFieldToBasicUnit(val); 
}
