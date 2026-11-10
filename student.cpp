#include "student.h"

Student::Student()
    : m_gpa(0.0)
{
}

Student::Student(const QString &name, const QString &rollNumber,
                 const QString &fatherName, const QString &caste,
                 const QString &district, double gpa)
    : m_name(name),
      m_rollNumber(rollNumber),
      m_fatherName(fatherName),
      m_caste(caste),
      m_district(district),
      m_gpa(gpa)
{
}

QString Student::name() const       { return m_name; }
QString Student::rollNumber() const { return m_rollNumber; }
QString Student::fatherName() const { return m_fatherName; }
QString Student::caste() const      { return m_caste; }
QString Student::district() const   { return m_district; }
double Student::gpa() const         { return m_gpa; }

void Student::setName(const QString &name)             { m_name = name; }
void Student::setRollNumber(const QString &rollNumber) { m_rollNumber = rollNumber; }
void Student::setFatherName(const QString &fatherName) { m_fatherName = fatherName; }
void Student::setCaste(const QString &caste)           { m_caste = caste; }
void Student::setDistrict(const QString &district)     { m_district = district; }
void Student::setGpa(double gpa)                       { m_gpa = gpa; }
