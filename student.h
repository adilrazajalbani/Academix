#ifndef STUDENT_H
#define STUDENT_H

#include <QString>

// Represents ONE student record.
class Student
{
public:
    Student();   // creates an empty student
    Student(const QString &name, const QString &rollNumber,
            const QString &fatherName, const QString &caste,
            const QString &district, double gpa);

    // Getters (read the private data)
    QString name() const;
    QString rollNumber() const;
    QString fatherName() const;
    QString caste() const;
    QString district() const;
    double gpa() const;

    // Setters (change the private data)
    void setName(const QString &name);
    void setRollNumber(const QString &rollNumber);
    void setFatherName(const QString &fatherName);
    void setCaste(const QString &caste);
    void setDistrict(const QString &district);
    void setGpa(double gpa);

private:
    QString m_name;
    QString m_rollNumber;
    QString m_fatherName;
    QString m_caste;
    QString m_district;
    double m_gpa;
};

#endif // STUDENT_H
