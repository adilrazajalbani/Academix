#include <QApplication>
#include <QStyleHints>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");   // gives a consistent look on every computer
    // Always use the light theme, even if Windows is in dark mode
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);

    MainWindow window;
    window.show();

    return app.exec();
}