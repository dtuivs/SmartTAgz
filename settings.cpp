#include "settings.h"
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QTextStream>
#include <QWidget>
#include "register_files.h"
#include "ui_settings.h"

register_files cabinet;
settings::settings(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::settings)
{
    ui->setupUi(this);
    setWindowTitle("Settings");
    ui->ticketFolder_input->setText(cabinet.checkPersistenceFile("TICKET_FOLDER_LOCATION"));
    ui->databaseLocation_input->setText(cabinet.checkPersistenceFile("DATABASE_FILE_LOCATION"));
    ui->CurrentTicketPath_label->setText(cabinet.checkPersistenceFile("CURRENT_TICKET_LOCATION"));
    ui->salesTax_input->setText(cabinet.checkPersistenceFile("SALES_TAX_PERCENT"));
    connect(ui->settings_close_button, &QPushButton::clicked, this, &QWidget::close);
}

settings::~settings()
{
    delete ui;
}

void settings::on_ticketFolderLocate_button_clicked()
{
    QString ticketFolder = QFileDialog::getExistingDirectory();
    if(ticketFolder != nullptr){
        cabinet.updatePersistenceFile("TICKET_FOLDER_LOCATION", ticketFolder);
        ui->ticketFolder_input->setText(ticketFolder);
    }
}

void settings::on_ticketFolder_input_returnPressed()
{
    QString input = ui->ticketFolder_input->text();
    cabinet.updatePersistenceFile("TICKET_FOLDER_LOCATION", input);
}

void settings::on_databaseLocate_button_clicked()
{
    QString databaseNewLocation = QFileDialog::getOpenFileName();
    if(databaseNewLocation != nullptr){
        cabinet.updatePersistenceFile("DATABASE_FILE_LOCATION", databaseNewLocation);
        ui->databaseLocation_input->setText(databaseNewLocation);
    }
}

void settings::on_salesTax_input_textChanged(const QString &arg1)
{
    cabinet.updatePersistenceFile("SALES_TAX_PERCENT", arg1);
    qDebug() << "Tax updated";
}

void settings::on_databaseLocation_input_returnPressed()
{
    QString input = ui->databaseLocation_input->text();
    cabinet.updatePersistenceFile("DATABASE_FILE_LOCATION", input);
}

void settings::on_settings_close_button_clicked() {}
