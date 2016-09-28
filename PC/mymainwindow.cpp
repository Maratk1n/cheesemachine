#include "mymainwindow.h"
#include "ui_mymainwindow.h"
#include "settingsdialog.h"
#include "console.h"
#include "qcustomplot.h"

#include <QtSerialPort/QSerialPort>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QFont>

MyMainWindow::MyMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MyMainWindow)
{
    //создаем директорию, если ее еще нет
    QDir dir("plotImages");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    ui->setupUi(this);

    //настройки графика
    ui->PlotWidget->clearGraphs(); //очищаем графики
    ui->PlotWidget->addGraph(); //добавляем график
    //ui->PlotWidget->graph(0)->setPen(QColor(50, 50, 50, 255));//задаем цвет точки
    //ui->PlotWidget->graph(0)->setLineStyle(QCPGraph::lsNone);//убираем линии
    //ui->PlotWidget->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4)); //формируем вид точек
    ui->PlotWidget->xAxis->setLabel("Time, (s)"); //подпись для оси Х
    ui->PlotWidget->yAxis->setLabel("The load on the sensor, (g)"); //подпись для У
    //Установим область, которая будет показываться на графике
    ui->PlotWidget->xAxis->setRange(0, 10);//Для оси Ox
    ui->PlotWidget->yAxis->setRange(0, 20); //Для оси Оу
    //заголовок графика
    ui->PlotWidget->plotLayout()->insertRow(0);
    ui->PlotWidget->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->PlotWidget, "A plot of the load on the sensor of the time-dependent"));
    //И перерисуем график на нашем PlotWidget
    ui->PlotWidget->replot();



    console = new Console;
    console->setEnabled(false);
    //setCentralWidget(console);

    serial = new QSerialPort(this);

    settings = new SettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    initActionConnection();

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
}

MyMainWindow::~MyMainWindow()
{
    delete ui;
    delete settings;
}

void MyMainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    if (serial->open(QIODevice::ReadOnly)) {
        if (serial->setBaudRate(p.baudRate)
                && serial->setDataBits(p.dataBits)
                && serial->setParity(p.parity)
                && serial->setStopBits(p.stopBits)
                && serial->setFlowControl(p.flowControl)) {

            console->setEnabled(true);
            console->setLocalEchoEnable(p.localEchoEnabled);
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
            ui->actionConfigure->setEnabled(false);
            ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                       .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

        } else {
            serial->close();
            QMessageBox::critical(this, tr("Error"), serial->errorString());
            ui->statusBar->showMessage(tr("Open Error"));
        }
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());
        ui->statusBar->showMessage(tr("Configure error"));
    }
}

void MyMainWindow::closeSerialPort()
{
    serial->close();
    console->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    ui->statusBar->showMessage(tr("Disconnected"));
}

void MyMainWindow::about()
{
    QMessageBox::about(this, tr("About device for measuring the extensibility of cheese"),
                       tr("Authors:<br><ul> <li><b>Nikolay Moiseev</b> (Food technology)</li><br>"
                          "<li><b>Marat Gareev</b> (Developer, e-mail: maratkin94@gmail.com)</li></ul>"));
}

void MyMainWindow::readData()
{
    unsigned char dataByte;
    QByteArray data = serial->readAll();
    console->putData(data);
    //разбираем на байты и отправляет в обработку ( функция построения графика)
    for (int iByte = 0; iByte < data.size(); iByte++) {
        dataByte = data.data()[iByte];
        drawScope(dataByte);
    }

}

void MyMainWindow::saveImage()
{
    QDateTime dt = QDateTime::currentDateTime();
    QString outputDir = QDir::currentPath() + "/plotImages/";
    QString fileName = dt.toString("dd-MM-yy_HH;mm;ss")+".bmp";
    QFile file(outputDir + fileName);

    if (!file.open(QIODevice::WriteOnly)) {

    } else {
        ui->PlotWidget->saveBmp(outputDir + fileName, 0, 0, 1.0);
    }
}


//рисуем график
void MyMainWindow::drawScope(unsigned char buffer)
{
    static QVector<double> x, y; //массив Х и У неизвестного размера
    static int i = 0;
    static QString bufStr;
    static double xMin = 0, xMax = 10, yMin = 0, yMax = 20;

    switch (buffer) {
    case 'C': //если пришел байт очистки, удаляем график и обнуляем переменные
        i = 0;
        x.clear();
        y.clear();
        xMin = 0; xMax = 10; yMin = 0; yMax = 20;
        clearPlot();
        break;
    case ':': //конец посылки для оси Х
        x.resize(i + 1);
        x[i] = bufStr.toDouble() / 1000;
        if (xMin > x[i]) xMin = x[i] - 1;
        if (xMax < x[i]) xMax = x[i] + 1;
        bufStr = "";
        break;
    case ';': //конец посылки для оси Y
        y.resize(i + 1);
        y[i] = bufStr.toDouble();
        if (yMin > y[i]) yMin = y[i] - 1;
        if (yMax < y[i]) yMax = y[i] + 1;
        bufStr = "";
        i++;
        ui->PlotWidget->xAxis->setRange(xMin, xMax);
        ui->PlotWidget->yAxis->setRange(yMin, yMax);
        ui->PlotWidget->graph(0)->setData(x, y); //отрисовываем по двум массивам Х и У
        ui->PlotWidget->replot();
        break;
    default:
        bufStr += buffer;
        break;
    }
}

void MyMainWindow::clearPlot()
{
    ui->PlotWidget->clearGraphs();
    ui->PlotWidget->addGraph();
    ui->PlotWidget->xAxis->setRange(0, 10);
    ui->PlotWidget->yAxis->setRange(0, 20);
    ui->PlotWidget->replot();
}

void MyMainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        closeSerialPort();
    }
}

void MyMainWindow::initActionConnection()
{
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConfigure, SIGNAL(triggered()), settings, SLOT(show()));
    connect(ui->actionClear, SIGNAL(triggered()), console, SLOT(clear()));
    connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(clearPlot())); //дополнительный слот для очистки графика
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionSave_scope_as_image, SIGNAL(triggered()), this, SLOT(saveImage()));
}
