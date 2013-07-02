#ifndef AE20125GUI_H
#define AE20125GUI_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class AE20125gui;
}

class AE20125gui : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit AE20125gui(QWidget *parent = 0);
    ~AE20125gui();
    
private slots:
    void on_pushButton_con_connect_clicked();

    void on_pushButton_con_about_clicked();

    void on_radioButton_mode_sweep_clicked();

    void on_radioButton_mode_mod_clicked();

    void on_radioButton_mode_norm_clicked();

    void on_radioButton_mod_typ_PSK_clicked();

    void on_radioButton_mod_typ_fsk_clicked();

    void on_radioButton_mod_src_int_clicked();

    void on_radioButton_mod_src_ext_clicked();

    void on_checkBox_pll_on_stateChanged(int arg1);

    void on_spinBox_m10_valueChanged(int arg1);

    void on_spinBox_m1_valueChanged(int arg1);

    void on_spinBox_k100_valueChanged(int arg1);

    void on_spinBox_k10_valueChanged(int arg1);

    void on_spinBox_k1_valueChanged(int arg1);

    void on_spinBox_100_valueChanged(int arg1);

    void on_spinBox_10_valueChanged(int arg1);

    void on_spinBox_1_valueChanged(int arg1);

    void on_spinBox_01_valueChanged(int arg1);

    void on_radioButton_wav_sine_clicked();

    void on_spinBox_set_cal_valueChanged(int arg1);

    void on_doubleSpinBox_set_frq_valueChanged(double arg1);

    void on_comboBox_set_wav_currentIndexChanged(const QString &arg1);

    void on_doubleSpinBox_sweep_start_valueChanged(double arg1);

    void on_doubleSpinBox_sweep_stop_valueChanged(double arg1);

    void on_doubleSpinBox_sweep_freq_valueChanged(double arg1);

    void on_doubleSpinBox_pll_fact_valueChanged(double arg1);

    void on_spinBox_pll_off_valueChanged(int arg1);

    void on_doubleSpinBox_mod_typ_freq_valueChanged(double arg1);

    void on_doubleSpinBox_mod_typ_deg_valueChanged(double arg1);

    void on_doubleSpinBox_mod_src_freq_valueChanged(double arg1);

    void on_radioButton_sweep_loop_clicked();

    void on_radioButton_sweep_swing_clicked();

    void readData();

    void Sleep(int ms);

    void on_radioButton_wav_tri_clicked();

    void on_radioButton_wav_rect_clicked();

private:
    Ui::AE20125gui *ui;
    void log(QString msg);
    void validateFreq();
    void sendCmd(QString cmd);
    QSerialPort *serial;
};

#endif // AE20125GUI_H
