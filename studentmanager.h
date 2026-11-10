#ifndef STUDENTMANAGER_H
#define STUDENTMANAGER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include "student.h"

// Manages ALL records: add, update, delete, navigate, and the CSV file.
class StudentManager
{
public:
    StudentManager();

    // ----- File handling -----
    bool loadFromFile();                 // false if the file cannot be read/created
    QString filePath() const;
    int skippedRows() const;             // damaged CSV rows ignored while loading

    // ----- Information -----
    int count() const;                   // number of records
    int currentIndex() const;            // -1 means "new record mode"
    bool hasCurrent() const;             // is a record currently selected?
    Student currentStudent() const;      // the selected record (empty if none)
    bool hasPrevious() const;
    bool hasNext() const;

    // ----- Navigation -----
    bool movePrevious();
    bool moveNext();
    void startNewRecord();               // selects nothing (new record mode)

    // ----- Operations (return false and set errorMessage() on failure) -----
    bool addStudent(const Student &student);
    bool updateStudent(const QString &originalRoll, const Student &updated);
    bool deleteStudent(const QString &originalRoll);

    bool rollNumberExists(const QString &roll, int ignoreIndex = -1) const;
    QString errorMessage() const;

private:
    int findIndexByRoll(const QString &roll) const;
    bool saveToFile();
    QString escapeField(const QString &field) const;
    QVector<QStringList> parseCsv(const QString &text) const;

    QVector<Student> m_students;   // all records, in the order they are stored
    int m_currentIndex;            // position of the displayed record, -1 = none
    QString m_filePath;
    QString m_lastError;
    int m_skippedRows;
};

#endif // STUDENTMANAGER_H
