#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    void on_one_button_clicked();

    void on_load_button_clicked();

    void loadFile(QString filename);

    void noteCurrentPath(const QString &filepath, const QString &destinationPath);

    void saveFile(QTableWidget *given_table, const QString &filePath);

    void on_code_inputBox_textChanged(const QString &arg1);

    void on_code_inputBox_returnPressed();

    void on_two_Button_clicked();

    void on_three_button_clicked();

    void on_four_button_clicked();

    void on_five_button_clicked();

    void on_six_button_clicked();

    void on_seven_button_clicked();

    void on_eight_button_clicked();

    void on_nine_button_clicked();

    void on_zero_button_clicked();

    void on_period_button_clicked();

    void on_backspace_button_clicked();

    void on_enter_button_clicked();

    void on_newticket_button_clicked();

    void on_save_button_clicked();

    void on_searchbar_input_textChanged(const QString &arg1);

    void on_delete_button_clicked();

    void on_receipt_button_clicked();

    void on_addtoticket_button_clicked();

    void on_barcode_button_clicked();

    void on_priceChange_button_clicked();

    void on_qtyChange_button_clicked();

private:
    Ui::MainWindow *ui;

    void addtoTicket(QString lookup_code);

    void cellChanged(int row, int column);

    QImage generateBarcode(const QString &code);

    void updateDatabase(QString upc, QString description, QString price);

    bool code_input_check(const QString &input);

    QString processPrice(const QString &price);

    QString generateTimestamp();

    QString processQty(const QString &quantity);

    QString checkNotedTicketFolder();

    QString checkNotedDatabase();

    QString checkNotedTax();

    QString checkNotedTicket();

    void update_foundView(QString rawitem);

    QString plusOne(QString qty);

    void update_totalPrice();

    void searchDatabase(const QString &searchText, const QString &databasePath);

    void autosave();

    void digit_clicked(QString digit);

    void on_actionSales_Tax_clicked();

    void on_actionLocate_Database_clicked();

    void on_actionTicketFolder_clicked();

    QString expandtoUPCA(const QString &upc);

};
#endif // MAINWINDOW_H
