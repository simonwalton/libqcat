#include "qcatcondition.h"
#include <iostream>

QCATCondition::QCATCondition(shared_ptr<QCATAttribute> lhs)
{
    m_rhs_is_field = false;
    setLHS(lhs);
}

QCATCondition::QCATCondition(shared_ptr<QCATAttribute> lhs, QCATOp op, shared_ptr<QCATAttribute> rhs)
    :m_op(op)
{
    setLHS(lhs);
    setRHS(rhs);
}
	
QCATCondition::QCATCondition(shared_ptr<QCATAttribute> lhs, shared_ptr<QCATAttribute> rhs)
    :m_op(fop_equal)
{
    setLHS(lhs);
    setRHS(rhs);
}

QCATCondition::QCATCondition(shared_ptr<QCATAttribute> lhs, QCATOp op,std::string rhs)
    :m_op(op)
{
    setLHS(lhs);
    setRHS(rhs);
}

QCATCondition::QCATCondition(shared_ptr<QCATAttribute> lhs, QCATOp op, std::string rhsa, std::string rhsb)
    :m_op(op)
{
    setLHS(lhs);
    setRHS(rhsa,rhsb);
}

bool QCATCondition::isComplete() const
{
    return (m_rhs_is_field ? !m_rhs_field->field()->name().empty() : !m_rhs_constant_a.empty());
}

std::string QCATCondition::RHSValue() const
{
    return m_rhs_constant_a;
}

std::string QCATCondition::opStr() const
{
    return strForOp(m_op);
}

void QCATCondition::setLHS(shared_ptr<QCATAttribute> f)
{
    m_lhs = f;
}

void QCATCondition::setRHS(shared_ptr<QCATAttribute> f)
{
    m_rhs_is_field = true;
    m_rhs_field = f;
}

void QCATCondition::setRHS(std::string c)
{
    m_rhs_is_field = false;
    m_rhs_constant_a = c;
}

void QCATCondition::setRHS(std::string c, std::string d)
{
    m_rhs_is_field = false;
    m_rhs_constant_a = c;
    m_rhs_constant_b = d;
}

shared_ptr<QCATAttribute> QCATCondition::LHS() const
{
    return m_lhs;
}

QCATOp QCATCondition::op() const
{
    return m_op;
}

std::string QCATCondition::sql()
{
    // TODO For the time being, having a field as RHS is unsupported
    if(m_rhs_is_field) {
        throw exception();
    }
	std::string str = m_lhs->sqlWhere() + QCATCondition::strForOp(m_op);

	if(m_op == fop_between) {
		str += m_lhs->sqlBinForValue(m_rhs_constant_a) + " AND " + m_lhs->sqlBinForValue(m_rhs_constant_b);
	}
	else {
	    str += m_lhs->sqlBinForValue(m_rhs_constant_a);
	}

    return pad(str);
}

std::string QCATCondition::pad(std::string str)
{
    return " " + str + " ";
}

std::string QCATCondition::strForOp(QCATOp op)
{
	// TODO: only equals and between supported right now
    switch(op) {
        case fop_equal: return " = ";
		case fop_between: return " BETWEEN ";
        default: return "";
    }
}

std::string QCATCondition::toString()
{
    return this->sql();
}
