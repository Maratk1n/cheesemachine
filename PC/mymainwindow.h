#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>

QT_BEGIN_NAMESPACE

namespace Ui {
class MyMainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;

class MyMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MyMainWindow(QWidget *parent = 0);
    ~MyMainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void clearPlot();
    void about();
    void readData();
    void saveImage();

    void handleError(QSerialPort::SerialPortError error);

private:
    void initActionConnection();

private:
    void drawScope(unsigned char buffer);
    Ui::MyMainWindow *ui;
    Console *console;
    SettingsDialog *settings;
    QSerialPort *serial;
};

#endif // MYMAINWINDOW_H
