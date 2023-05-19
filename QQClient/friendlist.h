#ifndef FRIENDLIST_H
#define FRIENDLIST_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QToolButton>


#include <addfriend.h>
#include <chat.h>

namespace Ui {
class FriendList;
}

class FriendList : public QWidget
{
    Q_OBJECT

public:
    explicit FriendList(QTcpSocket *s, QString account, QString name, QString friendlist, QWidget *parent = nullptr);
    ~FriendList();

    //void处理收到的添加好友请求
    void receiveAddFriend(QTcpSocket *socket, const QStringList &fields);

    //收到聊天请求
    void receiveChatRequest(QTcpSocket *socket, const QJsonDocument jsonDoc);

    //此用户不在线
    void isNotOnline( const QStringList &fields);

    //重新挂起connect
    void reConnect();

private slots:

    //添加好友
    void on_addFriendButton_clicked();

    //重新挂起connect
    void re_Connect();


private:
    Ui::FriendList *ui;
    QTcpSocket *socket;
    QString m_name;
    QString m_account;
    QMap<QString, Chat*> chatWindowMap; //保存已经打开过的聊天窗口对象
};

#endif // FRIENDLIST_H
