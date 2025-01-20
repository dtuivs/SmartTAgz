#ifndef REGISTER_FILES_H
#define REGISTER_FILES_H
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QString>
#include <QDebug>

class register_files
{
public:
    register_files();

    QString checkPersistenceFile(QString property);

    void updatePersistenceFile(QString property, QString newValue);

    QString return_settingsFolder();

    QString return_ticketLocation();

    QString return_defaultTicketFolderLocation();

    QString return_salesTaxLocation();

    QString return_databaseLocation();

    QString checkNotedTicketFolder();

    QString checkNotedDatabase();

    QString checkNotedTax();

    QString checkNotedTicket();

    void noteCurrentTicketPath(const QString &ticketPath, const QString &Destination);

    void updateDatabaseDescription(QString upc, QString description);

    void updateDatabasePrice(QString upc, QString price);

    void updateDatabase(QString upc, QString description, QString price);
};

#endif // REGISTER_FILES_H
