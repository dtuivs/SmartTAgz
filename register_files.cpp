#include "register_files.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QString>
#include <QDebug>

const QString settingsFolder = QDir::homePath()+"/.register/";
const QString ticketLocation = settingsFolder+"currentList.txt";
const QString salesTaxLocation = settingsFolder+"tax.txt";
const QString defaultTicketFolderLocation = settingsFolder+"defaultTicketsFolder.txt";
const QString databaseLocation = settingsFolder+"databaselocation.txt";

const QString persistenceFilePath = settingsFolder+"persistence";

register_files::register_files() {

}

QString register_files::checkPersistenceFile(QString property){
    QFile persistenceFile(persistenceFilePath);

    if(!persistenceFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        return QString();
    }

    QTextStream in(&persistenceFile);
    QString propertyValue = "";
    while(!in.atEnd()){
        QString line = in.readLine();
        if(line.startsWith(property)){
            QStringList Splittext = line.split(": ");
            propertyValue = Splittext[1].trimmed();
        }
    }
    persistenceFile.close();
    return propertyValue;
}

void register_files::updatePersistenceFile(QString property, QString newValue){
    QString updatedProperty = QString("%1: %2").arg(property, newValue);
    QFile persistenceFile(persistenceFilePath);

    if(!persistenceFile.open(QIODevice::ReadOnly | QIODevice::Text)){

    }
    QTextStream in(&persistenceFile);
    QStringList NewPropertyList;
    bool found = false;

    while(!in.atEnd()){
        QString line = in.readLine();
        if(line.startsWith(property)){
            NewPropertyList.append(updatedProperty);
            found = true;
        }else{
                NewPropertyList.append(updatedProperty);
        }
    }
    if(!found){
        NewPropertyList.append(updatedProperty);
    }
    persistenceFile.close();
    QTextStream out(&persistenceFile);
    if(!persistenceFile.open(QIODevice::WriteOnly | QIODevice::Text)){

    }

    for(const QString &line : NewPropertyList){
        out << line << "\n";
    }
    persistenceFile.close();
    qDebug() << "Persistence updated:" + updatedProperty;

}

QString register_files::return_settingsFolder(){
    return settingsFolder;
}

QString register_files::return_ticketLocation(){
    return ticketLocation;
}

QString register_files::return_salesTaxLocation(){
    return salesTaxLocation;
}

QString register_files::return_defaultTicketFolderLocation(){
    return defaultTicketFolderLocation;
}

QString register_files::return_databaseLocation(){
    return databaseLocation;
}

QString register_files::checkNotedTicketFolder(){

    QFile note(defaultTicketFolderLocation);

    if(!note.exists()){
        qDebug() << "defaultTicketsFolder.txt not found";
        return QDir::homePath()+"/.register/";
    }

    if(!note.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Can't REad?! defaultTicketsFolder.txt";
        return QDir::homePath()+"/.register/";
    }

    QTextStream in(&note);
    QString defaultTicketFolder = in.readLine();
    note.close();
    return defaultTicketFolder;

}

QString register_files::checkNotedDatabase(){
    QFile note(databaseLocation);

    if(!note.exists()){
        qDebug() << "checkNotedDatabase.txt not found";
        return QDir::homePath()+"/.register/upc_database.txt";
    }

    if(!note.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Can't REad?! checkNotedDatabase.txt";
        return QDir::homePath()+"/.register/upc_database.txt";
    }

    QTextStream in(&note);
    QString databasePath = in.readLine();
    note.close();
    return databasePath;
}

QString register_files::checkNotedTax(){

    QFile note(salesTaxLocation);
    if(!note.exists()){
        qDebug() << "tax.txt not found";
        return "";
    }
    if(!note.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "coudln't load tax.txt";
        return "";
    }

    QTextStream in(&note);
    QString currentTax = in.readLine();
    note.close();

    return currentTax;
}

QString register_files::checkNotedTicket(){

    QFile note(ticketLocation);
    if(!note.exists()){
        qDebug() << "currentList.txt not found";
        return "";
    }
    if(!note.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "coudln't load currentList.txt";
        return "";
    }

    QTextStream in(&note);
    QString currentTicketPath = in.readLine();
    note.close();

    return currentTicketPath;
}

void register_files::noteCurrentTicketPath(const QString &ticketPath, const QString &Destination){
    QFile note(Destination);

    if(!note.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << ": could not load note";
        return;
    }

    QTextStream output(&note);
    output << ticketPath;
    note.close();
    //qDebug() << "current ticket noted.";
}

void register_files::updateDatabaseDescription(QString upc, QString description)
{
    QString updated_item;
    QString database_filepath = checkNotedDatabase();
    QFile database_file = database_filepath;

    if(!database_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "updateDatabase: oops, couldn't open database.";
    }

    QStringList lines;
    QTextStream in(&database_file);
    bool found = false;
    while(!in.atEnd()){
        QString line = in.readLine();
        if(line.startsWith(upc)){
            found = true;
            QStringList splitupclabel = line.split(" - ");
            QStringList itemdata = splitupclabel[1].trimmed().split(" #)- ");
            QString price = "";
            if(itemdata.size() > 1){
                price = itemdata[1].trimmed();
                updated_item = QString("%1 - %2 #)- %3").arg(upc, description, price);
                lines.append(updated_item);
            }else{
                updated_item = QString("%1 - %2").arg(upc, description);
                lines.append(updated_item);
            }
        }else{
            lines.append(line);
        }
    }
    if(!found){
        updated_item = QString("%1 - %2").arg(upc, description);
        lines.append(updated_item);
        qDebug() << "appending one.";
    }
    database_file.close();

    QTextStream out(&database_file);
    if(!database_file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "updateDatabase: oops, couldn't open database.";
    }

    for(const QString &line : lines){
        out << line << "\n";
    }
    database_file.close();
    qDebug() << "save successful: database";
}

void register_files::updateDatabasePrice(QString upc, QString price){

}

void register_files::updateDatabase(QString upc, QString description, QString price){
    QString updated_item;
    bool pricePresent = false;
    if(price == ""){
        updated_item = QString("%1 - %2").arg(upc, description);
    }else{
        pricePresent = true;
        updated_item = QString("%1 - %2 #)- price:%3").arg(upc, description, price);
    }
    qDebug()  << updated_item;
    QString database_filepath = checkNotedDatabase();
    QFile database_file = database_filepath;
    if(!database_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "updateDatabase: oops, couldn't open database.";
    }

    QStringList lines;
    QTextStream in(&database_file);
    bool found;
    while(!in.atEnd()){
        QString line = in.readLine();
        if(line.startsWith(upc)){
            QStringList splitupclabel = line.split(" - ");
            QStringList parts = splitupclabel[1].trimmed().split(" #)- ");


            if(parts.size() > 1 && !pricePresent){                          //check database for a price and whether input has new price
                QString databasePrice = parts[1].trimmed();
                updated_item = QString("%1 #)- %2").arg(updated_item, databasePrice);
            }

            lines.append(updated_item);

            found = true;
            qDebug() << "updating one.";

        }else{
            lines.append(line);
        }
    }
    if(!found){
        lines.append(updated_item);
        qDebug() << "appending one.";
    }
    database_file.close();

    QTextStream out(&database_file);
    if(!database_file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "updateDatabase: oops, couldn't open database.";
    }

    for(const QString &line : lines){
        out << line << "\n";
    }
    database_file.close();
    qDebug() << "save successful: database";
}
