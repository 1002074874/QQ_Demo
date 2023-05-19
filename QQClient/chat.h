#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonObject>
#include <QToolButton>


namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(QTcpSocket *s, QString account, QString name, QString m_account, QString m_name,QWidget *parent = nullptr);
    ~Chat();

    void receiveMessage(QString text, QString time);


private slots:
    void on_close_Button_clicked();

    void on_sendmsg_Button_clicked();

private:
    Ui::Chat *ui;
    QTcpSocket *socket;
    QString friendAccount; // 用于保存当前聊天窗口对应的好友的账号
    QString friendName;   // 用于保存当前聊天窗口对应的好友的称呼
    QString my_name;
    QString my_account;

};

#endif // CHAT_H
