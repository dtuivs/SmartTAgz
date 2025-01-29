#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include "register_files.h"

namespace Ui {
class settings;
}

class settings : public QWidget
{
    Q_OBJECT

public:
    explicit settings(QWidget *parent = nullptr);
    ~settings();

private slots:
    void on_ticketFolderLocate_button_clicked();

    void on_ticketFolder_input_returnPressed();

    void on_databaseLocate_button_clicked();

    void on_salesTax_input_textChanged(const QString &arg1);

    void on_databaseLocation_input_returnPressed();

    void on_settings_close_button_clicked();

private:
    Ui::settings *ui;
};

#endif // SETTINGS_H
