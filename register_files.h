#ifndef REGISTER_FILES_H
#define REGISTER_FILES_H
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QTextStream>

class register_files
{
public:
    register_files();

    QString generateTimestamp();

    bool checkFileExistence(QString filePath);

    QString checkPersistenceFile(QString property);

    void updatePersistenceFile(QString property, QString newValue);

    QString return_settingsFolder();

    QString return_ticketLocation();

    void updateDatabaseDescription(QString upc, QString description);

    void updateDatabasePrice(QString upc, QString price);

    void updateDatabase(QString upc, QString description, QString price);
};

#endif // REGISTER_FILES_H
