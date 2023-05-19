#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtSql>
#include <QThread>



namespace Ui {
class Server;
}

class Server : public QWidget
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = nullptr);
    ~Server();

protected slots:

    void onNewConnection();

private slots:
    // 当客户端有消息到达时执行该函数
    void onReadyRead();

    // 当客户端断开连接时执行该函数
    void onDisconnected();

private:
    //客户端信息结构体
    struct ClientInfo
    {
        QTcpSocket *socket;         //客户端socket
        QString ip;                 //客户端IP地址
        int port;                   //客户端端口号
        QString account;            //客户端账号
        QString name;               //客户端称呼
        QString friendList;         //好友列表
    };

    Ui::Server *ui;
    QSqlDatabase db;                //数据库连接
    QTcpServer *server;
    QVector<ClientInfo> clients;  //在线客户端信息



    //对应处理客户端发送的登录请求
    void processLogin(QTcpSocket *socket, const QStringList &fields);

    //对应处理客户端发送的注册账户请求
    void processRegister(QTcpSocket *socket, const QStringList &fields);

    //对应处理客户端发送的添加好友请求(添加方)
    void processAddFriend_1(QTcpSocket *socket, const QStringList &fields);

    //对应处理客户端发送的添加好友请求(被添加方)
    void processAddFriend_2(const QStringList &fields);

    //对应处理客户端发送的聊天消息请求
    void processChat(QTcpSocket *socket, const QJsonDocument jsonDoc);

    //将在线客户端信息显示在服务器ui界面textBrowser_2上
    void showClients();
};

#endif // SERVER_H
