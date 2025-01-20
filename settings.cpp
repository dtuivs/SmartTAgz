#include "settings.h"
#include "ui_settings.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QString>
#include "register_files.h"

register_files cabinet;
settings::settings(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , ui(new Ui::settings)
{
    ui->setupUi(this);
    setWindowTitle("Settings");
    ui->ticketFolder_input->setText(cabinet.checkNotedTicketFolder());
    ui->databaseLocation_input->setText(cabinet.checkNotedDatabase());
    ui->CurrentTicketPath_label->setText(cabinet.checkNotedTicket());
    ui->salesTax_input->setText(cabinet.checkNotedTax());

}

settings::~settings()
{
    delete ui;
}

void settings::update_ticketFolderLocation(QString newFolderPath){

    QFile file(cabinet.return_defaultTicketFolderLocation());

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "Couldn't find defaultTicketsFolder.txt";
    }

    QTextStream output(&file);

    output << newFolderPath;
    ui->ticketFolder_input->setText(newFolderPath);

    file.close();
    qDebug() << "Ticket Folder updated.";
}

void settings::update_databaseLocation(QString newDatabaseLocation){

    QFile file(cabinet.return_databaseLocation());

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "Coudln't find databaselocation.txt";
    }

    QTextStream output(&file);

    output << newDatabaseLocation;
    ui->databaseLocation_input->setText(newDatabaseLocation);
    file.close();
    qDebug() << "Database location updated.";
}

void settings::on_ticketFolderLocate_button_clicked()
{
    QString ticketFolder = QFileDialog::getExistingDirectory();

    update_ticketFolderLocation(ticketFolder);
}


void settings::on_ticketFolder_input_returnPressed()
{
    QString input = ui->ticketFolder_input->text();
    update_ticketFolderLocation(input);
}


void settings::on_databaseLocate_button_clicked()
{
    QString databaseNewLocation = QFileDialog::getOpenFileName();
    update_databaseLocation(databaseNewLocation);
}


void settings::on_salesTax_input_textChanged(const QString &arg1)
{
    QFile file(cabinet.return_salesTaxLocation());
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "Coudln't find tax";
    }

    QTextStream output(&file);

    output << arg1;
    file.close();
    qDebug() << "Tax updated";
}



void settings::on_databaseLocation_input_returnPressed()
{
    QString input = ui->databaseLocation_input->text();
    update_databaseLocation(input);
}
