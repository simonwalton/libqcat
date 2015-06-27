#ifndef FACASCONDITIONOP_H
#define FACASCONDITIONOP_H

#include <string>
#include <memory>
using namespace std;

#include "qcatattribute.h"

enum QCATConditionOp
{
    fop_equal = 0,
    fop_less = 1,
    fop_less_equal = 2,
    fop_more = 3,
    fop_more_equal = 4,
	fop_between = 5
};

typedef enum QCATConditionOp QCATOp;

/*!
 * \brief The QCATCondition class
 */
class QCATCondition
{
public:
    QCATCondition() {}
    QCATCondition(shared_ptr<QCATAttribute> lhs);
    QCATCondition(shared_ptr<QCATAttribute> lhs, shared_ptr<QCATAttribute> rhs);
    QCATCondition(shared_ptr<QCATAttribute> lhs, QCATOp op, shared_ptr<QCATAttribute> rhs);
    QCATCondition(shared_ptr<QCATAttribute> lhs, QCATOp op, std::string rhs);
    QCATCondition(shared_ptr<QCATAttribute> lhs, QCATOp op, std::string rhsa, std::string rhsb);

    void setLHS(shared_ptr<QCATAttribute>);
    void setRHS(shared_ptr<QCATAttribute>);
    void setRHS(std::string);
    void setRHS(std::string, std::string);

    shared_ptr<QCATAttribute> LHS() const;
    QCATOp op() const;

    std::string RHSValue() const;
    std::string opStr() const;

    /*!
     * \brief Returns true if this condition is complete with an RHS value
     */
    bool isComplete() const;

    /*!
     * \brief Provides valid SQL snippet for this condition
     * \return Conditional string; i.e. a < b
     */
    std::string sql();

    /*!
     * \brief Provides a readable description of this condition
     */
    std::string toString();

private:
    static std::string strForOp(QCATOp);
    static std::string pad(std::string str);

    QCATOp m_op;
    shared_ptr<QCATAttribute> m_lhs;
    shared_ptr<QCATAttribute> m_rhs_field;
    bool m_rhs_is_field;
    std::string m_rhs_constant_a, m_rhs_constant_b;
};

#endif // FACASCONDITIONOP_H
