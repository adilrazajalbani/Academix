#include "mainwindow.h"

#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFont>
#include <QHBoxLayout>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildInterface();
    applyStyle();

    // Load records from students.csv (it is created if it does not exist)
    if (!m_manager.loadFromFile()) {
        showMessage(MessageType::Error, "File Error", m_manager.errorMessage());
    } else if (m_manager.skippedRows() > 0) {
        showMessage(MessageType::Warning, "Some Rows Ignored",
            QString("%1 damaged row(s) in students.csv could not be read and were ignored.\n"
                    "They will be removed the next time the file is saved.")
                .arg(m_manager.skippedRows()));
    }

    enterNewRecordMode();   // start with an empty form
}

// ===================== Building the window =====================

void MainWindow::buildInterface()
{
    setWindowTitle("ACADEMIX - Student Record Management System");
    setFixedSize(600, 570);

    // Window icon: a slate rounded square with a white "A", drawn in code
    QPixmap iconPixmap(64, 64);
    iconPixmap.fill(Qt::transparent);
    QPainter iconPainter(&iconPixmap);
    iconPainter.setRenderHint(QPainter::Antialiasing);
    iconPainter.setPen(Qt::NoPen);
    iconPainter.setBrush(QColor("#2F3E4E"));
    iconPainter.drawRoundedRect(2, 2, 60, 60, 12, 12);
    iconPainter.setPen(Qt::white);
    QFont iconFont = iconPainter.font();
    iconFont.setPixelSize(40);
    iconFont.setBold(true);
    iconPainter.setFont(iconFont);
    iconPainter.drawText(QRect(0, 0, 64, 64), Qt::AlignCenter, "A");
    iconPainter.end();
    setWindowIcon(QIcon(iconPixmap));

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(28, 20, 28, 24);
    mainLayout->setSpacing(12);

    // ----- Heading -----
    QLabel *title = new QLabel("ACADEMIX");
    title->setObjectName("title");
    title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("Student Record Management System");
    subtitle->setObjectName("subtitle");
    subtitle->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);
    mainLayout->addSpacing(4);

    // ----- Form (six input fields) -----
    QFrame *card = new QFrame;
    card->setObjectName("card");
    QFormLayout *form = new QFormLayout(card);
    form->setContentsMargins(24, 18, 24, 18);
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(12);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);   // labels level with the boxes

    m_nameEdit     = new QLineEdit;
    m_rollEdit     = new QLineEdit;
    m_fatherEdit   = new QLineEdit;
    m_casteEdit    = new QLineEdit;
    m_districtEdit = new QLineEdit;
    m_gpaEdit      = new QLineEdit;
    m_nameEdit->setPlaceholderText("Adil Raza");
    m_rollEdit->setPlaceholderText("K26SW020");
    m_fatherEdit->setPlaceholderText("Nazeer Ahmed");
    m_casteEdit->setPlaceholderText("Jalbani");
    m_districtEdit->setPlaceholderText("Ghotki");
    m_gpaEdit->setPlaceholderText("0.00 - 4.00");

    form->addRow("Student Name:", m_nameEdit);
    form->addRow("Roll Number:",  m_rollEdit);
    form->addRow("Father's Name:", m_fatherEdit);
    form->addRow("Caste:",        m_casteEdit);
    form->addRow("District:",     m_districtEdit);
    form->addRow("GPA:",          m_gpaEdit);

    // Give every label and its input box exactly the same height, and centre the
    // label text vertically, so the text of each label lines up with the text in its box.
    QLineEdit *edits[] = { m_nameEdit, m_rollEdit, m_fatherEdit,
                           m_casteEdit, m_districtEdit, m_gpaEdit };
    const int rowHeight = 36;
    for (QLineEdit *edit : edits) {
        edit->setFixedHeight(rowHeight);
        QLabel *label = qobject_cast<QLabel *>(form->labelForField(edit));
        if (label) {
            label->setFixedHeight(rowHeight);
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }

        // Only a mouse click can focus a box, so the Tab key does not move between boxes
        edit->setFocusPolicy(Qt::ClickFocus);
    }

    // Pressing Enter in a box moves the cursor to the next box
    for (int i = 0; i < 5; ++i) {
        QLineEdit *next = edits[i + 1];
        connect(edits[i], &QLineEdit::returnPressed, this, [next]() {
            next->setFocus();
            next->selectAll();
        });
    }

    mainLayout->addWidget(card);

    // ----- Record position indicator -----
    m_recordLabel = new QLabel;
    m_recordLabel->setObjectName("recordLabel");
    m_recordLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_recordLabel);

    // ----- Six buttons -----
    m_addButton      = new QPushButton("Add");
    m_updateButton   = new QPushButton("Update");
    m_deleteButton   = new QPushButton("Delete");
    m_deleteButton->setObjectName("deleteButton");
    m_previousButton = new QPushButton("<");
    m_previousButton->setToolTip("Previous record");
    m_previousButton->setObjectName("arrowButton");
    m_nextButton     = new QPushButton(">");
    m_nextButton->setToolTip("Next record");
    m_nextButton->setObjectName("arrowButton");
    m_clearButton    = new QPushButton("New Record");
    m_clearButton->setToolTip("Clear the form and start a new record");

    // All six buttons get the same, smaller size (width, height)
    QPushButton *allButtons[] = { m_addButton, m_updateButton, m_deleteButton,
                                  m_previousButton, m_nextButton, m_clearButton };
    for (QPushButton *button : allButtons) {
        button->setFixedSize(120, 32);
        button->setFocusPolicy(Qt::NoFocus);   // Tab does not jump to the buttons
    }

    QGridLayout *buttons = new QGridLayout;
    buttons->setHorizontalSpacing(12);
    buttons->setVerticalSpacing(10);
    buttons->addWidget(m_addButton,      0, 0);
    buttons->addWidget(m_updateButton,   0, 1);
    buttons->addWidget(m_deleteButton,   0, 2);
    buttons->addWidget(m_previousButton, 1, 0);
    buttons->addWidget(m_nextButton,     1, 1);
    buttons->addWidget(m_clearButton,    1, 2);

    // Stretches on both sides keep the button group in the centre of the window
    QHBoxLayout *buttonRow = new QHBoxLayout;
    buttonRow->addStretch();
    buttonRow->addLayout(buttons);
    buttonRow->addStretch();
    mainLayout->addLayout(buttonRow);

    // ----- Connect buttons to their functions -----
    connect(m_addButton,      &QPushButton::clicked, this, &MainWindow::onAddClicked);
    connect(m_updateButton,   &QPushButton::clicked, this, &MainWindow::onUpdateClicked);
    connect(m_deleteButton,   &QPushButton::clicked, this, &MainWindow::onDeleteClicked);
    connect(m_previousButton, &QPushButton::clicked, this, &MainWindow::onPreviousClicked);
    connect(m_nextButton,     &QPushButton::clicked, this, &MainWindow::onNextClicked);
    connect(m_clearButton,    &QPushButton::clicked, this, &MainWindow::onClearClicked);
}

void MainWindow::applyStyle()
{
    setStyleSheet(R"(
        QMainWindow, QWidget#centralWidget { background-color: #F5F6F8; }
        QMainWindow { background-color: #F5F6F8; }

        QLabel { color: #1B2A41; font-size: 14px; }

        /* Pop-up message boxes: dark text on a light background */
        QMessageBox { background-color: #FFFFFF; }
        QMessageBox QLabel { color: #1F2A36; background-color: transparent; font-size: 14px; }
        QMessageBox QPushButton { min-width: 90px; min-height: 30px; padding: 0px 14px; }
        QLabel#title {
            color: #2F3E4E; font-size: 34px; font-weight: bold; letter-spacing: 4px; padding-left: 4px;
        }
        QLabel#subtitle { color: #4A5B78; font-size: 15px; }
        QLabel#recordLabel {
            color: #2F3E4E; font-size: 15px; font-weight: bold; padding: 4px;
        }

        QFrame#card {
            background-color: #FFFFFF;
            border: 1px solid #D5DDEA;
            border-radius: 8px;
        }

        QLineEdit {
            background-color: #FFFFFF;
            color: #1B2A41;
            border: 1px solid #B8C4D9;
            border-radius: 5px;
            padding: 7px 9px;
            font-size: 14px;
            min-height: 20px;
        }
        QLineEdit:focus { border: 2px solid #2F3E4E; padding: 6px 8px; }

        QPushButton {
            background-color: #2F3E4E;
            color: #FFFFFF;
            border: none;
            border-radius: 5px;
            padding: 0px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover    { background-color: #44576B; }
        QPushButton:pressed  { background-color: #1F2A36; }
        QPushButton:disabled { background-color: #D5DBE5; color: #7C8899; }

        QPushButton#arrowButton { font-size: 18px; }

        QPushButton#deleteButton          { background-color: #B3261E; }
        QPushButton#deleteButton:hover    { background-color: #C9382F; }
        QPushButton#deleteButton:pressed  { background-color: #8C1D18; }
        QPushButton#deleteButton:disabled { background-color: #D5DBE5; color: #7C8899; }
    )");
}

// ===================== Helper functions =====================

// Reads the six fields, checks them, and builds a Student.
// Returns false (after showing a message) if anything is invalid.
bool MainWindow::readStudentFromForm(Student &student)
{
    QString name     = m_nameEdit->text().trimmed();
    QString roll     = m_rollEdit->text().trimmed();
    QString father   = m_fatherEdit->text().trimmed();
    QString caste    = m_casteEdit->text().trimmed();
    QString district = m_districtEdit->text().trimmed();
    QString gpaText  = m_gpaEdit->text().trimmed();

    // 1. All six fields are required
    QStringList missing;
    if (name.isEmpty())     missing << "Student Name";
    if (roll.isEmpty())     missing << "Roll Number";
    if (father.isEmpty())   missing << "Father's Name";
    if (caste.isEmpty())    missing << "Caste";
    if (district.isEmpty()) missing << "District";
    if (gpaText.isEmpty())  missing << "GPA";

    if (!missing.isEmpty()) {
        showMessage(MessageType::Warning, "Missing Information",
            "Please fill in the following required field(s):\n\n- " + missing.join("\n- "));
        return false;
    }

    
    // 2. GPA must be a valid number with a maximum of two decimal places
    QRegularExpression numberPattern("^[0-9]+(\\.[0-9]{1,2})?$");
    if (!numberPattern.match(gpaText).hasMatch()) {
    showMessage(MessageType::Warning, "Invalid GPA",
        "GPA must be a number with a maximum of two decimal places (e.g., 3.50).");
    return false;
    }

    // 3. GPA must be between 0.00 and 4.00
    bool ok = false;
    double gpa = gpaText.toDouble(&ok);

    if (!ok || gpa < 0.0 || gpa > 4.0) {
    showMessage(MessageType::Warning, "GPA Out of Range",
        "GPA must be between 0.00 and 4.00.");
    return false;
}


    student = Student(name, roll, father, caste, district, gpa);
    return true;
}

void MainWindow::showCurrentRecord()
{
    Student s = m_manager.currentStudent();

    m_nameEdit->setText(s.name());
    m_rollEdit->setText(s.rollNumber());
    m_fatherEdit->setText(s.fatherName());
    m_casteEdit->setText(s.caste());
    m_districtEdit->setText(s.district());
    m_gpaEdit->setText(QString::number(s.gpa(), 'f', 2));

    m_originalRoll = s.rollNumber();   // remember which record is on screen
    updateButtonStates();
}

void MainWindow::enterNewRecordMode()
{
    m_manager.startNewRecord();

    m_nameEdit->clear();
    m_rollEdit->clear();
    m_fatherEdit->clear();
    m_casteEdit->clear();
    m_districtEdit->clear();
    m_gpaEdit->clear();
    m_originalRoll.clear();

    updateButtonStates();
    m_nameEdit->setFocus();
}

void MainWindow::updateButtonStates()
{
    bool showingRecord = m_manager.hasCurrent();

    // Add is available on the empty form AND when the last record is shown
    // (that is, whenever there is no next record).
    m_addButton->setEnabled(!m_manager.hasNext());
    m_updateButton->setEnabled(showingRecord);
    m_deleteButton->setEnabled(showingRecord);
    m_previousButton->setEnabled(m_manager.hasPrevious());
    m_nextButton->setEnabled(m_manager.hasNext());

    if (showingRecord)
        m_recordLabel->setText(QString("Record %1 of %2")
                                   .arg(m_manager.currentIndex() + 1)
                                   .arg(m_manager.count()));
    else if (m_manager.count() == 0)
        m_recordLabel->setText("New Record (no records saved yet)");
    else
        m_recordLabel->setText(QString("New Record (%1 saved)").arg(m_manager.count()));
}

// ===================== Button functions =====================

void MainWindow::onAddClicked()
{
    Student student;
    if (!readStudentFromForm(student))
        return;

    if (!m_manager.addStudent(student)) {
        showMessage(MessageType::Warning, "Cannot Add Record", m_manager.errorMessage());
        return;
    }

    showMessage(MessageType::Success, "Success", "The record was added successfully.");
    enterNewRecordMode();
}

void MainWindow::onUpdateClicked()
{
    if (!m_manager.hasCurrent() || m_originalRoll.isEmpty()) {
        showMessage(MessageType::Warning, "No Record Selected",
                             "There is no record selected to update.");
        return;
    }

    Student student;
    if (!readStudentFromForm(student))
        return;

    if (!m_manager.updateStudent(m_originalRoll, student)) {
        showMessage(MessageType::Warning, "Cannot Update Record", m_manager.errorMessage());
        return;
    }

    showMessage(MessageType::Success, "Success", "The record was updated successfully.");
    showCurrentRecord();   // refresh the form (and the original Roll Number)
}

void MainWindow::onDeleteClicked()
{
    if (!m_manager.hasCurrent() || m_originalRoll.isEmpty()) {
        showMessage(MessageType::Warning, "No Record Selected",
                             "There is no record selected to delete.");
        return;
    }

    Student s = m_manager.currentStudent();
    bool confirmed = askConfirm("Confirm Delete",
        QString("Are you sure you want to delete this record?\n\n%1 (Roll Number: %2)")
            .arg(s.name(), s.rollNumber()));

    if (!confirmed)
        return;

    if (!m_manager.deleteStudent(m_originalRoll)) {
        showMessage(MessageType::Warning, "Cannot Delete Record", m_manager.errorMessage());
        return;
    }

    showMessage(MessageType::Success, "Deleted", "The record was deleted successfully.");

    if (m_manager.hasCurrent())
        showCurrentRecord();     // next record (or previous if it was the last)
    else
        enterNewRecordMode();    // no records remain
}

void MainWindow::onPreviousClicked()
{
    if (!m_manager.movePrevious()) {
        showMessage(MessageType::Info, "Navigation", m_manager.errorMessage());
        updateButtonStates();
        return;
    }
    showCurrentRecord();
}

void MainWindow::onNextClicked()
{
    if (!m_manager.moveNext()) {
        showMessage(MessageType::Info, "Navigation", m_manager.errorMessage());
        updateButtonStates();
        return;
    }
    showCurrentRecord();
}

void MainWindow::onClearClicked()
{
    enterNewRecordMode();
}

// ===================== Pop-up messages =====================

// Draws a round coloured icon with a simple symbol inside it.
QPixmap MainWindow::makeIcon(MessageType type) const
{
    const int size = 48;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Colour of the icon
    QColor color("#2F3E4E");                                       // Info, Question (slate)
    if (type == MessageType::Success) color = QColor("#2E7D32");   // green
    if (type == MessageType::Warning) color = QColor("#E08A00");   // amber
    if (type == MessageType::Error)   color = QColor("#C62828");   // red

    painter.setBrush(color);

    if (type == MessageType::Warning) {
        // Triangle with slightly rounded corners (pen has the same colour as the fill)
        painter.setPen(QPen(color, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const QPoint triangle[3] = { QPoint(24, 5), QPoint(44, 41), QPoint(4, 41) };
        painter.drawPolygon(triangle, 3);
    } else {
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(2, 2, size - 4, size - 4);
    }

    // White symbol inside the icon
    QPen pen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    if (type == MessageType::Success) {                      // tick mark
        const QPoint tick[3] = { QPoint(14, 25), QPoint(21, 32), QPoint(34, 17) };
        painter.drawPolyline(tick, 3);
    } else if (type == MessageType::Error) {                 // cross
        painter.drawLine(16, 16, 32, 32);
        painter.drawLine(32, 16, 16, 32);
    } else if (type == MessageType::Warning) {               // exclamation mark
        painter.drawLine(24, 17, 24, 28);
        painter.drawPoint(24, 34);
    } else {                                                 // "i" or "?"
        QFont font = painter.font();
        font.setPixelSize(30);
        font.setBold(true);
        painter.setFont(font);
        QString letter = (type == MessageType::Info) ? "i" : "?";
        painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, letter);
    }

    return pixmap;
}

// Shows a message with an OK button.
void MainWindow::showMessage(MessageType type, const QString &title, const QString &text)
{
    QMessageBox box(this);
    box.setWindowTitle(title);
    box.setText(text);
    box.setTextFormat(Qt::PlainText);
    box.setIconPixmap(makeIcon(type));
    box.setStandardButtons(QMessageBox::Ok);
    box.setDefaultButton(QMessageBox::Ok);
    box.exec();
}

// Shows a Yes / No question. Returns true only if the user clicks Yes.
bool MainWindow::askConfirm(const QString &title, const QString &text)
{
    QMessageBox box(this);
    box.setWindowTitle(title);
    box.setText(text);
    box.setTextFormat(Qt::PlainText);
    box.setIconPixmap(makeIcon(MessageType::Question));
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    return box.exec() == QMessageBox::Yes;
}