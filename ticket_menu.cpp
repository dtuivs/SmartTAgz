#include "ticket_menu.h"
#include "ui_ticket_menu.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QString>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTextStream>
#include "register_files.h"

register_files ticketCabinet;


ticket_menu::ticket_menu(QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::ticket_menu)
{
    ui->setupUi(this);
    setWindowTitle("Choose Ticket:");
    connect(ui->ticketChooser_view, &QTableWidget::cellDoubleClicked, this, &ticket_menu::setTicket);
    ui->ticketChooser_view->setSortingEnabled(true);
    fetchTickets(ticketCabinet.checkPersistenceFile("TICKET_FOLDER_LOCATION"));
}

ticket_menu::~ticket_menu()
{
    delete ui;
}

void ticket_menu::fetchTickets(QString folder){
    QDir ticketsFolder(folder);

    if(!ticketsFolder.exists()){

    }
    QStringList filters;
    filters << "*_List.txt";
    QFileInfoList txtFiles = ticketsFolder.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Time);

    for(QFileInfo file : txtFiles){
        int row = ui->ticketChooser_view->rowCount();
        ui->ticketChooser_view->insertRow(row);
        QTableWidgetItem *fileLabel = new QTableWidgetItem(file.fileName());
        QTableWidgetItem *filemodified = new QTableWidgetItem(file.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
        ui->ticketChooser_view->setItem(row, 0, fileLabel);
        ui->ticketChooser_view->setItem(row, 1, filemodified);
        filemodified->setFlags(filemodified->flags() & ~Qt::ItemIsSelectable);
        fileLabel->setFlags(fileLabel->flags() & ~Qt::ItemIsEditable);
    }
    ui->ticketChooser_view->resizeColumnsToContents();
}

void ticket_menu::setTicket(){
    QTableWidgetItem *selectedItem = ui->ticketChooser_view->currentItem();
    if(selectedItem){
        QString fullTicketPath = ticketCabinet.checkPersistenceFile("TICKET_FOLDER_LOCATION") + "/" +selectedItem->text();
        ticketCabinet.updatePersistenceFile("CURRENT_TICKET_LOCATION", fullTicketPath);
        emit selectedTicket(fullTicketPath);
        this->close();
    }
}

void ticket_menu::on_buttonBox_accepted()
{
    setTicket();
}

