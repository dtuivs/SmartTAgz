#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QString>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>
#include <QtSvg/QtSvg>
#include "./ui_mainwindow.h"
#include "register_files.h"
#include "settings.h"
#include "ticket_menu.h"
#include <zint.h>

register_files mainCabinet;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->ticketview, &QTableWidget::cellChanged, this, &MainWindow::update_totalPrice);
    loadFile(mainCabinet.checkPersistenceFile("CURRENT_TICKET_LOCATION"));
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::on_actionSettings_clicked);
    connect(ui->action_HideKeypad, &QAction::triggered, this, &MainWindow::toggle_keypad);
    connect(ui->ticketview, &QTableWidget::cellChanged, this, &MainWindow::cellChanged);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::chooseTicketFile);

    QRegularExpression input_pattern(R"(^\d{8}|\d{12}$)");
    QRegularExpressionValidator *input_check = new QRegularExpressionValidator(input_pattern, this);
    ui->code_inputBox->setValidator(input_check);
    ui->code_inputBox_2->setValidator(input_check);

    ui->ticketview->setIconSize(QSize(300, 300));
    ui->ticketview->resizeColumnsToContents();
    QString keypadhiddenString = mainCabinet.checkPersistenceFile("KEYPADHIDDEN");
    if (keypadhiddenString == "1") {
        toggle_keypad(1);
        ui->action_HideKeypad->setChecked(true);
    } else {
        toggle_keypad(0);
        ui->action_HideKeypad->setChecked(false);
    }

    ui->exportBarcodes_button->setHidden(true);
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

void MainWindow::addtoTicket(QString lookup_code)
{
    if (code_input_check(lookup_code)) { //First check that it's a valid upc before doing anything
        //first check ticket view
        bool found = false;
        for (int row = 0; row < ui->ticketview->rowCount(); ++row) {
            QString table_upc = ui->ticketview->item(row, 0)->text();
            //qDebug() << lookup_code + " : " + table_upc;
            if (table_upc == lookup_code) {
                found = true;
                QString item_start_qty = ui->ticketview->item(row, 2)->text();
                QString updated_qty = plusOne(item_start_qty);

                QTableWidgetItem *updated_qty_label = new QTableWidgetItem(updated_qty);
                ui->ticketview->setItem(row, 2, updated_qty_label);
                ui->ticketview->scrollToItem(ui->ticketview->item(row, 2),
                                             QAbstractItemView::PositionAtBottom);
            }
        }
        //if not found then check database
        if (!found) {
            qDebug() << "Checking database.";
            QString databasefilepath = mainCabinet.checkPersistenceFile("DATABASE_FILE_LOCATION");
            QFile databaseFile(databasefilepath);
            if (!databaseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug() << "RETURNPRESSED: coulndt open database.";
            }

            QTextStream database(&databaseFile);
            int foundcount = 0;
            while (!database.atEnd()) {
                QString line = database.readLine();
                if (line.startsWith(lookup_code)) {
                    foundcount++;
                    int row = ui->ticketview->rowCount();
                    ui->ticketview->insertRow(row);

                    QStringList splitupclabel = line.split(" - ");
                    QString upc = splitupclabel[0].trimmed();
                    QTableWidgetItem *upctext = new QTableWidgetItem(upc);
                    ui->ticketview->setItem(row, 0, upctext);
                    ui->ticketview->item(row, 0)->setFlags(ui->ticketview->item(row, 0)->flags()
                                                           & ~Qt::ItemIsEditable);

                    //qDebug() << "upc added: "  + upc;
                    QStringList parts = splitupclabel[1].trimmed().split(" #)- ");

                    QString label = parts[0].trimmed();
                    QTableWidgetItem *labeltext = new QTableWidgetItem(label);
                    ui->ticketview->setItem(row, 1, labeltext);
                    //qDebug() << "label added: " + label;
                    QTableWidgetItem *view_qty = new QTableWidgetItem("1");
                    ui->ticketview->setItem(row, 2, view_qty);

                    if (parts.size() > 1) {
                        QString price = parts[1].trimmed();
                        QTableWidgetItem *pricetext = new QTableWidgetItem(processPrice(price));
                        ui->ticketview->setItem(row, 3, pricetext);
                    }
                    //generate barcode and add to fourth last column
                    QTableWidgetItem *barcodeSpace = new QTableWidgetItem();
                    QPixmap barcode = QPixmap::fromImage(generateBarcode(upc));
                    QPixmap sizedBarcode = barcode.scaled(ui->app_pageview->size() / 3,
                                                          Qt::KeepAspectRatio,
                                                          Qt::SmoothTransformation);
                    barcodeSpace->setIcon(QIcon(sizedBarcode));
                    ui->ticketview->setItem(row, 4, barcodeSpace);

                    ui->ticketview->scrollToBottom();
                }
            }

            QRegularExpressionMatch eightLong = isEightLong_digits.match(lookup_code);
            if (eightLong.hasMatch() && foundcount == 0) {
                QString expanded_code = expandtoUPCA(lookup_code);
                while (!database.atEnd()) {
                    QString line = database.readLine();
                    if (line.startsWith(expanded_code)) {
                        foundcount++;
                        int row = ui->ticketview->rowCount();
                        ui->ticketview->insertRow(row);

                        QStringList splitupclabel = line.split(" - ");
                        QString upc = splitupclabel[0].trimmed();
                        QTableWidgetItem *upctext = new QTableWidgetItem(upc);
                        ui->ticketview->setItem(row, 0, upctext);
                        ui->ticketview->item(row, 0)->setFlags(ui->ticketview->item(row, 0)->flags()
                                                               & ~Qt::ItemIsEditable);

                        //qDebug() << "upc added: "  + upc;
                        QStringList parts = splitupclabel[1].trimmed().split(" #)- ");

                        QString label = parts[0].trimmed();
                        QTableWidgetItem *labeltext = new QTableWidgetItem(label);
                        ui->ticketview->setItem(row, 1, labeltext);
                        //qDebug() << "label added: " + label;
                        QTableWidgetItem *view_qty = new QTableWidgetItem("1");
                        ui->ticketview->setItem(row, 2, view_qty);

                        if (parts.size() > 1) {
                            QString price = parts[1].trimmed();
                            QTableWidgetItem *pricetext = new QTableWidgetItem(processPrice(price));
                            ui->ticketview->setItem(row, 3, pricetext);
                        }
                        //generate barcode and add to fourth last column
                        QTableWidgetItem *barcodeSpace = new QTableWidgetItem();
                        QPixmap barcode = QPixmap::fromImage(generateBarcode(upc));
                        QPixmap sizedBarcode = barcode.scaled(ui->app_pageview->size() / 3,
                                                              Qt::KeepAspectRatio,
                                                              Qt::SmoothTransformation);
                        barcodeSpace->setIcon(QIcon(sizedBarcode));
                        ui->ticketview->setItem(row, 4, barcodeSpace);

                        ui->ticketview->scrollToBottom();
                    }
                }
            }

            //If not in database add empty row
            if (foundcount == 0) {
                int row = ui->ticketview->rowCount();
                ui->ticketview->insertRow(row);
                QTableWidgetItem *upc = new QTableWidgetItem(lookup_code);
                ui->ticketview->setItem(row, 0, upc);
                QTableWidgetItem *view_qty = new QTableWidgetItem("1");
                ui->ticketview->setItem(row, 2, view_qty);

                //generate barcode and add to fourth last column
                QTableWidgetItem *barcodeSpace = new QTableWidgetItem();
                QPixmap barcode = QPixmap::fromImage(generateBarcode(lookup_code));
                QPixmap sizedBarcode = barcode.scaled(ui->app_pageview->size() / 3,
                                                      Qt::KeepAspectRatio,
                                                      Qt::SmoothTransformation);
                barcodeSpace->setIcon(QIcon(sizedBarcode));
                ui->ticketview->setItem(row, 4, barcodeSpace);
                ui->ticketview->scrollToBottom();
            }
            databaseFile.close();
        }
    }
}

void MainWindow::toggle_keypad(bool isGone)
{
    QString StringIsGone;
    if (isGone) {
        ui->code_inputBox_2->setHidden(false);

        ui->code_inputBox->setHidden(true);
        ui->backspace_button->setHidden(true);
        ui->zero_button->setHidden(true);
        ui->one_button->setHidden(true);
        ui->two_Button->setHidden(true);
        ui->three_button->setHidden(true);
        ui->four_button->setHidden(true);
        ui->five_button->setHidden(true);
        ui->six_button->setHidden(true);
        ui->seven_button->setHidden(true);
        ui->eight_button->setHidden(true);
        ui->nine_button->setHidden(true);
        ui->enter_button->setHidden(true);
        ui->period_button->setHidden(true);
        ui->priceChange_button->setHidden(true);
        ui->qtyChange_button->setHidden(true);
        ui->line->setHidden(true);
        ui->line_2->setHidden(true);

        StringIsGone = "1";
    } else {
        ui->code_inputBox_2->setHidden(true);
        ui->code_inputBox->setHidden(false);
        ui->backspace_button->setHidden(false);
        ui->zero_button->setHidden(false);
        ui->one_button->setHidden(false);
        ui->two_Button->setHidden(false);
        ui->three_button->setHidden(false);
        ui->four_button->setHidden(false);
        ui->five_button->setHidden(false);
        ui->six_button->setHidden(false);
        ui->seven_button->setHidden(false);
        ui->eight_button->setHidden(false);
        ui->nine_button->setHidden(false);
        ui->enter_button->setHidden(false);
        ui->period_button->setHidden(false);
        ui->priceChange_button->setHidden(false);
        ui->qtyChange_button->setHidden(false);
        ui->line->setHidden(false);
        ui->line_2->setHidden(false);

        StringIsGone = "0";
    }

    mainCabinet.updatePersistenceFile("KEYPADHIDDEN", StringIsGone);
}

QImage MainWindow::generateBarcode(const QString &code)
{
    struct zint_symbol *symbol = ZBarcode_Create();
    if (!symbol) {
        qWarning("Failed making symbols!");
        return QImage();
    }
    if (code.size() == 8) {
        symbol->symbology = BARCODE_UPCE;
        symbol->height = 500;
        symbol->width = 1000;
        symbol->whitespace_width = 10;
        symbol->border_width = 2;
        symbol->show_hrt = 1;
    } else if (code.size() == 12) {
        symbol->symbology = BARCODE_UPCA;
        symbol->height = 500;
        symbol->width = 1525;
        symbol->whitespace_width = 10;
        symbol->border_width = 2;
        symbol->show_hrt = 1;
    }
    QByteArray codeData = code.toUtf8();
    int error = ZBarcode_Encode_and_Buffer(symbol, (unsigned char *) codeData.constData(), 0, 0);

    if (error != 0) {
        qWarning("failed to encode upc");
        ZBarcode_Delete(symbol);
        return QImage();
    }

    QImage barcodeImage(symbol->bitmap_width, symbol->bitmap_height, QImage::Format_ARGB32);
    barcodeImage.fill(Qt::white);

    for (int y = 0; y < symbol->bitmap_height; ++y) {
        for (int x = 0; x < symbol->bitmap_width; ++x) {
            //barcodeImage.setPixelColor(x, y, symbol->bitmap[y*symbol->bitmap_width+x] ? Qt::black : Qt::white);
            barcodeImage.setPixelColor(x,
                                       y,
                                       symbol->bitmap[y * symbol->bitmap_width + x / 4]
                                           ? Qt::black
                                           : Qt::white);
        }
    }

    ZBarcode_Delete(symbol);
    return barcodeImage;
}

void MainWindow::exportBarcodesList()
{
    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();

    if (!selectItems.isEmpty()) {
        QString saveTo = QFileDialog::getExistingDirectory(this, "Save where?");

        if (!saveTo.isEmpty()) {
            QList<int *> doneList;
            for (QTableWidgetItem *row_item : selectItems) {
                int row = row_item->row();
                int *rowValue = new int(row);
                bool alreadyDone = false;
                for (int *doneitem : doneList) {
                    alreadyDone = *doneitem == *rowValue ? true : false;
                }

                if (!alreadyDone) {
                    QString upc = ui->ticketview->item(row, 0)->text();
                    QString fileName = QString("%1/%2_barcode.png").arg(saveTo, upc);

                    struct zint_symbol *symbol = ZBarcode_Create();
                    if (!symbol) {
                        qWarning("Failed making symbols!");
                        //return QImage();
                    }
                    if (upc.size() == 8) {
                        symbol->height = 500;
                        symbol->width = 1000;
                        symbol->whitespace_width = 10;
                        symbol->border_width = 2;
                        symbol->show_hrt = 1;
                        symbol->symbology = BARCODE_UPCE;
                    } else if (upc.size() == 12) {
                        symbol->height = 500;
                        symbol->width = 1525;
                        symbol->whitespace_width = 10;
                        symbol->border_width = 2;
                        symbol->show_hrt = 1;
                        symbol->symbology = BARCODE_UPCA;
                    }
                    QByteArray codeData = upc.toUtf8();
                    int error = ZBarcode_Encode_and_Buffer(symbol,
                                                           (unsigned char *) codeData.constData(),
                                                           0,
                                                           0);

                    if (error != 0) {
                        qWarning("failed to encode upc");
                        ZBarcode_Delete(symbol);
                        //return QImage();
                    }

                    QImage barcodeImage(symbol->bitmap_width,
                                        symbol->bitmap_height,
                                        QImage::Format_ARGB32);
                    barcodeImage.fill(Qt::white);

                    for (int y = 0; y < symbol->bitmap_height; ++y) {
                        for (int x = 0; x < symbol->bitmap_width; ++x) {
                            //barcodeImage.setPixelColor(x, y, symbol->bitmap[y*symbol->bitmap_width+x] ? Qt::black : Qt::white);
                            barcodeImage
                                .setPixelColor(x,
                                               y,
                                               symbol->bitmap[y * symbol->bitmap_width + x / 4]
                                                   ? Qt::black
                                                   : Qt::white);
                        }
                    }
                    barcodeImage.save(saveTo);
                    ZBarcode_Delete(symbol);
                }
                doneList.append(rowValue);
            }
        }
    }
}

void MainWindow::cellChanged(int row, int column)
{
    qDebug() << "Cell Changed -- row: " + QString::number(row)
                    + ", Column: " + QString::number(column);
    if (column == 2) {
        //update_totalPrice();
    } else if (column == 1 || column == 3) {
        QString upc = ui->ticketview->item(row, 0) ? ui->ticketview->item(row, 0)->text() : "noupc";
        QString description = ui->ticketview->item(row, 1)->text();
        qDebug() << row;
        if (column == 3) {
            if (ui->ticketview->item(row, 1)->text() != nullptr
                && ui->ticketview->item(row, 1)->text() != nullptr) {
                QString price = ui->ticketview->item(row, 3) ? ui->ticketview->item(row, 3)->text()
                                                             : "";
                mainCabinet.updateDatabase(upc, description, price);
            }
        } else if (column == 1) {
            mainCabinet.updateDatabaseDescription(upc, description);
        }
        //qDebug() << ui->ticketview->item(row, 1)->text();
    }
    autosave();
    ui->ticketview->resizeColumnsToContents();
}

void MainWindow::autosave()
{
    QString currentTicket = mainCabinet.checkPersistenceFile("CURRENT_TICKET_LOCATION");
    saveFile(ui->ticketview, currentTicket);
}

void MainWindow::update_totalPrice()
{
    float total = 0.0;
    for (int row = 0; row < ui->ticketview->rowCount(); ++row) {
        //qDebug() << "checking row: " + QString::number(row);
        bool converted = false;
        QString table_qty;
        float qty = 1;
        if (ui->ticketview->item(row, 2) != nullptr) { //checking QTY if empty
            table_qty = ui->ticketview->item(row, 2)->text();
            //qDebug() << "Qty:"+table_qty;
            if (table_qty != nullptr) {
                qty = table_qty.toFloat();
            }
        } else {
            //qDebug() << "CHeck price: nullptr";
        }
        if (ui->ticketview->item(row, 3) != nullptr) { //checking price if empty
            QString table_price = ui->ticketview->item(row, 3)->text();
            //qDebug() << "Price:" + table_price;
            float plusthis = 0.0;
            if (table_price != nullptr) {
                plusthis = table_price.toFloat(&converted);
            }
            if (converted) {
                plusthis *= qty;
                total = total + plusthis;
            }
        }
    }
    //qDebug() << total;
    float tax = mainCabinet.checkPersistenceFile("SALES_TAX_PERCENT").toFloat();
    float tax_cut = tax / 100;
    tax_cut = tax_cut * total;
    total = total + tax_cut;
    if (ui->price_label) {
        QString taxcut_label = "+$" + QString::number(tax_cut, 'f', 2) + "("
                               + QString::number(tax, 'f', 2) + "%)";
        ui->tax_label->setText(taxcut_label);
        QString total_label = "Total: $" + QString::number(total, 'f', 2);
        ui->price_label->setText(total_label);

        //qDebug() << total_label;
    }
}

QString MainWindow::processPrice(const QString &price)
{
    if (isPrice_one.match(price).hasMatch()) {
        return price.mid(1);
    } else if (isPrice_two.match(price).hasMatch()) {
        return price.mid(6);
    } else {
        return QString();
    }
}

QString MainWindow::processQty(const QString &quantity)
{
    if (isQty.match(quantity).hasMatch()) {
        return quantity.mid(1);
    } else {
        return QString();
    }
}

void MainWindow::loadFile(QString filename)
{
    if (filename.isEmpty()) {
        return;
    }
    QFile file(filename);
    QFileInfo ticketData(file);
    QString cleanTitle = ticketData.fileName();
    setWindowTitle(cleanTitle);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Coudln't load file: " + filename;
        return;
    }

    ui->ticketview->setRowCount(0);

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString pattern = " #)- ";
        QString line = in.readLine();
        QStringList parts = line.split(pattern);
        int row = ui->ticketview->rowCount();
        ui->ticketview->insertRow(row);

        QString upc = parts[0].trimmed();
        if (upc == "" || upc == nullptr) {
        } else {
            QTableWidgetItem *item_upc = new QTableWidgetItem(upc);
            ui->ticketview->setItem(row, 0, item_upc);
            item_upc->setFlags(item_upc->flags() & ~Qt::ItemIsEditable);

            if (parts.size() >= 2) {
                QString label = parts[1].trimmed();
                //qDebug() << label;
                QTableWidgetItem *item_label = new QTableWidgetItem(label);
                ui->ticketview->setItem(row, 1, item_label);
                //item_label->setFlags(item_label->flags() & ~Qt::ItemIsEditable);
            }
            if (parts.size() >= 3) {
                QString quantity = parts[2].trimmed();
                QString qtyProcessed = processQty(quantity);
                //qDebug() << quantity;
                QTableWidgetItem *item_qty = new QTableWidgetItem(qtyProcessed);
                ui->ticketview->setItem(row, 2, item_qty);
            }
            if (parts.size() >= 4) {
                QString price = parts[3].trimmed();
                QString priceProcessed = processPrice(price);
                //qDebug() << priceProcessed;
                QTableWidgetItem *item_price = new QTableWidgetItem(priceProcessed);
                ui->ticketview->setItem(row, 3, item_price);
            }
            //generate barcode and add to fourth last column
            QTableWidgetItem *barcodeSpace = new QTableWidgetItem();
            QPixmap barcode = QPixmap::fromImage(generateBarcode(upc));
            QPixmap sizedBarcode = barcode.scaled(QSize(300, 300),
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation);
            barcodeSpace->setIcon(QIcon(sizedBarcode));
            ui->ticketview->setItem(row, 4, barcodeSpace);
            barcodeSpace->setFlags(item_upc->flags() & ~Qt::ItemIsEditable);
        }
    }
}

void MainWindow::saveFile(QTableWidget *given_table, const QString &filePath)
{
    QFile ticket_filePath(filePath);

    if (!ticket_filePath.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "could not open file";
    }

    QTextStream output(&ticket_filePath);

    for (int row = 0; row < given_table->rowCount(); ++row) {
        QString upc = given_table->item(row, 0) ? given_table->item(row, 0)->text() : "no_upc";
        QString label = given_table->item(row, 1) ? given_table->item(row, 1)->text() : "label";
        QString qty = given_table->item(row, 2) ? "x" + given_table->item(row, 2)->text() : "x1";
        QString price = "";
        QString formatted_line = "";
        if (given_table->item(row, 3)) {
            price = given_table->item(row, 3)->text();
            formatted_line = QString("%1 #)- %2 #)- %3 #)- $%4").arg(upc, label, qty, price);
        } else {
            formatted_line = QString("%1 #)- %2 #)- %3").arg(upc, label, qty);
        }
        //qDebug() << formatted_line;
        output << formatted_line << "\n";
    }
    ticket_filePath.close();
    qDebug() << "Save Successful.";
}

void MainWindow::searchDatabase(const QString &SearchText, const QString &databasePath)
{
    QFile databasefile(databasePath);

    ui->foundView->setRowCount(0);

    if (!databasefile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "couldnt open database";
    }

    QTextStream database(&databasefile);
    while (!database.atEnd()) {
        QString line = database.readLine();
        if (line.contains(SearchText, Qt::CaseInsensitive)) {
            //function should be expecting a line of text formatted: UPC: descrption #)- price:0.00
            update_foundView(line);
        }
    }
    databasefile.close();

    ui->foundView->resizeColumnsToContents();
}

QString MainWindow::expandtoUPCA(const QString &upc)
{
    QString base = upc.length() == 8 ? upc.mid(1, 6) : upc;
    QString checksum = upc.length() == 8 ? upc[7] : '0';

    if (base.length() != 6) {
        return "-1";
    }

    QChar lastDigit = base[5];
    QString expanded = "";

    if (lastDigit == '0' || lastDigit == '1' || lastDigit == '2') {
        expanded = base.left(2) + lastDigit + "0000" + base.mid(2, 3);
    } else if (lastDigit == '3') {
        expanded = base.left(3) + "00000" + base.mid(3, 2);
    } else if (lastDigit == '4') {
        expanded = base.left(4) + "00000" + base[4];
    } else {
        expanded = base.left(5) + "0000" + lastDigit;
    }
    return "0" + expanded + checksum;
}

bool MainWindow::code_input_check(const QString &input)
{
    QString code = input;
    QRegularExpressionMatch twelveLong = istwelveLong_digits.match(code);
    QRegularExpressionMatch eightLong = isEightLong_digits.match(code);

    //qDebug() << code + " : " + QString::number(code.size());
    if (!eightLong.hasMatch()) {
        if (!twelveLong.hasMatch()) {
            return false;
        }
    } else {
        //qDebug() << expandtoUPCA(code);
        code = expandtoUPCA(code);
    }

    int sum = 0;
    for (int i = 0; i < 11; ++i) {
        int digit = code[i].digitValue();
        sum += (i % 2 == 0) ? digit * 3 : digit;
    }
    int last_digit = code[11].digitValue();
    int checksum = (10 - (sum % 10)) % 10;
    //qDebug() << QString::number(checksum) + " ? " + QString::number(last_digit);
    if (checksum == last_digit) {
        return true;
    } else {
        return false;
    }
}

void MainWindow::on_code_inputBox_textChanged(const QString &arg1)
{
    ui->enter_button->setDisabled(!code_input_check(arg1));
}

void MainWindow::update_foundView(QString rawitem)
{ //function should be expecting a line of text formatted: UPC: descrption #)- price:0.00
    int row = ui->foundView->rowCount();
    ui->foundView->insertRow(row);

    QStringList splitupcLabel = rawitem.split(" - ");
    QString upc = splitupcLabel[0].trimmed();
    QTableWidgetItem *upctext = new QTableWidgetItem(upc);
    ui->foundView->setItem(row, 0, upctext);

    QStringList parts = splitupcLabel[1].trimmed().split(" #)- ");

    QString label = parts[0].trimmed();
    QTableWidgetItem *labeltext = new QTableWidgetItem(label);
    ui->foundView->setItem(row, 1, labeltext);

    if (parts.size() > 1) {
        QString price = parts[1].trimmed();
        QTableWidgetItem *pricetext = new QTableWidgetItem(processPrice(price));
        ui->foundView->setItem(row, 2, pricetext);
    }
}

QString MainWindow::plusOne(QString qty)
{
    bool converted;
    int plus = qty.toInt(&converted);
    if (converted) {
        plus += 1;
        QString qty_label = QString::number(plus);
        return qty_label;
    } else {
        return "";
    }
}

void MainWindow::on_code_inputBox_returnPressed()
{
    QString given = ui->code_inputBox->text();

    if (given.size() != 0) {
        addtoTicket(given);

        ui->code_inputBox->setText("");
        ui->ticketview->resizeColumnsToContents();
    }
}

void MainWindow::digit_clicked(QString digit)
{
    QString given = ui->code_inputBox->text();
    given += digit;
    ui->code_inputBox->setText(given);
}

void MainWindow::on_one_button_clicked()
{
    digit_clicked("1");
}

void MainWindow::on_two_Button_clicked()
{
    digit_clicked("2");
}

void MainWindow::on_three_button_clicked()
{
    digit_clicked("3");
}

void MainWindow::on_four_button_clicked()
{
    digit_clicked("4");
}

void MainWindow::on_five_button_clicked()
{
    digit_clicked("5");
}

void MainWindow::on_six_button_clicked()
{
    digit_clicked("6");
}

void MainWindow::on_seven_button_clicked()
{
    digit_clicked("7");
}

void MainWindow::on_eight_button_clicked()
{
    digit_clicked("8");
}

void MainWindow::on_nine_button_clicked()
{
    digit_clicked("9");
}

void MainWindow::on_zero_button_clicked()
{
    digit_clicked("0");
}

void MainWindow::on_period_button_clicked()
{
    digit_clicked(".");
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

void MainWindow::on_newticket_button_clicked()
{
    QDir folderPath(mainCabinet.return_settingsFolder());
    if (!folderPath.exists()) {
        folderPath.mkpath(".");
    }
    QString newfilename = mainCabinet.generateTimestamp() + "_List.txt";
    mainCabinet.updatePersistenceFile("CURRENT_TICKET_LOCATION",
                                      mainCabinet.checkPersistenceFile("TICKET_FOLDER_LOCATION")
                                          + "/" + newfilename);
    ui->ticketview->setRowCount(0);
    setWindowTitle(newfilename);
}

void MainWindow::on_load_button_clicked()
{
    launchTicketsMenu();
}

void MainWindow::chooseTicketFile()
{
    QString filename = QFileDialog::getOpenFileName(nullptr,
                                                    "Load Ticket",
                                                    mainCabinet.checkPersistenceFile(
                                                        "TICKET_FOLDER_LOCATION"),
                                                    nullptr);
    if(filename != nullptr){
        mainCabinet.updatePersistenceFile("CURRENT_TICKET_LOCATION", filename);
        loadFile(filename);
    }
}

void MainWindow::on_save_button_clicked()
{
    QString ticketDestinaton = mainCabinet.checkPersistenceFile("CURRENT_TICKET_LOCATION");
    QString saveTo = QFileDialog::getSaveFileName(nullptr, "Save As", ticketDestinaton, nullptr);
    if (saveTo != "") {
        saveFile(ui->ticketview, saveTo);
        mainCabinet.updatePersistenceFile("CURRENT_TICKET_LOCATION", saveTo);
        loadFile(saveTo);
    }
}

void MainWindow::on_searchbar_input_textChanged(const QString &arg1)
{
    QString databasefilePath = mainCabinet.checkPersistenceFile("DATABASE_FILE_LOCATION");
    searchDatabase(arg1, databasefilePath);
}

void MainWindow::on_delete_button_clicked()
{
    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();

    if (selectItems.isEmpty()) {
        qDebug() << "no selected items";
    } else {
        for (QTableWidgetItem *item : selectItems) {
            if (item) {
                int removalRow = item->row();
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
    QFile savePath = QFileDialog::getSaveFileName(nullptr,
                                                  "Save Receipt",
                                                  homePath + "/Downloads/" + mainCabinet.generateTimestamp()
                                                      + "_receipt",
                                                  nullptr)
                     + ".txt";
    if (!savePath.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "couldn't do it.";
    }
    QTextStream output(&savePath);

    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();
    if (selectItems.isEmpty()) {
        qDebug() << "nothing selected: full receipt";
        for (int row = 0; row < ui->ticketview->rowCount(); ++row) {
            QString upc = ui->ticketview->item(row, 0) ? ui->ticketview->item(row, 0)->text() : "";
            QString description = ui->ticketview->item(row, 1)
                                      ? ui->ticketview->item(row, 1)->text()
                                      : "item";
            QString qty = ui->ticketview->item(row, 2) ? ui->ticketview->item(row, 2)->text() : "1";
            QString price = ui->ticketview->item(row, 3) ? ui->ticketview->item(row, 3)->text()
                                                         : "0.00";

            QString formatted_line = QString("%1 %2 @$%3(x%4)").arg(upc, description, price, qty);
            //qDebug() << formatted_line ;
            output << formatted_line << "\n";
        }
    } else {
        qDebug() << "limited receipt";
        for (QTableWidgetItem *row_item : selectItems) {
            if (row_item) {
                int row = row_item->row();
                //qDebug()  << row_item->text();
                QString upc = ui->ticketview->item(row, 0) ? ui->ticketview->item(row, 0)->text()
                                                           : "no_upc";
                QString description = ui->ticketview->item(row, 1)
                                          ? ui->ticketview->item(row, 1)->text()
                                          : "label";
                QString qty = ui->ticketview->item(row, 2) ? ui->ticketview->item(row, 2)->text()
                                                           : "1";
                QString price = ui->ticketview->item(row, 3) ? ui->ticketview->item(row, 3)->text()
                                                             : "0";

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

    if (selectItems.isEmpty()) {
        qDebug() << "addtoticket: Nothing selected";
    } else {
        QList<int *> doneList;
        for (QTableWidgetItem *row_item : selectItems) {
            //qDebug() << row_item->row();
            int row = row_item->row();
            int *rowValue = new int(row);
            bool alreadyDone = false;
            for (int *doneitem : doneList) {
                //qDebug() << QString::number(*doneitem) + " : " + QString::number(*rowValue);
                if (*doneitem == *rowValue) {
                    alreadyDone = true;

                } else {
                    alreadyDone = false;
                }
                //qDebug() << alreadyDone;
            }
            qDebug() << alreadyDone;
            if (!alreadyDone) {
                qDebug() << "adding to ticket: " + QString::number(row);
                QString upc = ui->foundView->item(row, 0)->text();
                addtoTicket(upc);
            }
            doneList.append(rowValue);
        }
        qDeleteAll(doneList);
        doneList.clear();
    }
}

void MainWindow::on_barcode_button_clicked()
{
    QString incoming;
    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();
    if (selectItems.isEmpty()) {
        incoming = ui->ticketview->item(0, 0)->text();
    } else {
        int row = selectItems[0]->row();
        incoming = ui->ticketview->item(row, 0)->text();
    }
    QPixmap image = QPixmap::fromImage(generateBarcode(incoming));
    QPixmap shrunk = image.scaled(ui->app_pageview->size() / 4,
                                  Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
    ui->barcode_view->setPixmap(shrunk);
}

void MainWindow::on_priceChange_button_clicked()
{
    const QString input = ui->code_inputBox->text();
    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();
    if (selectItems.size() == 1) {
        int row = selectItems[0]->row();
        QTableWidgetItem *newPrice = new QTableWidgetItem(input);
        ui->ticketview->setItem(row, 3, newPrice);
        ui->code_inputBox->setText("");
        ui->ticketview->resizeColumnsToContents();
    }
}

void MainWindow::on_qtyChange_button_clicked()
{
    const QString input = ui->code_inputBox->text();
    QList<QTableWidgetItem *> selectItems = ui->ticketview->selectedItems();
    if (selectItems.size() == 1) {
        int row = selectItems[0]->row();
        ui->ticketview->item(row, 2)->setText(input);
        ui->code_inputBox->setText("");
        ui->ticketview->resizeColumnsToContents();
    }
}

void MainWindow::on_actionSettings_clicked()
{
    if (!settingsWindow) {
        settingsWindow = new settings(this);
        settingsWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(settingsWindow, &QWidget::destroyed, this, [this]() { settingsWindow = nullptr; });
        settingsWindow->show();
    } else {
        settingsWindow->raise();
        settingsWindow->activateWindow();
    }
}

void MainWindow::launchTicketsMenu(){
    if(!ticketMenu){
        ticketMenu = new ticket_menu(this);
        ticketMenu->setAttribute(Qt::WA_DeleteOnClose);
        connect(ticketMenu, &QWidget::destroyed, this, [this](){ ticketMenu = nullptr; });
        connect(ticketMenu, &ticket_menu::selectedTicket, this, &MainWindow::loadFile);
        ticketMenu->show();
    }else{
        ticketMenu->raise();
        ticketMenu->activateWindow();
    }
}

void MainWindow::on_code_inputBox_2_textChanged(const QString &arg1) {}

void MainWindow::on_code_inputBox_2_returnPressed()
{
    QString given = ui->code_inputBox_2->text();

    if (given.size() != 0) {
        addtoTicket(given);
        ui->code_inputBox_2->setText("");
        ui->ticketview->resizeColumnsToContents();
    }
}

void MainWindow::on_exportBarcodes_button_clicked()
{
    exportBarcodesList();
}
