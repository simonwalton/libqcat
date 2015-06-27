#ifndef FACASFIELDBINNUMERIC_H
#define FACASFIELDBINNUMERIC_H

#include "qcatbin.h"

template<class T>
class QCATBinNumeric : public QCATBin
{
public:
    QCATBinNumeric()
        :QCATBin("number","numbers") {
        m_width = 1.0;
    }

    std::string sqlAttrToBin(std::string attr) const;
    std::string sqlValToBin(std::string val) const;
	std::string sqlBinToVal(std::string binval) const;
	std::string sqlFieldToBasicUnit(std::string field) const;
	std::string sqlValToBasicUnit(std::string field) const;
	bool isQuantitative() const { return true; } 

    virtual std::vector<QCATBinSuggestion> suggestions() const {
        return boost::assign::list_of<QCATBinSuggestion>
            (QCATBinSuggestion("1",1))
            (QCATBinSuggestion("2",2))
            (QCATBinSuggestion("5",5))
            (QCATBinSuggestion("10",10));
    }
};

typedef QCATBinNumeric<double> QCATBinDouble;
typedef QCATBinNumeric<int> QCATBinInteger;

#endif // FACASFIELDBINNUMERIC_H
