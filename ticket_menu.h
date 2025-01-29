#ifndef TICKET_MENU_H
#define TICKET_MENU_H

#include <QDialog>

namespace Ui {
class ticket_menu;
}

class ticket_menu : public QDialog
{
    Q_OBJECT

public:
    explicit ticket_menu(QWidget *parent = nullptr);
    ~ticket_menu();

signals:
    void selectedTicket(const QString &filename);

private:
    Ui::ticket_menu *ui;

    void fetchTickets(QString folder);

    void setTicket();
};

#endif // TICKET_MENU_H
