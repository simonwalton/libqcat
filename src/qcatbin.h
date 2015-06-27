#ifndef FACASFIELDBIN_H
#define FACASFIELDBIN_H

#include <string>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

struct QCATBinSuggestion
{
    QCATBinSuggestion(std::string description, int width)
        :description(description),width(width) {}
    std::string description;
    int width;
};

class QCATBin
{
public:
    QCATBin(std::string unit, std::string unitPlural)
        :m_width(1),m_unit(unit),m_unitPlural(unitPlural) {}

    /*!
     * \brief Generate SQL for converting an existing attribute (column) to the bin
     * \param attr Valid name of attribute
     * \return SQL string
     */
    virtual std::string sqlAttrToBin(std::string attr) const = 0;

    /*!
     * \brief Generate SQL for converting an a constant value to binned value
     * \param attr Valid name of attribute
     * \return SQL string
     */
    virtual std::string sqlValToBin(std::string val) const = 0;

	/*!
	 * \brief Reverse of above - generates SQL that takes a binned value and provides back the first 'real' value matching the bin (its min boundary)
	 * \param binval The binned value
	 */
	virtual std::string sqlBinToVal(std::string binval) const = 0;

	/*!
	 * \brief Converts the given field this bin type's basic unit (integer, minute, etc)
	 * \return SQL to convert field to basic unit
	 */
	virtual std::string sqlFieldToBasicUnit(std::string field) const = 0;

	/*!
	 * \brief Converts the given arbitrary value to this bin type's basic unit (integer, minute, etc)
	 * \return SQL to convert value to basic unit
	 */
	virtual std::string sqlValToBasicUnit(std::string val) const = 0;

    /*!
     * \brief Is this just a passthrough bin? (I.E. just the attribute itself)
     */
    virtual bool isPassthrough() const {
        return false;
    }

    /*!
      * \brief Provides a list of available bins for display to user
      * \return Vector of QCATBinSuggestion objects
      */
    virtual std::vector<QCATBinSuggestion> suggestions() const = 0;/* {
        return boost::assign::list_of<QCATBinSuggestion>(
            QCATBinSuggestion("Identity",1));
    }*/

	/*!
	 * \return True if this bin type can be divided into smaller chunks (numeric, timestamp types)
	 */
	virtual bool isQuantitative() const = 0;
    
	/*!
     * \brief Gives the value of the currently chosen bin
     * \return
     */
    virtual double binWidth() const {
        return m_width;
    }

    void setBinWidth(double width) {
        m_width = width;
    }

    /*!
    * \brief This bin's unit with correct pluralisation (e.g. minute/minutes, etc)
    */
    std::string unit() const {
        return m_width == 1 ? m_unit : m_unitPlural;
    }

    /*!
     * \brief Gives the description of the currently chosen bin
     * \return
     */
    virtual std::string currentBinDescription() const {
        return boost::lexical_cast<std::string>(m_width) + " " + unit();
    }

protected:


    /*!
    * \brief This bin's unit (e.g. minutes, etc)
    */
    std::string unitSingular() const {
        return m_unit;
    }

    /*!
    * \brief The plural version of this bin's unit
    */
    std::string unitPlural() const {
        return m_unitPlural;
    }
    /*!
     * \brief Returns a padded string suitable for concatenating into SQL queries
     */
    static std::string safePad(std::string);

    double m_width;
    std::string m_unit, m_unitPlural;
};

#endif // FACASFIELDBIN_H
