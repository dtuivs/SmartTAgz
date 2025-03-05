#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QFileSystemWatcher>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>
#include "register_files.h"
#include "settings.h"
#include "ticket_menu.h"

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

    void reload();

    void fileHasChanged();

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

    void on_code_inputBox_2_textChanged(const QString &arg1);

    void on_code_inputBox_2_returnPressed();

    void on_exportBarcodes_button_clicked();

private:
    Ui::MainWindow *ui;

    settings *settingsWindow = nullptr;

    ticket_menu *ticketMenu = nullptr;

    void addtoTicket(QString lookup_code);

    void cellChanged(int row, int column);

    void toggle_keypad(bool isGone);

    QImage generateBarcode(const QString &code);

    void exportBarcodesList();

    bool code_input_check(const QString &input);

    QString processPrice(const QString &price);

    QString processQty(const QString &quantity);

    void update_foundView(QString rawitem);

    QString plusOne(QString qty);

    void update_totalPrice();

    void searchDatabase(const QString &searchText, const QString &databasePath);

    void autosave();

    QFileSystemWatcher *fileWatcher;

    void digit_clicked(QString digit);

    void on_actionSettings_clicked();

    void launchTicketsMenu();

    void chooseTicketFile();

    QString expandtoUPCA(const QString &upc);

    QTimer *reloadTimer;

    bool isWriting;
};
#endif // MAINWINDOW_H
