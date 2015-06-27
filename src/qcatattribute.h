#ifndef QCATATTRIBUTE_H
#define QCATATTRIBUTE_H

#include <memory>
#include "qcatfield.h"

class QCATBinStrategy;
class QCATBin;

/*!
 * A QCATAttribute represents a field from a data source instantiated into a QCAT
 */
class QCATAttribute
{
public:
	/*!
	 * Construct from an existing field
	 */
	QCATAttribute(shared_ptr<QCATField>);

	bool hasBinStrategy() const;
	void setBinStrategy(shared_ptr<QCATBinStrategy>);
	shared_ptr<QCATBinStrategy> binStrategy() const;

	shared_ptr<QCATField> field() const { return m_field; }
	shared_ptr<QCATBin> bin() const { return m_bin; }
	std::string name() const { return m_field->name(); }
	QCATDataSource* const db() { return m_field->db(); }

	QCATBinStats binStats();

	/*!
	 * \brief For this field and its bin specification, get a list of binned values within its distribution
	 */
	std::vector<std::string> uniques();

  /*!
     * \brief Generates SQL for querying this field. If the bin size is -1 (default), then this simply is the name of the field.
     * Otherwise, the function generates correct histogram bin mapping for the field's datatype. If the AS is not specified and
     * binning is enabled, the AS will be set to the field name (without binning), and the field name appended with "_binned" (with binning).
     * \return SQL substring representing the field.
     */
    std::string sqlSelect() const;
    std::string sqlHash() const;
    std::string sqlWhere() const;
	std::string sqlNoAS() const;
	std::string sqlUnbinned() const;

    /*!
     * \brief Generates SQL for binning an arbitrary constant value
     */
    std::string sqlBinForValue(std::string) const;


   	std::string sql(bool withAS) const;

    /*!
     * \brief Generates the SQL expression representing the bin
     */
    std::string sqlBinExpr(std::string str, std::string AS = std::string()) const;

    /*!
     * \brief Provides a SQL-valid expression for a value of this field (i.e. quoted if date, string, etc)
     * If the field's type requires conversion from string (i.e. a timestamp type) then the function provides
     * this conversion too.
     */
    std::string sqlForVal(std::string) const;

    /*!
     * \brief Returns a padded string suitable for concatenating into SQL queries
     */
    static std::string safePad(std::string);

    /*!
     * \brief Quotes a string for SQL literals
     */

    std::string sqlBinForStr(std::string) const;

    /*!
     * \brief Set up default bin values for the datatype
     */
    void setDefaultBin();

	bool OK() const;


private:
	void applyBinStrategy();

	shared_ptr<QCATBinStrategy> m_binStrategy;
	shared_ptr<QCATBin> m_bin;
	shared_ptr<QCATField> m_field;
};

#endif
