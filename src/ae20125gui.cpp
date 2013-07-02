#include "ae20125gui.h"
#include "ui_ae20125gui.h"
#include "ui_about.h"

#include <QCoreApplication>

#include <QList>
#include <QtAlgorithms>   // for qSort()
#include <QtCore/qmath.h>

#include <QTextStream>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

bool connected = false;
bool limit = false;
long lastfreq=100000;
QString buf;

AE20125gui::AE20125gui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AE20125gui)
{
    ui->setupUi(this);
    AE20125gui::log("Searching for serial ports...");

    QList<QSerialPortInfo> serialPortInfoList = QSerialPortInfo::availablePorts();

    foreach (const QSerialPortInfo &serialPortInfo, serialPortInfoList) {
        ui->comboBox_con_port->addItem(serialPortInfo.systemLocation());
    }

    serial = new QSerialPort(this);
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));

    AE20125gui::log("Idle");

}

AE20125gui::~AE20125gui()
{
    delete ui;
}

/**
 * SLEEP HACK
 */
#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif
void AE20125gui::Sleep(int ms)
{
    if(ms <= 0) return;

#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

void AE20125gui::on_pushButton_con_connect_clicked()
{
    ui->pushButton_con_connect->setEnabled(false);
    ui->comboBox_con_port->setEnabled(false);

    if(!connected) {
        AE20125gui::log("Connecting to " +  ui->comboBox_con_port->currentText());

        QList<QSerialPortInfo> serialPortInfoList = QSerialPortInfo::availablePorts();

        int found=0;

        foreach (const QSerialPortInfo &serialPortInfo, serialPortInfoList) {
            if(serialPortInfo.systemLocation() == ui->comboBox_con_port->currentText()) {
                found=1;
                if(serialPortInfo.isBusy()) {
                    AE20125gui::log("Port " + ui->comboBox_con_port->currentText() + " is busy!");

                    ui->pushButton_con_connect->setEnabled(true);
                    ui->comboBox_con_port->setEnabled(true);

                    return;
                }
            }
        }

        if(found==0) {

            ui->pushButton_con_connect->setEnabled(true);
            ui->comboBox_con_port->setEnabled(true);

            AE20125gui::log("Port " + ui->comboBox_con_port->currentText() + " not found!");
        } else {

            serial->setPortName(ui->comboBox_con_port->currentText());

            if(!serial->open(QIODevice::ReadWrite)) {
                serial->close();
                AE20125gui::log("Failed to connect to " + ui->comboBox_con_port->currentText() + ": " + serial->errorString());

                ui->pushButton_con_connect->setEnabled(true);
                ui->comboBox_con_port->setEnabled(true);

                return;
            }

            if(
                !serial->setBaudRate(QSerialPort::Baud9600) ||
                !serial->setDataBits(QSerialPort::Data8) ||
                !serial->setParity(QSerialPort::NoParity) ||
                !serial->setStopBits(QSerialPort::OneStop)
              ) {
                serial->close();
                AE20125gui::log("Failed to configure " + ui->comboBox_con_port->currentText() + ": " + serial->errorString());

                ui->pushButton_con_connect->setEnabled(true);
                ui->comboBox_con_port->setEnabled(true);

                return;
            }


            AE20125gui::log("Connected to " +  ui->comboBox_con_port->currentText());

            connected = true;
            ui->label_con_status->setText("Connected");

            ui->pushButton_con_connect->setText("Disconnect");
            ui->pushButton_con_connect->setEnabled(true);

            ui->groupBox_freq->setEnabled(true);
            ui->groupBox_mode->setEnabled(true);
            ui->groupBox_wav->setEnabled(true);
            ui->groupBox_set->setEnabled(true);
            ui->groupBox_file->setEnabled(true);
            ui->groupBox_pll->setEnabled(true);

            AE20125gui::Sleep(500);

            AE20125gui::sendCmd("T:0");
        }
    } else {
        serial->close();
        connected = false;
        ui->label_con_status->setText("Not connected");
        ui->pushButton_con_connect->setText("Connect");
        ui->pushButton_con_connect->setEnabled(true);
        ui->comboBox_con_port->setEnabled(true);


        ui->groupBox_freq->setEnabled(false);
        ui->groupBox_mode->setEnabled(false);
        ui->groupBox_wav->setEnabled(false);
        ui->groupBox_set->setEnabled(false);
        ui->groupBox_file->setEnabled(false);
        ui->groupBox_sweep->setEnabled(false);
        ui->groupBox_mod->setEnabled(false);
        ui->groupBox_pll->setEnabled(false);

        ui->label_hw_fwv->setText("unknown");
        ui->label_hw_hwv->setText("unknown");
        ui->label_hw_pidv->setText("unknown");

        AE20125gui::log("Disconnected from " +  ui->comboBox_con_port->currentText());
    }
}

void AE20125gui::log(QString msg) {
    ui->statusBar->showMessage(msg);
    ui->textEdit_debug->append(msg);
}

void AE20125gui::on_pushButton_con_about_clicked()
{
    QDialog* about = new QDialog(0,0);

    Ui_About aboutUi;
    aboutUi.setupUi(about);

    about->show();
}

void AE20125gui::on_radioButton_mode_norm_clicked()
{
    ui->groupBox_mod->setEnabled(false);
    ui->groupBox_sweep->setEnabled(false);
    AE20125gui::sendCmd("C:0");
    AE20125gui::sendCmd("V:0");
}

void AE20125gui::on_radioButton_mode_sweep_clicked()
{
    ui->groupBox_mod->setEnabled(false);
    ui->groupBox_sweep->setEnabled(true);
    AE20125gui::sendCmd("C:1");
}

void AE20125gui::on_radioButton_mode_mod_clicked()
{
    ui->groupBox_mod->setEnabled(true);
    ui->groupBox_sweep->setEnabled(false);
    AE20125gui::sendCmd("C:2");
}

void AE20125gui::on_radioButton_mod_typ_PSK_clicked()
{
    ui->label_mod_typ_phase->setEnabled(true);
    ui->label_mod_typ_deg->setEnabled(true);
    ui->doubleSpinBox_mod_typ_deg->setEnabled(true);

    ui->label_mod_typ_freq->setEnabled(false);
    ui->label_mod_typ_hz->setEnabled(false);
    ui->doubleSpinBox_mod_typ_freq->setEnabled(false);

    AE20125gui::sendCmd("M:1");
}

void AE20125gui::on_radioButton_mod_typ_fsk_clicked()
{
    ui->label_mod_typ_phase->setEnabled(false);
    ui->label_mod_typ_deg->setEnabled(false);
    ui->doubleSpinBox_mod_typ_deg->setEnabled(false);

    ui->label_mod_typ_freq->setEnabled(true);
    ui->label_mod_typ_hz->setEnabled(true);
    ui->doubleSpinBox_mod_typ_freq->setEnabled(true);

    AE20125gui::sendCmd("M:0");
}

void AE20125gui::on_radioButton_mod_src_int_clicked()
{
    ui->label_mod_src_freq->setEnabled(true);
    ui->label_mod_src_hz->setEnabled(true);
    ui->doubleSpinBox_mod_src_freq->setEnabled(true);

    AE20125gui::sendCmd("P:0");
}

void AE20125gui::on_radioButton_mod_src_ext_clicked()
{
    ui->label_mod_src_freq->setEnabled(false);
    ui->label_mod_src_hz->setEnabled(false);
    ui->doubleSpinBox_mod_src_freq->setEnabled(false);

    AE20125gui::sendCmd("P:1");
}

void AE20125gui::on_checkBox_pll_on_stateChanged(int arg1)
{
    if(arg1 == 0) {
        ui->doubleSpinBox_pll_fact->setEnabled(false);
        ui->label_pll_fact->setEnabled(false);
        ui->label_pll_hz->setEnabled(false);
        ui->label_pll_off->setEnabled(false);
        ui->spinBox_pll_off->setEnabled(false);
        AE20125gui::sendCmd("D:0");
    }else{
        ui->doubleSpinBox_pll_fact->setEnabled(true);
        ui->label_pll_fact->setEnabled(true);
        ui->label_pll_hz->setEnabled(true);
        ui->label_pll_off->setEnabled(true);
        ui->spinBox_pll_off->setEnabled(true);
        AE20125gui::sendCmd("D:1");
    }
}

void AE20125gui::validateFreq() {
    //@todo save sine value?

    if(limit) {
        if(ui->spinBox_m10->value() > 0) {
            ui->spinBox_m1->setMaximum(3);
            ui->spinBox_m1->setValue(3);
            AE20125gui::log("Frequency is limited to 2.5MHz in this mode");
        }

        ui->spinBox_m10->setValue(0);
        ui->spinBox_m10->setEnabled(false);

        if(ui->spinBox_m1->value() > 2) {
            ui->spinBox_m1->setValue(2);
            ui->spinBox_k100->setValue(5);
            ui->spinBox_k10->setValue(0);
            ui->spinBox_k1->setValue(0);
            ui->spinBox_100->setValue(0);
            ui->spinBox_10->setValue(0);
            ui->spinBox_1->setValue(0);
            ui->spinBox_01->setValue(0);
            AE20125gui::log("Frequency is limited to 2.5MHz in this mode");
        }
        ui->spinBox_m1->setMaximum(2);

        if(ui->spinBox_m1->value() == 2 && ui->spinBox_k100->value() >= 5) {
            AE20125gui::log("Frequency is limited to 2.5MHz in this mode");
            ui->spinBox_k100->setValue(5);
            ui->spinBox_k10->setValue(0);
            ui->spinBox_k1->setValue(0);
            ui->spinBox_100->setValue(0);
            ui->spinBox_10->setValue(0);
            ui->spinBox_1->setValue(0);
            ui->spinBox_01->setValue(0);
        }

        ui->spinBox_m1->setMaximum(2);

        if(ui->spinBox_m1->value() == 2) {
            ui->spinBox_k100->setMaximum(5);

            if(ui->spinBox_k100->value() == 5) {
                ui->spinBox_k10->setMaximum(0);
                ui->spinBox_k1->setMaximum(0);
                ui->spinBox_100->setMaximum(0);
                ui->spinBox_10->setMaximum(0);
                ui->spinBox_1->setMaximum(0);
                ui->spinBox_01->setMaximum(0);
            } else {
                ui->spinBox_k10->setMaximum(9);
                ui->spinBox_k1->setMaximum(9);
                ui->spinBox_100->setMaximum(9);
                ui->spinBox_10->setMaximum(9);
                ui->spinBox_1->setMaximum(9);
                ui->spinBox_01->setMaximum(9);
            }
        }else{
            ui->spinBox_k100->setMaximum(9);
            ui->spinBox_k10->setMaximum(9);
            ui->spinBox_k1->setMaximum(9);
            ui->spinBox_100->setMaximum(9);
            ui->spinBox_10->setMaximum(9);
            ui->spinBox_1->setMaximum(9);
            ui->spinBox_01->setMaximum(9);
        }
    }else{
        ui->spinBox_m10->setEnabled(true);
        if(ui->spinBox_m10->value() == 1) {
            AE20125gui::log("Frequency is limited to 10MHz in this mode");
            ui->spinBox_m1->setValue(0);
            ui->spinBox_k100->setValue(0);
            ui->spinBox_k10->setValue(0);
            ui->spinBox_k1->setValue(0);
            ui->spinBox_100->setValue(0);
            ui->spinBox_10->setValue(0);
            ui->spinBox_1->setValue(0);
            ui->spinBox_01->setValue(0);

            ui->spinBox_m1->setMaximum(0);
            ui->spinBox_k100->setMaximum(0);
            ui->spinBox_k10->setMaximum(0);
            ui->spinBox_k1->setMaximum(0);
            ui->spinBox_100->setMaximum(0);
            ui->spinBox_10->setMaximum(0);
            ui->spinBox_1->setMaximum(0);
            ui->spinBox_01->setMaximum(0);
        }else{
            ui->spinBox_m1->setMaximum(9);
            ui->spinBox_k100->setMaximum(9);
            ui->spinBox_k10->setMaximum(9);
            ui->spinBox_k1->setMaximum(9);
            ui->spinBox_100->setMaximum(9);
            ui->spinBox_10->setMaximum(9);
            ui->spinBox_1->setMaximum(9);
            ui->spinBox_01->setMaximum(9);
        }
    }

    //Calculate numeric frequency
    long freq = 0;
    freq += ui->spinBox_m10->value()  * 100000000;
    freq += ui->spinBox_m1->value()   * 10000000;
    freq += ui->spinBox_k100->value() * 1000000;
    freq += ui->spinBox_k10->value()  * 100000;
    freq += ui->spinBox_k1->value()   * 10000;
    freq += ui->spinBox_100->value()  * 1000;
    freq += ui->spinBox_10->value()   * 100;
    freq += ui->spinBox_1->value()    * 10;
    freq += ui->spinBox_01->value();

    if(freq < 1) {
        AE20125gui::log("Frequency can not be 0Hz");
        ui->spinBox_01->setValue(1);
        freq = 1;
    }

    if(lastfreq != freq) {
        AE20125gui::sendCmd("A:" + QString::number(freq));
        lastfreq = freq;
    }
}

void AE20125gui::sendCmd(QString cmd) {
    AE20125gui::log("TX: 201:" + cmd + ":;");
    serial->write("201:" + cmd.toAscii ()+ ":;");
}

void AE20125gui::on_radioButton_wav_rect_clicked()
{
    limit = true;
    AE20125gui::validateFreq();
    sendCmd("B:2");
}


void AE20125gui::on_radioButton_wav_tri_clicked()
{
    limit = true;
    AE20125gui::validateFreq();
    sendCmd("B:1");
}

void AE20125gui::on_radioButton_wav_sine_clicked()
{
    limit = false;
    AE20125gui::validateFreq();
    sendCmd("B:0");
}

void AE20125gui::on_spinBox_m10_valueChanged(int arg1)
{
    AE20125gui::validateFreq();
}

void AE20125gui::on_spinBox_m1_valueChanged(int arg1)
{
    AE20125gui::validateFreq();
}

void AE20125gui::on_spinBox_k100_valueChanged(int arg1)
{
    AE20125gui::validateFreq();
}

void AE20125gui::on_spinBox_k10_valueChanged(int arg1)
{
    AE20125gui::validateFreq();
}

void AE20125gui::on_spinBox_k1_valueChanged(int arg1)
{
    AE20125gui::validateFreq();
}

void AE20125gui::on_spinBox_100_valueChanged(int arg1)
{
    AE20125gui::validateFreq();
}

void AE20125gui::on_spinBox_10_valueChanged(int arg1)
{
    AE20125gui::validateFreq();
}

void AE20125gui::on_spinBox_1_valueChanged(int arg1)
{
    AE20125gui::validateFreq();
}

void AE20125gui::on_spinBox_01_valueChanged(int arg1)
{
    AE20125gui::validateFreq();
}

void AE20125gui::on_spinBox_set_cal_valueChanged(int arg1)
{
    AE20125gui::sendCmd("I:"+QString::number(arg1));
}

void AE20125gui::on_doubleSpinBox_set_frq_valueChanged(double arg1)
{
    int freq = arg1 * 10;
    AE20125gui::sendCmd("H:"+QString::number(freq));
}

void AE20125gui::on_comboBox_set_wav_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "Sine")     AE20125gui::sendCmd("G:0");
    if(arg1 == "Triangle") AE20125gui::sendCmd("G:1");
    if(arg1 == "Square")   AE20125gui::sendCmd("G:2");
}

void AE20125gui::on_doubleSpinBox_sweep_start_valueChanged(double arg1)
{
    int freq = arg1 * 10;
    AE20125gui::sendCmd("J:"+QString::number(freq));
}

void AE20125gui::on_doubleSpinBox_sweep_stop_valueChanged(double arg1)
{
    int freq = arg1 * 10;
    AE20125gui::sendCmd("K:"+QString::number(freq));
}

void AE20125gui::on_doubleSpinBox_sweep_freq_valueChanged(double arg1)
{
    int freq = arg1 * 10;
    AE20125gui::sendCmd("L:"+QString::number(freq));
}

void AE20125gui::on_doubleSpinBox_pll_fact_valueChanged(double arg1)
{
    int freq = arg1 * 10;
    AE20125gui::sendCmd("E:"+QString::number(freq));
}

void AE20125gui::on_spinBox_pll_off_valueChanged(int arg1)
{
    AE20125gui::sendCmd("F:"+QString::number(arg1));
}

void AE20125gui::on_doubleSpinBox_mod_typ_freq_valueChanged(double arg1)
{
    int freq = arg1 * 10;
    AE20125gui::sendCmd("N:"+QString::number(freq));
}

void AE20125gui::on_doubleSpinBox_mod_typ_deg_valueChanged(double arg1)
{
    int freq = arg1 * 10;
    AE20125gui::sendCmd("O:"+QString::number(freq));
}

void AE20125gui::on_doubleSpinBox_mod_src_freq_valueChanged(double arg1)
{
    int freq = arg1 * 10;
    AE20125gui::sendCmd("Q:"+QString::number(freq));
}

void AE20125gui::on_radioButton_sweep_loop_clicked()
{
    AE20125gui::sendCmd("R:0");
}

void AE20125gui::on_radioButton_sweep_swing_clicked()
{
    AE20125gui::sendCmd("R:1");
}

void AE20125gui::readData() {

    QByteArray requestData = serial->readAll();
    while (serial->waitForReadyRead(10))
                    requestData += serial->readAll();

    QString line = QString(requestData);

    QStringList splitted = line.split(";");

    for(int i=0; i<splitted.size(); i++) {
        QStringList values = splitted.at(i).split(":");

        if(values.at(0) != "201") {
            //Empty or broken data
            continue;
        }

        if(values.at(1) == "U") {   //Keep Alive
            //AE20125gui::log("Keep Alive");
            continue;
        }

        AE20125gui::log("RX: " + splitted.at(i));

        if(values.at(1) == "A") {   //Frequency
            long freq = values.at(2).toDouble();
            int cnt=0;

            if(freq >= 100000000) {
                cnt = floor(freq / 100000000);
                freq -= cnt*100000000;
                ui->spinBox_m10->setValue(cnt);
            }

            if(freq >= 10000000) {
                cnt = floor(freq / 10000000);
                freq -= cnt*10000000;
                ui->spinBox_m1->setValue(cnt);
            }

            if(freq >= 1000000) {
                cnt = floor(freq / 1000000);
                freq -= cnt*1000000;
                ui->spinBox_k100->setValue(cnt);
            }

            if(freq >= 100000) {
                cnt = floor(freq / 100000);
                freq -= cnt*100000;
                ui->spinBox_k10->setValue(cnt);
            }

            if(freq >= 10000) {
                cnt = floor(freq / 10000);
                freq -= cnt*10000;
                ui->spinBox_k1->setValue(cnt);
            }

            if(freq >= 1000) {
                cnt = floor(freq / 1000);
                freq -= cnt*1000;
                ui->spinBox_100->setValue(cnt);
            }

            if(freq >= 100) {
                cnt = floor(freq / 100);
                freq -= cnt*100;
                ui->spinBox_10->setValue(cnt);
            }

            if(freq >= 10) {
                cnt = floor(freq / 10);
                freq -= cnt*10;
                ui->spinBox_1->setValue(cnt);
            }

            ui->spinBox_01->setValue(freq);
        }

        if(values.at(1) == "B") {   //Waveform
            if(values.at(2) == "0") ui->radioButton_wav_sine->click();
            if(values.at(2) == "1") ui->radioButton_wav_tri->click();
            if(values.at(2) == "2") ui->radioButton_wav_rect->click();
        }

        if(values.at(1) == "C") {   //Mode
            if(values.at(2) == "0") ui->radioButton_mode_norm->click();
            if(values.at(2) == "1") ui->radioButton_mode_sweep->click();
            if(values.at(2) == "2") ui->radioButton_mode_mod->click();
        }

        if(values.at(1) == "D") {   //PLL Reference
            if(values.at(2) == "0") ui->checkBox_pll_on->setChecked(false);
            if(values.at(2) == "1") ui->checkBox_pll_on->setChecked(true);
        }

        if(values.at(1) == "E") {   //PLL Factor
            double var = values.at(2).toDouble() / 10;
            ui->doubleSpinBox_pll_fact->setValue(var);
        }

        if(values.at(1) == "F") {   //PLL Offset
            ui->spinBox_pll_off->setValue(values.at(2).toInt());
        }

        if(values.at(1) == "G") {   //Startup Waveform
            ui->comboBox_set_wav->setCurrentIndex(values.at(2).toInt());
        }

        if(values.at(1) == "H") {   //Startup Frequency
            double var = values.at(2).toDouble() / 10;
            ui->doubleSpinBox_set_frq->setValue(var);
        }

        if(values.at(1) == "I") {   //Calibration Offset
            ui->spinBox_set_cal->setValue(values.at(2).toInt());
        }

        if(values.at(1) == "J") {   //Sweep Start Frequency
            double var = values.at(2).toDouble() / 10;
            ui->doubleSpinBox_sweep_start->setValue(var);
        }

        if(values.at(1) == "K") {   //Sweep Stop Frequency
            double var = values.at(2).toDouble() / 10;
            ui->doubleSpinBox_sweep_stop->setValue(var);
        }

        if(values.at(1) == "L") {   //Sweep Frequency
            double var = values.at(2).toDouble() / 10;
            ui->doubleSpinBox_sweep_freq->setValue(var);
        }

        if(values.at(1) == "M") {   //Modulation Type
            if(values.at(2) == "0") ui->radioButton_mod_typ_fsk->click();
            if(values.at(2) == "1") ui->radioButton_mod_typ_PSK->click();
        }

        if(values.at(1) == "N") {   //FSK Frequency
            double var = values.at(2).toDouble() / 10;
            ui->doubleSpinBox_mod_typ_freq->setValue(var);
        }

        if(values.at(1) == "O") {   //PSK Phase
            double var = values.at(2).toDouble() / 10;
            ui->doubleSpinBox_mod_typ_deg->setValue(var);
        }

        if(values.at(1) == "P") {   //Modulation Source
            if(values.at(2) == "0") ui->radioButton_mod_src_int->click();
            if(values.at(2) == "1") ui->radioButton_mod_src_ext->click();
        }

        if(values.at(1) == "Q") {   //Internal Modulation Frequency
            double var = values.at(2).toDouble() / 10;
            ui->doubleSpinBox_mod_src_freq->setValue(var);
        }

        if(values.at(1) == "R") {   //Sweep Mode
            if(values.at(2) == "0") ui->radioButton_sweep_loop->click();
            if(values.at(2) == "1") ui->radioButton_sweep_swing->click();
        }

        if(values.at(1) == "X") {
            ui->label_hw_hwv->setText(values.at(2));
        }

        if(values.at(1) == "Y") {
            ui->label_hw_fwv->setText(values.at(2));
        }

        if(values.at(1) == "Z") {
            ui->label_hw_pidv->setText(values.at(2));
        }

        //@todo presets (1-9,0)
    }
}
