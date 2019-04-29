#ifndef TRIGGER_CONTROLLER_H
#define TRIGGER_CONTROLLER_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QProcess>
#include <QTimer>
#include <ui_trigger_controller.h>

namespace Ui {
    class Trigger_Controller: public Ui_Form {};
} // namespace Ui

class Trigger_Controller : public QWidget
{
    Q_OBJECT

public:
    explicit Trigger_Controller(QWidget *parent = 0);
    ~Trigger_Controller();

    QSerialPort *port;
    //QLineEdit *line_Edit;
    //QLabel *received_Data;

public slots:
    void tryConncting(bool);
    void ACN_Trigger(bool);
    void SOS_Trigger_Press();
    void SOS_Trigger_Release();
    void setVolValue(int);
    void setStdout();
    void on_checkStart();
    void on_checkStop();
    void onStatusUpdate();

private:
    Ui::Trigger_Controller *ui;
    QProcess *m_process;
    QTimer* m_timer;

    void serial_rescan();
    bool m_bIsVoice;
};

#endif // TRIGGER_CONTROLLER_H
