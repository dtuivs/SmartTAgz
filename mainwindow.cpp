#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QWidget>
#include <QString>
#include <QLabel>
#include <QImage>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDateTime>
#include <QDebug>
#include <zint.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->ticketview, &QTableWidget::cellChanged, this, &MainWindow::update_totalPrice);
    loadFile(checkNotedTicket());
    connect(ui->actionSales_Tax, &QAction::triggered, this, &MainWindow::on_actionSales_Tax_clicked);
    connect(ui->actionLocate_Database, &QAction::triggered, this, &MainWindow::on_actionLocate_Database_clicked);
    connect(ui->actionTickets_Folder, &QAction::triggered, this, &MainWindow::on_actionTicketFolder_clicked);
    connect(ui->ticketview, &QTableWidget::cellChanged, this, &MainWindow::cellChanged);

    QRegularExpression input_pattern(R"(^\d{8}|\d{12}$)");
    QRegularExpressionValidator *input_check = new QRegularExpressionValidator(input_pattern, this);
    ui->code_inputBox->setValidator(input_check);

    ui->enter_button->setDisabled(true);

    ui->ticketview->resizeColumnsToContents();
}

MainWindow::~MainWindow()
{
    delete ui;
}


QRegularExpression istwelveLong_digits(R"(^\d{12}$)");
QRegularExpression isEightLong_digits(R"(^0\d{7}$)");
QRegularExpression isPrice_one(R"(^\$\d+(\.\d{1,2})?$)");
QRegularExpression isPrice_two(R"(^price\:\d+(\.\d{1,2})?$)");
QRegularExpression isQty(R"(^x\d+$)");

const QString settingsFolder = QDir::homePath()+"/.register/";
const QString ticketLocation = settingsFolder+"currentList.txt";
const QString salesTaxLocation = settingsFolder+"tax.txt";
const QString defaultTicketFolderLocation = settingsFolder+"defaultTicketsFolder.txt";
const QString databaseLocation = settingsFolder+"databaselocation.txt";

void MainWindow::addtoTicket(QString lookup_code){

    //first check ticket view
    bool found;
    for(int row = 0; row < ui->ticketview->rowCount(); ++row){
        QString table_upc = ui->ticketview->item(row, 0)->text();
        //qDebug() << lookup_code + " : " + table_upc;
        if(table_upc == lookup_code){
            found = true;
            QString item_start_qty = ui->ticketview->item(row, 2)->text();
            QString updated_qty = plusOne(item_start_qty);

            QTableWidgetItem* updated_qty_label = new QTableWidgetItem(updated_qty);
            ui->ticketview->setItem(row, 2, updated_qty_label);
        }
    }
    //if not found then check database
    if(!found){

        qDebug() << "Checking database.";
        QString databasefilepath = checkNotedDatabase();
        QFile databaseFile(databasefilepath);
        if(!databaseFile.open(QIODevice::ReadOnly | QIODevice::Text)){
            qDebug() << "RETURNPRESSED: coulndt open database.";
        }

        QTextStream database(&databaseFile);
        int foundcount = 0;
        while(!database.atEnd()){
            QString line = database.readLine();
            if(line.startsWith(lookup_code)){
                foundcount++;
                int row = ui->ticketview->rowCount();
                ui->ticketview->insertRow(row);

                QStringList splitupclabel = line.split(" - ");
                QString upc = splitupclabel[0].trimmed();
                QTableWidgetItem* upctext = new QTableWidgetItem(upc);
                ui->ticketview->setItem(row, 0, upctext);
                ui->ticketview->item(row, 0)->setFlags(ui->ticketview->item(row, 0)->flags() & ~Qt::ItemIsEditable);

                qDebug() << "upc added: "  + upc;
                QStringList parts = splitupclabel[1].trimmed().split(" #)- ");

                QString label = parts[0].trimmed();
                QTableWidgetItem* labeltext = new QTableWidgetItem(label);
                ui->ticketview->setItem(row, 1, labeltext);
                qDebug() << "label added: " + label;
                QTableWidgetItem* view_qty = new QTableWidgetItem("1");
                ui->ticketview->setItem(row, 2, view_qty);

                if(parts.size() > 1){
                    QString price = parts[1].trimmed();
                    QTableWidgetItem* pricetext = new  QTableWidgetItem(processPrice(price));
                    ui->ticketview->setItem(row, 3, pricetext);
                }

            }
        }
        //If not in database add empty row
        if(foundcount == 0){
            int row = ui->ticketview->rowCount();
            ui->ticketview->insertRow(row);
            QTableWidgetItem* upc = new QTableWidgetItem(lookup_code);
            ui->ticketview->setItem(row, 0, upc);
            QTableWidgetItem* view_qty = new QTableWidgetItem("1");
            ui->ticketview->setItem(row, 2, view_qty);
        }
    }
}

QImage MainWindow::generateBarcode(const QString &code){


    struct zint_symbol *symbol = ZBarcode_Create();

    if(!symbol){
        qWarning("Failed making symbols!");
        return QImage();
    }
    if(code.size() == 8){
        symbol->symbology = BARCODE_UPCE;
        symbol->height = 250;
        symbol->width = 230;
        symbol->whitespace_width = 10;
        symbol->border_width = 2;
    }else if(code.size() == 12){
        symbol->symbology = BARCODE_UPCA;
        symbol->height = 250;
        symbol->width = 350;
        symbol->whitespace_width = 10;
        symbol->border_width = 2;
    }


    QByteArray codeData = code.toUtf8();
    int error = ZBarcode_Encode_and_Buffer(symbol, (unsigned char *)codeData.constData(), 0, 90);

    if(error != 0){
        qWarning("failed to encode upc");
        ZBarcode_Delete(symbol);
        return QImage();
    }

    QImage barcodeImage(symbol->bitmap_width, symbol->bitmap_height, QImage::Format_ARGB32);
    barcodeImage.fill(Qt::white);

    for(int y= 0; y < symbol->bitmap_height; ++y){
        for(int x=0;x < symbol->bitmap_width; ++x){
            barcodeImage.setPixelColor(x, y, symbol->bitmap[y*symbol->bitmap_width+x] ? Qt::black : Qt::white);
        }
    }

    ZBarcode_Delete(symbol);
    return barcodeImage;
}

void MainWindow::cellChanged(int row, int column){
    qDebug() << "Cell Changed -- row: " + QString::number(row) + ", Column: " + QString::number(column);
    if(column == 2){
        //update_totalPrice();
    }else if (column == 1 || column == 3){
        qDebug() << row;
        if(column==3){
        //    update_totalPrice();
        }
        qDebug() << ui->ticketview->item(row, 1)->text();
        if(ui->ticketview->item(row, 1)->text() != nullptr || ui->ticketview->item(row, 1)->text() != "label"){
            QString description = ui->ticketview->item(row, 1)->text();
            QString upc = ui->ticketview->item(row, 0) ? ui->ticketview->item(row, 0)->text() : "noupc";
            QString price = ui->ticketview->item(row, 3) ? ui->ticketview->item(row, 3)->text() : "";

            updateDatabase(upc, description, price);
        }else{
            qDebug() << "Bloop: cellchanged ticketcheck";
        }
    }
    autosave();
    ui->ticketview->resizeColumnsToContents();
}

void MainWindow::updateDatabase(QString upc, QString description, QString price){
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

void MainWindow::autosave(){
    QString currentTicket = checkNotedTicket();
    saveFile(ui->ticketview, currentTicket);
}

void MainWindow::update_totalPrice(){
    float total = 0.0;
    for(int row = 0; row < ui->ticketview->rowCount(); ++row){
        //qDebug() << "checking row: " + QString::number(row);
        bool converted = false;
        QString table_qty;
        float qty;
        if (ui->ticketview->item(row, 2) != nullptr){                       //checking QTY if empty
            table_qty = ui->ticketview->item(row, 2)->text();
            //qDebug() << "Qty:"+table_qty;
            if(table_qty == nullptr){
                qty = 1;
            }else{
                qty = table_qty.toFloat();
            }
        }else {
            qDebug() << "CHeck price: nullptr";
        }
        if(ui->ticketview->item(row, 3) != nullptr){                        //checking price if empty
            QString table_price = ui->ticketview->item(row, 3)->text();
            //qDebug() << "Price:" + table_price;
            float plusthis;
            if(table_price == nullptr){
                plusthis = 0.0;
            }else{
                plusthis = table_price.toFloat(&converted);
            }
            if(converted){
                plusthis = plusthis * qty;
                total = total + plusthis;
            }
        }
    }
    //qDebug() << total;
    float tax = checkNotedTax().toFloat();
    float tax_cut = tax/100;
    tax_cut = tax_cut*total;
    total = total + tax_cut;
    if(ui->price_label){
        QString taxcut_label =  "+$"+QString::number(tax_cut, 'f', 2)+"("+QString::number(tax, 'f', 2)+"%)";
        ui->tax_label->setText(taxcut_label);
        QString total_label = "Total: $" + QString::number(total, 'f', 2);
        ui->price_label->setText(total_label);

        qDebug() << total_label;
    }

}

QString MainWindow::processPrice(const QString &price){
    if(isPrice_one.match(price).hasMatch()){
        return price.mid(1);
    }else if(isPrice_two.match(price).hasMatch()){
        return price.mid(6);
    }else {
        return QString();
    }
}

QString MainWindow::processQty(const QString &quantity){
    if(isQty.match(quantity).hasMatch()){
        return quantity.mid(1);
    }else{
        return QString();
    }
}

QString MainWindow::checkNotedTicketFolder(){
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

QString MainWindow::checkNotedDatabase(){
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

QString MainWindow::checkNotedTax(){

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

QString MainWindow::checkNotedTicket(){

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

void MainWindow::loadFile(QString filename){
    if(filename.isEmpty()){
        return;
    }
    QFile file(filename);
    QFileInfo ticketData(file);
    QString cleanTitle = ticketData.fileName();
    setWindowTitle(cleanTitle);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Coudln't load file: " + filename;
        return;
    }

    ui->ticketview->setRowCount(0);

    QTextStream in(&file);
    while(!in.atEnd()){
        QString pattern = " #)- ";
        QString line = in.readLine();
        QStringList parts = line.split(pattern);
        int row = ui->ticketview->rowCount();
        ui->ticketview->insertRow(row);


        QString upc = parts[0].trimmed();
        QTableWidgetItem* item_upc = new QTableWidgetItem(upc);
        ui->ticketview->setItem(row, 0, item_upc);
        item_upc->setFlags(item_upc->flags() & ~Qt::ItemIsEditable);

        if(parts.size() >= 2){

            QString label = parts[1].trimmed();
            qDebug() << label;
            QTableWidgetItem* item_label = new QTableWidgetItem(label);
            ui->ticketview->setItem(row, 1, item_label);
            //item_label->setFlags(item_label->flags() & ~Qt::ItemIsEditable);
        }
        if(parts.size()>=3){
            QString quantity = parts[2].trimmed();
            QString qtyProcessed = processQty(quantity);
            qDebug() << quantity;
            QTableWidgetItem* item_qty = new QTableWidgetItem(qtyProcessed);
            ui->ticketview->setItem(row, 2, item_qty);
        }
        if(parts.size() >= 4){
            QString price = parts[3].trimmed();
            QString priceProcessed = processPrice(price);
            qDebug() << priceProcessed;
            QTableWidgetItem* item_price = new QTableWidgetItem(priceProcessed);
            ui->ticketview->setItem(row, 3, item_price);
        }
    }

}

void MainWindow::saveFile(QTableWidget *given_table, const QString &filePath){
    QFile ticket_filePath(filePath);

    if(!ticket_filePath.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "could not open file";
    }

    QTextStream output(&ticket_filePath);

    for(int row = 0; row < given_table->rowCount(); ++row){
        QString upc = given_table->item(row, 0) ? given_table->item(row, 0)->text() : "no_upc";
        QString label = given_table->item(row,1) ? given_table->item(row, 1)->text() : "label";
        QString qty = given_table->item(row, 2) ? "x" + given_table->item(row,2)->text() : "x1";
        QString price = "";
        QString formatted_line = "";
        if(given_table->item(row, 3)){
            price = given_table->item(row, 3)->text();
            formatted_line = QString("%1 #)- %2 #)- %3 #)- $%4").arg(upc, label, qty, price);
        }else {
            formatted_line = QString("%1 #)- %2 #)- %3").arg(upc, label, qty);
        }
        qDebug() << formatted_line;
        output << formatted_line << "\n";
    }
    ticket_filePath.close();
    qDebug() << "Save Successful.";
}

void MainWindow::noteCurrentPath(const QString &filepath, const QString &destinationPath){
    QFile note(destinationPath);

    if(!note.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << ": could not load note";
        return;
    }

    QTextStream output(&note);
    output << filepath;
    note.close();
    qDebug() << "current ticket noted.";
}

void MainWindow::searchDatabase(const QString &SearchText, const QString &databasePath){
    QFile databasefile(databasePath);

    ui->foundView->setRowCount(0);

    if(!databasefile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "couldnt open database";
    }

    QTextStream database(&databasefile);
    while(!database.atEnd()){
        QString line = database.readLine();
        if(line.contains(SearchText, Qt::CaseInsensitive)){
            //function should be expecting a line of text formatted: UPC: descrption #)- price:0.00
            update_foundView(line);
        }
    }
    databasefile.close();

    ui->foundView->resizeColumnsToContents();
}

QString MainWindow::expandtoUPCA(const QString &upc){
    QString base = upc.length() == 8 ? upc.mid(1, 6) : upc;
    QString checksum = upc.length() == 8 ? upc[7] : '0';

    if(base.length() != 6){
        return "-1";
    }

    QChar lastDigit = base[5];
    QString expanded = "";

    if(lastDigit == '0' || lastDigit == '1' || lastDigit == '2'){
        expanded = base.left(2) + lastDigit + "0000"  + base.mid(2, 3);
    }else if( lastDigit == '3'){
        expanded = base.left(3) + "00000" + base.mid(3, 2);
    } else if( lastDigit == '4'){
        expanded = base.left(4) + "00000" + base[4];
    }else{
        expanded = base.left(5)+ "0000" + lastDigit;
    }
    return "0" + expanded + checksum;
}


bool MainWindow::code_input_check(const QString &input){

    QString code = input;
    QRegularExpressionMatch twelveLong = istwelveLong_digits.match(code);
    QRegularExpressionMatch eightLong = isEightLong_digits.match(code);

    qDebug() << code + " : " + QString::number(code.size());
    if(!eightLong.hasMatch()){
        if(!twelveLong.hasMatch()){
            return false;
        }
    }else{
        qDebug() << expandtoUPCA(code);
        code = expandtoUPCA(code);
    }

    int sum = 0;
    for(int i = 0; i<11;++i){
        int digit = code[i].digitValue();
        sum += (i%2 == 0) ? digit *  3 : digit;
    }
    int last_digit = code[11].digitValue();
    int  checksum = (10 - (sum %  10)) % 10;
    qDebug() << QString::number(checksum) + " ? " + QString::number(last_digit);
    if(checksum == last_digit){
        return true;
    } else {
        return false;
    }

}

void MainWindow::on_code_inputBox_textChanged(const QString &arg1)
{
    ui->enter_button->setDisabled(!code_input_check(arg1));
}


void MainWindow::update_foundView(QString rawitem){ //function should be expecting a line of text formatted: UPC: descrption #)- price:0.00
    int row = ui->foundView->rowCount();
    ui->foundView->insertRow(row);

    QStringList splitupcLabel = rawitem.split(" - ");
    QString upc = splitupcLabel[0].trimmed();
    QTableWidgetItem* upctext = new QTableWidgetItem(upc);
    ui->foundView->setItem(row, 0, upctext);

    QStringList parts = splitupcLabel[1].trimmed().split(" #)- ");

    QString label = parts[0].trimmed();
    QTableWidgetItem* labeltext = new QTableWidgetItem(label);
    ui->foundView->setItem(row, 1, labeltext);

    if(parts.size() > 1){
        QString price = parts[1].trimmed();
        QTableWidgetItem* pricetext = new QTableWidgetItem(processPrice(price));
        ui->foundView->setItem(row, 2, pricetext);
    }
}

QString MainWindow::plusOne(QString qty){
    bool converted;

    qDebug() << qty;
    int plus = qty.toInt(&converted);

    if(converted){
        plus += 1;
        QString qty_label = QString::number(plus);
        qDebug() << qty_label;
        return qty_label;
    }else {
        return "";
    }
}



void MainWindow::on_code_inputBox_returnPressed()
{
    QString given = ui->code_inputBox->text();

    if(given.size() != 0){
        addtoTicket(given);

        ui->code_inputBox->setText("");
        ui->ticketview->resizeColumnsToContents();
    }
}

void MainWindow::on_one_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "1";
    ui->code_inputBox->setText(given);
}

void MainWindow::on_two_Button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "2";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_three_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "3";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_four_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "4";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_five_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "5";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_six_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "6";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_seven_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "7";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_eight_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "8";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_nine_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "9";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_zero_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += "0";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_period_button_clicked()
{
    QString given = ui->code_inputBox->text();
    given += ".";
    ui->code_inputBox->setText(given);
}


void MainWindow::on_backspace_button_clicked()
{
    QString given = ui->code_inputBox->text();
    ui->code_inputBox->setText(given.chopped(1));
}


void MainWindow::on_enter_button_clicked()
{
    on_code_inputBox_returnPressed();
}

QString MainWindow::generateTimestamp(){
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timestamp = currentTime.toString("yyyyMMMddhhmm");
    return timestamp;

}


void MainWindow::on_newticket_button_clicked()
{
    QDir folderPath(settingsFolder);
    if(!folderPath.exists()){
        folderPath.mkpath(".");
    }
    QString newfilename = generateTimestamp() + "_List.txt";
    noteCurrentPath( checkNotedTicketFolder()+"/"+newfilename, ticketLocation);
    ui->ticketview->setRowCount(0);
    setWindowTitle(newfilename);
}

void MainWindow::on_load_button_clicked()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, "Load Ticket", defaultTicketFolderLocation, nullptr);
    noteCurrentPath(filename, ticketLocation);
    loadFile(filename);
}


void MainWindow::on_save_button_clicked()
{
    QString ticketDestinaton = checkNotedTicket();
    saveFile(ui->ticketview, ticketDestinaton);
}


void MainWindow::on_searchbar_input_textChanged(const QString &arg1)
{
    QString databasefilePath = checkNotedDatabase();
    searchDatabase(arg1, databasefilePath);
}


void MainWindow::on_delete_button_clicked()
{
    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();

    if(selectItems.isEmpty()){
        qDebug() << "no selected items";
    }else{
        for(QTableWidgetItem *item : selectItems){
            if(item){
                int removalRow =  item->row();
                ui->ticketview->removeRow(removalRow);
            }
        }
    }
    emit ui->ticketview->cellChanged(0, 2);
    autosave();
}


void MainWindow::on_receipt_button_clicked()
{
    QString homePath = QDir::homePath();
    QFile savePath = QFileDialog::getSaveFileName(nullptr, generateTimestamp()+"_receipt", homePath+"/Downloads/"+generateTimestamp()+"_receipt", nullptr) + ".txt";
    if(!savePath.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "couldn't do it.";
    }
    QTextStream output(&savePath);

    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();
    if(selectItems.isEmpty()){
        qDebug() << "nothing selected: full receipt";
        for(int row = 0; row < ui->ticketview->rowCount(); ++row){
            QString upc = ui->ticketview->item(row, 0) ? ui->ticketview->item(row, 0)->text() : "";
            QString description = ui->ticketview->item(row, 1)  ? ui->ticketview->item(row, 1)->text() : "item";
            QString qty = ui->ticketview->item(row, 2) ? ui->ticketview->item(row, 2)->text() : "1";
            QString price = ui->ticketview->item(row, 3) ? ui->ticketview->item(row, 3)->text() : "0.00";

            QString formatted_line = QString("%1 %2 @$%3(x%4)").arg(upc, description, price, qty);
            qDebug() << formatted_line ;
            output << formatted_line << "\n";
        }
    }else{
        qDebug() << "limited receipt";
        for(QTableWidgetItem *row_item : selectItems){
            if(row_item){
                int row = row_item->row();
                qDebug()  << row_item->text();
                QString upc = ui->ticketview->item(row, 0) ? ui->ticketview->item(row, 0)->text() : "no_upc";
                QString description = ui->ticketview->item(row, 1) ? ui->ticketview->item(row, 1)->text() : "label";
                QString qty = ui->ticketview->item(row, 2) ? ui->ticketview->item(row, 2)->text() : "1";
                QString price = ui->ticketview->item(row, 3) ? ui->ticketview->item(row, 3)->text() : "0";

                QString formatted_line = QString("%1 %2 @$%3(x%4)").arg(upc, description, price, qty);

                output << formatted_line << "\n";
            }
        }
    }
    savePath.close();
    qDebug() << "Export successful.";

}


void MainWindow::on_addtoticket_button_clicked()
{
    QList<QTableWidgetItem *> selectItems = ui->foundView->selectedItems();

    if(selectItems.isEmpty()){
        qDebug() << "addtoticket: Nothing selected";
    }else{
        for(QTableWidgetItem *row_item : selectItems){
            qDebug() << row_item->row();
            int row = row_item->row();
            QString upc = ui->foundView->item(row, 0)->text();
            addtoTicket(upc);
        }
    }
}


void MainWindow::on_barcode_button_clicked()
{
    QString incoming;
    QList<QTableWidgetItem *>  selectItems = ui->ticketview->selectedItems();
    if(selectItems.isEmpty()){
        incoming = ui->ticketview->item(0,0)->text();
    }else{
        int row = selectItems[0]->row();
        incoming = ui->ticketview->item(row, 0)->text();
    }
    QPixmap image = QPixmap::fromImage(generateBarcode(incoming));
    QPixmap shrunk = image.scaled(ui->app_pageview->size()/3, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->barcode_view->setPixmap(shrunk);
}


void MainWindow::on_priceChange_button_clicked()
{
    const QString input = ui->code_inputBox->text();

    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();
    if(selectItems.size() != 1){

    }else{
        int row = selectItems[0]->row();
        ui->ticketview->item(row, 3)->setText(input);
        ui->code_inputBox->setText("");
        ui->ticketview->resizeColumnsToContents();
    }
}


void MainWindow::on_qtyChange_button_clicked()
{
    const QString input = ui->code_inputBox->text();

    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();
    if(selectItems.size() != 1){

    }else{
        int row = selectItems[0]->row();
        ui->ticketview->item(row, 2)->setText(input);
        ui->code_inputBox->setText("");
        ui->ticketview->resizeColumnsToContents();
    }
}

void MainWindow::on_actionSales_Tax_clicked(){
    qDebug() << "adjust tax clicked";
    const QString input = ui->code_inputBox->text();

    QFile file(salesTaxLocation);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "Coudln't find tax";
    }

    QTextStream output(&file);

    output << input;
    file.close();
    qDebug() << "Tax updated";

    ui->code_inputBox->setText("");
    ui->ticketview->resizeColumnsToContents();
    emit ui->ticketview->cellChanged(0, 2);
}

void MainWindow::on_actionLocate_Database_clicked(){

    QString databaseNewLocation = QFileDialog::getOpenFileName();
    QFile file(databaseLocation);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "Coudln't find databaselocation.txt";
    }

    QTextStream output(&file);

    output << databaseNewLocation;

    file.close();
    qDebug() << "Database location updated.";
}

void MainWindow::on_actionTicketFolder_clicked(){
    QString ticketFolder = QFileDialog::getExistingDirectory();


    QFile file(defaultTicketFolderLocation);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << "Couldn't find defaultTicketsFolder.txt";
    }

    QTextStream output(&file);

    output << ticketFolder;

    file.close();
    qDebug() << "Ticket Folder updated.";
}
