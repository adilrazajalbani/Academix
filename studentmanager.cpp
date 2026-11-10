#include "studentmanager.h"

#include <QCoreApplication>
#include <QFile>
#include <QStringConverter>
#include <QTextStream>

StudentManager::StudentManager()
    : m_currentIndex(-1),
      m_skippedRows(0)
{
    // Reliable file location: the project folder (set in CMakeLists.txt).
    // If that is not available, use the folder of the .exe instead.
#ifdef ACADEMIX_DATA_DIR
    m_filePath = QString(ACADEMIX_DATA_DIR) + "/students.csv";
#else
    m_filePath = QCoreApplication::applicationDirPath() + "/students.csv";
#endif
}

// ===================== Information =====================

QString StudentManager::filePath() const   { return m_filePath; }
int StudentManager::skippedRows() const    { return m_skippedRows; }
int StudentManager::count() const          { return m_students.size(); }
int StudentManager::currentIndex() const   { return m_currentIndex; }
QString StudentManager::errorMessage() const { return m_lastError; }

bool StudentManager::hasCurrent() const
{
    return m_currentIndex >= 0 && m_currentIndex < m_students.size();
}

Student StudentManager::currentStudent() const
{
    if (hasCurrent())
        return m_students[m_currentIndex];
    return Student();
}

// The empty "new record" form counts as the position AFTER the last record.
// So from the empty form, Previous shows the last record and Next is disabled.
bool StudentManager::hasPrevious() const
{
    if (m_students.isEmpty())
        return false;
    if (!hasCurrent())
        return true;               // empty form -> can go back to the last record
    return m_currentIndex > 0;
}

bool StudentManager::hasNext() const
{
    if (!hasCurrent())
        return false;              // empty form or no records: nothing after it
    return m_currentIndex < m_students.size() - 1;
}

// ===================== Navigation =====================

bool StudentManager::movePrevious()
{
    if (!hasPrevious()) {
        m_lastError = "There is no previous record.";
        return false;
    }
    if (!hasCurrent())
        m_currentIndex = m_students.size() - 1;
    else
        m_currentIndex--;
    return true;
}

bool StudentManager::moveNext()
{
    if (!hasNext()) {
        m_lastError = "There is no next record.";
        return false;
    }
    m_currentIndex++;
    return true;
}

void StudentManager::startNewRecord()
{
    m_currentIndex = -1;
}

// ===================== Roll number checks =====================

int StudentManager::findIndexByRoll(const QString &roll) const
{
    for (int i = 0; i < m_students.size(); ++i) {
        if (QString::compare(m_students[i].rollNumber().trimmed(),
                             roll.trimmed(), Qt::CaseInsensitive) == 0)
            return i;
    }
    return -1;
}

bool StudentManager::rollNumberExists(const QString &roll, int ignoreIndex) const
{
    int index = findIndexByRoll(roll);
    return index != -1 && index != ignoreIndex;
}

// ===================== Add / Update / Delete =====================

bool StudentManager::addStudent(const Student &student)
{
    if (rollNumberExists(student.rollNumber())) {
        m_lastError = QString("Roll Number \"%1\" already exists.\n"
                              "Every record must have a unique Roll Number.")
                          .arg(student.rollNumber());
        return false;
    }

    m_students.push_back(student);

    if (!saveToFile()) {
        m_students.pop_back();   // undo, because it could not be saved
        return false;
    }
    return true;
}

bool StudentManager::updateStudent(const QString &originalRoll, const Student &updated)
{
    int index = findIndexByRoll(originalRoll);
    if (index == -1) {
        m_lastError = "The record to update could not be found.";
        return false;
    }

    // The new Roll Number may only be used if no OTHER record has it.
    if (rollNumberExists(updated.rollNumber(), index)) {
        m_lastError = QString("Roll Number \"%1\" already belongs to another record.\n"
                              "Please enter a unique Roll Number.")
                          .arg(updated.rollNumber());
        return false;
    }

    Student backup = m_students[index];
    m_students[index] = updated;

    if (!saveToFile()) {
        m_students[index] = backup;   // undo
        return false;
    }

    m_currentIndex = index;
    return true;
}

bool StudentManager::deleteStudent(const QString &originalRoll)
{
    int index = findIndexByRoll(originalRoll);
    if (index == -1) {
        m_lastError = "The record to delete could not be found.";
        return false;
    }

    Student backup = m_students[index];
    m_students.remove(index);

    if (!saveToFile()) {
        m_students.insert(index, backup);   // undo
        return false;
    }

    // Show the next record; if there is none, show the previous one.
    if (m_students.isEmpty())
        m_currentIndex = -1;
    else if (index < m_students.size())
        m_currentIndex = index;
    else
        m_currentIndex = m_students.size() - 1;

    return true;
}

// ===================== CSV file handling =====================

// Puts a field in quotes if it contains a comma or quotation mark,
// and doubles any quotation marks inside it:  He said "hi"  ->  "He said ""hi"""
QString StudentManager::escapeField(const QString &field) const
{
    QString text = field;
    if (text.contains(',') || text.contains('"') ||
        text.contains('\n') || text.contains('\r')) {
        text.replace("\"", "\"\"");
        text = "\"" + text + "\"";
    }
    return text;
}

// Splits the whole CSV text into rows of fields, understanding quotes.
QVector<QStringList> StudentManager::parseCsv(const QString &text) const
{
    QVector<QStringList> rows;
    QStringList row;
    QString field;
    bool inQuotes = false;

    for (int i = 0; i < text.size(); ++i) {
        QChar c = text[i];

        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < text.size() && text[i + 1] == '"') {
                    field += '"';      // two quotes in a row = one real quote
                    ++i;
                } else {
                    inQuotes = false;  // closing quote
                }
            } else {
                field += c;
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == ',') {
                row << field;
                field.clear();
            } else if (c == '\n') {
                row << field;
                field.clear();
                rows.push_back(row);
                row.clear();
            } else if (c == '\r') {
                // ignore (Windows line endings)
            } else {
                field += c;
            }
        }
    }

    if (!field.isEmpty() || !row.isEmpty()) {   // last line without a line break
        row << field;
        rows.push_back(row);
    }
    return rows;
}

bool StudentManager::saveToFile()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        m_lastError = QString("Could not write to the file:\n%1\n\nReason: %2")
                          .arg(m_filePath, file.errorString());
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "Name,RollNumber,FatherName,Caste,District,GPA\n";
    for (int i = 0; i < m_students.size(); ++i) {
        const Student &s = m_students[i];
        out << escapeField(s.name()) << ','
            << escapeField(s.rollNumber()) << ','
            << escapeField(s.fatherName()) << ','
            << escapeField(s.caste()) << ','
            << escapeField(s.district()) << ','
            << QString::number(s.gpa(), 'f', 2) << '\n';
    }
    out.flush();

    if (out.status() != QTextStream::Ok) {
        m_lastError = QString("An error occurred while writing to the file:\n%1")
                          .arg(m_filePath);
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool StudentManager::loadFromFile()
{
    m_students.clear();
    m_currentIndex = -1;
    m_skippedRows = 0;

    // First run: create the file with just a header line.
    if (!QFile::exists(m_filePath))
        return saveToFile();

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Could not read the file:\n%1\n\nReason: %2")
                          .arg(m_filePath, file.errorString());
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString text = in.readAll();
    file.close();

    const QVector<QStringList> rows = parseCsv(text);

    for (int i = 0; i < rows.size(); ++i) {
        const QStringList &row = rows[i];

        // skip empty lines
        if (row.size() == 1 && row[0].trimmed().isEmpty())
            continue;

        // skip the header line
        if (i == 0 && row.size() >= 2 &&
            row[1].trimmed().compare("RollNumber", Qt::CaseInsensitive) == 0)
            continue;

        // every record needs exactly 6 fields
        if (row.size() != 6) {
            m_skippedRows++;
            continue;
        }

        bool ok = false;
        double gpa = row[5].trimmed().toDouble(&ok);
        QString roll = row[1].trimmed();

        if (!ok || gpa < 0.0 || gpa > 4.0 || roll.isEmpty() || rollNumberExists(roll)) {
            m_skippedRows++;
            continue;
        }

        m_students.push_back(Student(row[0].trimmed(), roll, row[2].trimmed(),
                                     row[3].trimmed(), row[4].trimmed(), gpa));
    }

    return true;
}