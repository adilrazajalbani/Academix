#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QString>
#include "student.h"
#include "studentmanager.h"

class QLineEdit;
class QLabel;
class QPushButton;

// The one and only window of the application.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onAddClicked();
    void onUpdateClicked();
    void onDeleteClicked();
    void onPreviousClicked();
    void onNextClicked();
    void onClearClicked();

private:
    // Kinds of pop-up messages (each one gets its own icon)
    enum class MessageType { Success, Warning, Error, Info, Question };

    void buildInterface();
    void applyStyle();
    void showCurrentRecord();      // fills the form with the selected record
    void enterNewRecordMode();     // clears the form, enables Add only
    void updateButtonStates();     // buttons + "Record x of y" label
    bool readStudentFromForm(Student &student);   // validates the six fields

    // Pop-up messages
    QPixmap makeIcon(MessageType type) const;
    void showMessage(MessageType type, const QString &title, const QString &text);
    bool askConfirm(const QString &title, const QString &text);

    StudentManager m_manager;      // holds all the records
    QString m_originalRoll;        // Roll Number of the record being displayed

    // Widgets (created with new; Qt deletes them automatically via parents)
    QLineEdit *m_nameEdit;
    QLineEdit *m_rollEdit;
    QLineEdit *m_fatherEdit;
    QLineEdit *m_casteEdit;
    QLineEdit *m_districtEdit;
    QLineEdit *m_gpaEdit;

    QLabel *m_recordLabel;

    QPushButton *m_addButton;
    QPushButton *m_updateButton;
    QPushButton *m_deleteButton;
    QPushButton *m_previousButton;
    QPushButton *m_nextButton;
    QPushButton *m_clearButton;
};

#endif // MAINWINDOW_H