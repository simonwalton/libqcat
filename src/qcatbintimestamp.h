#ifndef FACASFIELDBINTIMESTAMP_H
#define FACASFIELDBINTIMESTAMP_H

#include "qcatbin.h"

class QCATBinTimestamp : public QCATBin
{
public:
    QCATBinTimestamp()
        :QCATBin("minute","minutes") {
        m_width = 15;
    }

    std::string sqlAttrToBin(std::string attr) const;
    std::string sqlValToBin(std::string val) const;
	std::string sqlBinToVal(std::string binval) const;
	std::string sqlFieldToBasicUnit(std::string field) const;
	std::string sqlValToBasicUnit(std::string field) const;
	bool isQuantitative() const { return true; } 

    virtual std::vector<QCATBinSuggestion> suggestions() const {
        return boost::assign::list_of<QCATBinSuggestion>
            (QCATBinSuggestion("1 minute",1))
            (QCATBinSuggestion("5 minutes",5))
            (QCATBinSuggestion("10 minutes",10))
            (QCATBinSuggestion("15 minutes",15))
            (QCATBinSuggestion("30 minutes",30))
            (QCATBinSuggestion("1 hour",60))
            (QCATBinSuggestion("6 hours",60*6))
            (QCATBinSuggestion("12 hours",60*12))
            (QCATBinSuggestion("1 day",60*24));
    }

    std::string currentBinDescription() const {
        return QCATBin::currentBinDescription(); // TODO: date formatting
    }
};

#endif // FACASFIELDBINTIMESTAMP_H
