#ifndef FACASFIELDBINPASSTHROUGH_H
#define FACASFIELDBINPASSTHROUGH_H

#include "qcatfield.h"
#include "qcatbin.h"

class QCATBinPassthrough : public QCATBin
{
public:
    QCATBinPassthrough(QCATFieldType type)
        :QCATBin("","") {
        m_fieldType = type;
        switch(type) {
            case fft_integer:
			case fft_double:
                m_unit = "number"; m_unitPlural = "numbers"; break;
            case fft_string:
                m_unit = "string"; m_unitPlural = "strings"; break;
            case fft_boolean:
                m_unit = "boolean"; m_unitPlural = "booleans"; break;
			default:
				std::cerr << "QCATBinPassthrough() Unknown type" << std::endl;
        }
    }

    std::string sqlAttrToBin(std::string attr) const;
    std::string sqlValToBin(std::string val) const;
	std::string sqlBinToVal(std::string binval) const;
	std::string sqlFieldToBasicUnit(std::string field) const;
	std::string sqlValToBasicUnit(std::string field) const;
	bool isQuantitative() const { return false; } 

    virtual std::vector<QCATBinSuggestion> suggestions() const {
        return boost::assign::list_of<QCATBinSuggestion>
            (QCATBinSuggestion("<identity>",1));
    }

    virtual bool isPassthrough() const {
        return true;
    }
private:
    bool shouldQuoteConstants() const {
        return m_fieldType == fft_string;
    }

    QCATFieldType m_fieldType;
    bool m_shouldQuoteConstants;
};

#endif // FACASFIELDBINPASSTHROUGH_H
