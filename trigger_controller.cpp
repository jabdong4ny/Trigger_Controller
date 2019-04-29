#include "trigger_controller.h"
#include "ui_trigger_controller.h"

#include <QtDebug>
#include <QSerialPortInfo>
#include <QMessageBox>

Trigger_Controller::Trigger_Controller(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Trigger_Controller),
    port(new QSerialPort()),
    m_bIsVoice(false)
{
    ui->setupUi(this);
    QObject::connect(ui->Connect, SIGNAL(clicked(bool)),this, SLOT(tryConncting(bool)));
    QObject::connect(ui->AcnButton, SIGNAL(clicked(bool)), this, SLOT(ACN_Trigger(bool)));
    QObject::connect(ui->VolumeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setVolValue(int)));
    QObject::connect(ui->SosButton, SIGNAL(pressed()), this, SLOT(SOS_Trigger_Press()));
    QObject::connect(ui->SosButton, SIGNAL(released()), this, SLOT(SOS_Trigger_Release()));
    QObject::connect(ui->MonitoringStartButton, SIGNAL(clicked()), this, SLOT(on_checkStart()));
    QObject::connect(ui->MonitoringStopButton, SIGNAL(clicked()), this, SLOT(on_checkStop()));

    ui->AcnButton->setEnabled(false);
    ui->VolumeSpinBox->setEnabled(false);
    ui->volumeDial->setEnabled(false);
    serial_rescan();
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(setStdout()) );
    //m_process->setWorkingDirectory(qApp->applicationDirPath());
    m_timer = new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onStatusUpdate()));

}

Trigger_Controller::~Trigger_Controller()
{
    delete ui;
}

void Trigger_Controller::tryConncting(bool){
    if(port->isOpen()){
        port->close();
        ui->AcnButton->setEnabled(false);
        ui->AcnButton->setStyleSheet(QLatin1String("color: rgb(255, 255, 255);\n"
            "background-color: rgb(193, 193, 193); font: 75 16pt \"Consolas\";"));
        ui->VolumeSpinBox->setEnabled(false);
        ui->volumeDial->setEnabled(false);
        ui->Connect->setText(QApplication::translate("Form", "Connect", Q_NULLPTR));
        return;
    }
    port->setPortName(ui->COMportComboBox->currentText());
    port->setBaudRate(QSerialPort::Baud115200);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    if(!port->open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(this, tr("port error"), "\n Serial port open error \n");
    } else {
        ui->AcnButton->setEnabled(true);
        ui->AcnButton->setStyleSheet(QLatin1String("color: rgb(255, 255, 255);\n"
            "background-color: rgb(255, 0, 0);font: 75 16pt \"Consolas\";"));
        ui->VolumeSpinBox->setEnabled(true);
        ui->volumeDial->setEnabled(true);
        ui->Connect->setText(QApplication::translate("Form", "Connected", Q_NULLPTR));
    }

}

void Trigger_Controller::ACN_Trigger(bool)
{
    QByteArray send_Data;
    send_Data = "spitx 2b09 0201 \r\n";
    qDebug() << send_Data.data();
    port->write(send_Data.data());
}

void Trigger_Controller::setVolValue(int val){
    qDebug() << val;
    QByteArray send_Data;
    QString temp = "did 2006 ";
    QString value;
    value.setNum(val);
    temp +=value;
    temp +=" \r\n";

    qDebug() << temp;
    port->write(temp.toUtf8().data());
}

void Trigger_Controller::SOS_Trigger_Press(){
    QString strCommand;
    if(QSysInfo::productType()=="windows")
        strCommand = "cmd /C ";
    strCommand += "adb shell sldd hmi BtnPress 1";
    m_process->start(strCommand);
    qDebug() << "send sos press key";
}

void Trigger_Controller::SOS_Trigger_Release(){
    QString strCommand;
    if(QSysInfo::productType()=="windows")
        strCommand = "cmd /C ";
    strCommand += "adb shell sldd hmi BtnPress 0";
    m_process->start(strCommand);
    qDebug() << "send sos release key";
}

void Trigger_Controller::serial_rescan()
{
    ui->COMportComboBox->clear();
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::  availablePorts()) {
        ui->COMportComboBox->addItem(serialPortInfo.portName());
    }
}

void Trigger_Controller::setStdout()
{
    QString msg = QString::fromLocal8Bit(m_process->readAllStandardOutput()/* + m_process->readAllStandardError()*/);
    qDebug() << msg;

    if(msg.contains("Service state (Voice) :", Qt::CaseInsensitive)){
        bool bIsServiceIn = false;
        if(msg.contains("IN SERVICE", Qt::CaseInsensitive)){
            bIsServiceIn = true;
            ui->VoiceStatelineEdit->setText(QApplication::translate("Form", "IN SERVICE", Q_NULLPTR));
        } else if(msg.contains("OUT OF SERVICE", Qt::CaseInsensitive)){
            ui->VoiceStatelineEdit->setText(QApplication::translate("Form", "OUT OF SERVICE", Q_NULLPTR));
        } else if(msg.contains("EMEGENCY ONLY", Qt::CaseInsensitive)){
            ui->VoiceStatelineEdit->setText(QApplication::translate("Form", "EMEGENCY ONLY", Q_NULLPTR));
        } else if(msg.contains("POWER OFF", Qt::CaseInsensitive)){
            ui->VoiceStatelineEdit->setText(QApplication::translate("Form", "POWER OFF", Q_NULLPTR));
        }

        if(bIsServiceIn == true) {
            ui->VoiceStatelineEdit->setStyleSheet(QLatin1String("background-color: rgb(255, 255, 0);\n"
                                                                "font: 75 10pt \"Consolas\";"));
        } else {
            ui->VoiceStatelineEdit->setStyleSheet(QLatin1String("background-color: rgb(255, 0, 0);]n"
                                                                "font: 75 10pt \"Consolas\";"));
        }

    }
    if(msg.contains("Service state (Data) :", Qt::CaseInsensitive)){
        bool bIsServiceIn = false;
        if(msg.contains("IN SERVICE", Qt::CaseInsensitive)){
            bIsServiceIn = true;
            ui->DataStateLineEdit->setText(QApplication::translate("Form", "IN SERVICE", Q_NULLPTR));
        } else if(msg.contains("OUT OF SERVICE", Qt::CaseInsensitive)){
            ui->DataStateLineEdit->setText(QApplication::translate("Form", "OUT OF SERVICE", Q_NULLPTR));
        } else if(msg.contains("EMEGENCY ONLY", Qt::CaseInsensitive)){
            ui->DataStateLineEdit->setText(QApplication::translate("Form", "EMEGENCY ONLY", Q_NULLPTR));
        } else if(msg.contains("POWER OFF", Qt::CaseInsensitive)){
            ui->DataStateLineEdit->setText(QApplication::translate("Form", "POWER OFF", Q_NULLPTR));
        }
        if(bIsServiceIn == true) {
            ui->DataStateLineEdit->setStyleSheet(QLatin1String("background-color: rgb(255, 255, 0);\n"
                                                               "font: 75 10pt \"Consolas\";"));
        } else {
            ui->DataStateLineEdit->setStyleSheet(QLatin1String("background-color: rgb(255, 0, 0);\n"
                                                               "font: 75 10pt \"Consolas\";"));
        }
    }
}

void Trigger_Controller::on_checkStart(){
    if(m_timer->isActive() == false){
        m_timer->start(1000);
    } else {
        qDebug() << "already started!!";
    }
}

void Trigger_Controller::on_checkStop(){
    m_timer->stop();
    ui->VoiceStatelineEdit->setStyleSheet(QLatin1String("background-color: rgb(255, 0, 0);\n"
                                                        "font: 75 10pt \"Consolas\";"));
    ui->VoiceStatelineEdit->setText(QApplication::translate("Form", "Service status", Q_NULLPTR));
    ui->DataStateLineEdit->setStyleSheet(QLatin1String("background-color: rgb(255, 0, 0);\n"
                                                       "font: 75 10pt \"Consolas\";"));
    ui->DataStateLineEdit->setText(QApplication::translate("Form", "Service status", Q_NULLPTR));
}

void Trigger_Controller::onStatusUpdate(){
    QString strCommand;
    if(QSysInfo::productType()=="windows")
        strCommand = "cmd /C ";
    QString value;
    value.setNum((m_bIsVoice == true) ? 0 : 1);
    strCommand += "adb shell sldd telephony getServiceState " + value;
    m_process->start(strCommand);
    qDebug() <<  strCommand;
    qDebug() << "get telephony service state " << m_bIsVoice;
    m_bIsVoice = !m_bIsVoice;
}
