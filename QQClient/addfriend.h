#ifndef ADDFRIEND_H
#define ADDFRIEND_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>

namespace Ui {
class addFriend;
}

class addFriend : public QWidget
{
    Q_OBJECT

public:
    explicit addFriend(QTcpSocket *s, QWidget *parent = nullptr);
    ~addFriend();

private slots:

    void on_pushButton_clicked();

signals:

    void re_ConnectSignal();

private:
    Ui::addFriend *ui;
    QTcpSocket *socket;
};

#endif // ADDFRIEND_H
