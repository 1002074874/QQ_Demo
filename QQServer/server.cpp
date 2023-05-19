#include "server.h"
#include "ui_server.h"

#include <QtGui/QPalette>
#include <QDateTime>


Server::Server(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);
    //链接数据库
    db = QSqlDatabase::addDatabase("QMYSQL");         //加载Mysql数据库驱动
    db.setDatabaseName("mydatabase");
    db.setHostName("localhost");
    db.setUserName("root");
    db.setPassword("1002074874");
    db.open();

    //监听端口"8888"
    server = new QTcpServer(this);
    server->listen(QHostAddress::Any, 8888);

    // 当有新连接的时候触发 newConnection 函数
    connect(server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

}

Server::~Server()
{
    delete ui;
}

//处理客户端的请求
void Server::onNewConnection()
{
    QTcpSocket *clientSocket = server->nextPendingConnection();

    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

}


//读取客户端的消息
void Server::onReadyRead()
{
    //sender() 函数可以获取到信号的发送者，也就是发出该信号的 QTcpSocket 对象的指针。使用 static_cast 将该指针转换为 QTcpSocket 类型的指针.
    //QString 的 split() 函数将字符串按照分隔符“|”进行分割，并将分割后得到的字符串存储到 QStringList 中。
    QTcpSocket *socket = static_cast<QTcpSocket *>(sender());
    int flag;

    QString message = QString(socket->readAll());
    QStringList fields = message.split("|");
    flag = fields[0].toInt();

    //判断解析出来的消息是不是 JSON 格式
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error == QJsonParseError::NoError && jsonDoc.isObject())
    {
        flag = 2;
    }

    switch (flag) {
    case 0: //处理登录请求
        processLogin(socket, fields);
        break;
    case 1: //处理注册账户请求
        processRegister(socket, fields);
        break;
    case 2: //处理聊天消息请求
        processChat(socket, jsonDoc);
        break;
    case 3: //处理添加好友请求（添加方）
        processAddFriend_1(socket, fields);
        break;
    case 4: //处理添加好友请求（被添加方）
        processAddFriend_2(fields);
        break;

    default:
        qDebug() << "未知请求";
        break;
    }
}


//客户端断开连接
void Server::onDisconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket *>(sender());

    QString account;

    //在在线客户端列表中查找该客户端，并将其从列表中删除
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].socket == socket)
        {
            account = clients[i].account;

            //服务器进行记录
            QDateTime dateTime= QDateTime::currentDateTime();    //获取时间
            ui->textBrowser_1->insertHtml("<span style='color:rgb(38,121,57);'>" + dateTime.toString("yyyy-MM-dd hh:mm:ss") + "</span>\n");
            ui->textBrowser_1->append(QString("账号:%1 退出登录\n\n").arg(account));

            clients.remove(i);
            break;
        }
     }


     // 将在线客户端信息显示在服务器ui界面textBrowser_2上
     showClients();
}


//处理客户端发送的登录请求
void Server::processLogin(QTcpSocket *socket, const QStringList &fields)
{
    //服务器进行记录
    QDateTime dateTime= QDateTime::currentDateTime();    //获取时间
    ui->textBrowser_1->insertHtml("<span style='color:rgb(38,121,57);'>" + dateTime.toString("yyyy-MM-dd hh:mm:ss") + "</span>\n");
    ui->textBrowser_1->append(QString("IP:%1 正在进行登录操作").arg(socket->peerAddress().toString()));

    QString account = fields.at(1);
    QString password = fields.at(2);

    //查询数据库，验证账号密码是否正确
    QSqlQuery query;
    query.exec(QString("SELECT * FROM user WHERE account='%1' AND password='%2'").arg(account).arg(password));
    if (query.next())
    {
        //检查该用户是否已登陆
        for (const auto& client : clients)
        {
            if (client.account == account)
            {
                //已在其他设备上登陆
                QString message = "1";
                socket->write(message.toUtf8());
                socket->flush();

                //服务器进行记录
                ui->textBrowser_1->append(QString("操作结果:失败  原因:该账号已在其他设备上登陆\n\n"));

                return;
            }
        }

        //该用户未登陆，继续执行登陆操作
        ClientInfo info;
        info.socket = socket;
        info.ip = socket->peerAddress().toString();
        info.port = socket->peerPort();
        info.account = query.value("account").toString();
        info.name = query.value("name").toString();
        info.friendList = query.value("friendlist").toString();

        clients.append(info);

        //服务器进行记录
        ui->textBrowser_1->append(QString("操作结果:%1账号已登录\n\n").arg(info.account));

        //发送登录成功消息给客户端
        QStringList fields;
        fields << info.name << info.friendList;
        QString message = "0|" + fields.join("|");
        socket->write(message.toUtf8());

        //将在线客户端信息显示在服务器ui界面textBrowser_2上
        showClients();
    }
    else
    {
        //登录失败，发送错误消息给客户端
        QString message = "2";

        //服务器进行记录
        ui->textBrowser_1->append(QString("操作结果:失败  原因:账号或密码错误\n\n"));

        socket->write(message.toUtf8());
        socket->flush();
    }
}


//对应处理客户端发送的注册账户请求
void Server::processRegister(QTcpSocket *socket, const QStringList &fields)
{
    //服务器进行记录
    QDateTime dateTime= QDateTime::currentDateTime();    //获取时间
    ui->textBrowser_1->insertHtml("<span style='color:rgb(38,121,57);'>" + dateTime.toString("yyyy-MM-dd hh:mm:ss") + "</span>\n");
    ui->textBrowser_1->append(QString("IP:%1 正在进行注册操作").arg(socket->peerAddress().toString()));

    QString account = fields.at(1); // 获取账号
    QString password = fields.at(2); // 获取密码
    QString name = fields.at(3); // 获取称呼

    // 检查账号是否已被注册
    QSqlQuery query;
    query.exec(QString("SELECT * FROM user WHERE account='%1'").arg(account));
    if (query.next())
    {
        //账号已存在，发送错误消息给客户端
        QString message = "1账号已被注册";
        socket->write(message.toUtf8());
        socket->flush();

        //服务器进行记录
        ui->textBrowser_1->append(QString("操作结果:失败  原因:账号已被注册\n\n"));

        return;
    }

    // 将账号、密码和称呼插入数据库中
    query.exec(QString("INSERT INTO user (account, password, name) VALUES ('%1', '%2', '%3')").arg(account).arg(password).arg(name));
    if (query.isActive())
    {
        //注册成功，发送成功消息给客户端
        QString message = "0";
        socket->write(message.toUtf8());
        socket->flush();

        //服务器进行记录
        ui->textBrowser_1->append(QString("操作结果:成功\n\n"));
    }
    else
    {
        //注册失败，发送错误消息给客户端并记录该操作
        QString message = "1注册失败";
        socket->write(message.toUtf8());
        socket->flush();

        //服务器进行记录
        ui->textBrowser_1->append(QString("操作结果:失败\n\n"));
    }
}


//对应处理客户端发送的添加好友请求(添加方)
void Server::processAddFriend_1(QTcpSocket *socket, const QStringList &fields)
{
    QString account_1;                   //添加方账号
    QString name_1;                      //添加方账号称呼
    QString account_2 = fields.at(1);    //被添加方账号
    QString name_2;                      //被添加方账号

    //找出添加方账号与称呼
    for (int i = 0; i < clients.size(); i++)
    {
        ClientInfo info = clients.at(i);
        if (info.socket == socket)
        {
            account_1 = info.account;
            name_1 = info.name;
        }
    }

    //记录该操作的时间以及操作内容
    QDateTime dateTime= QDateTime::currentDateTime();
    ui->textBrowser_1->insertHtml("<span style='color:rgb(38,121,57);'>" + dateTime.toString("yyyy-MM-dd hh:mm:ss") + "</span>\n");
    ui->textBrowser_1->append(QString("账号:%1  正在进行添加好友操作").arg(account_1));

    // 当前用户想要将自己添加为好友
    if (account_1 == account_2)
    {
        QString message = "4";
        socket->write(message.toUtf8());
        socket->flush();

        ui->textBrowser_1->append(QString("%1 添加好友 %2 失败  原因:不能添加自己为好友 \n\n").arg(account_1).arg(account_2));
        return;
    }


    //首先判断该账号是否注册，如果不存在，则发送错误消息给客户端
    QSqlQuery query_1(QString("SELECT * FROM user WHERE account='%1'").arg(account_2));
    if (!query_1.next())
    {
        QString Message = "1";
        socket->write(Message.toUtf8());
        socket->flush();

       ui->textBrowser_1->append(QString("操作结果:失败  原因:添加的该账号不存在\n\n"));

        return;
    }

    //判断该好友是否已经存在好友列表中
    QSqlQuery query_2(QString("SELECT * FROM user WHERE account = '%1'").arg(account_1));  //在数据库中查询添加方账号相关信息
    query_2.next();
    QStringList friendList = query_2.value("friendlist").toString().split("&&");  //添加方账号的friendlist（好友列表）
    foreach(QString tempfriend, friendList)
    {
        QStringList temp = tempfriend.split("+");   //temp = account(账号)与name（称呼）    添加方
        if(temp.at(0) == account_2)              //temp.at(0) == info.account 添加方好友列表中有account_2（被添加方）
        {
            //已是好友，发送错误消息给客户端并退出
            QString message = "2";
            socket->write(message.toUtf8());
            socket->flush();

            //记录该操作
            ui->textBrowser_1->append(QString("操作结果:失败  原因:被添加方已经是添加方的好友\n\n"));

            return;
        }
    }


    //被添加的账号存在，在“在线客户端列表”中查找该账号对应的客户端信息，判断需要添加的账号是否在线
    bool found = false;
    for (int i = 0; i < clients.size(); i++)
    {
        ClientInfo info = clients.at(i);
        if (account_2 == info.account)
        {
            name_2 = info.name;

            //被添加方账号存在且在线，将添加方发送的好友请求发送给被添加方客户端
            QString message = "9|" + account_1 + "|" + name_1 + "|" + account_2+ "|" + name_2;
            info.socket->write(message.toUtf8());
            info.socket->flush();

            //给添加方发送消息。
            QString message_2 = "0";
            socket->write(message_2.toUtf8());
            socket->flush();

            ui->textBrowser_1->append(QString("账号：%1 已向 账号：%2 发送添加好友请求\n\n").arg(account_1).arg(account_2));

            found = true;
            break;
        }
    }

    //未在“在线客户端列表”找到该账号对应的客户端信息
    if (found == false)
    {

        QString message = "3";
        socket->write(message.toUtf8());
        socket->flush();

        ui->textBrowser_1->append(QString("%1 添加好友 %2 失败  原因:添加的账号不在线 \n\n").arg(account_1).arg(account_2));
        return;
    }

}


//对应处理客户端发送的添加好友请求(被添加方)
void Server::processAddFriend_2(const QStringList &fields)
{
    int flag = fields.at(1).toInt();

    QString account_1 = fields.at(2);                   //添加方账号
    QString name_1 = fields.at(3);                      //添加方账号称呼
    QString account_2 = fields.at(4);                   //被添加方账号
    QString name_2 = fields.at(5);                      //被添加方账号

    //记录该操作的时间以及操作内容
    QDateTime dateTime= QDateTime::currentDateTime();
    ui->textBrowser_1->insertHtml("<span style='color:rgb(38,121,57);'>" + dateTime.toString("yyyy-MM-dd hh:mm:ss") + "</span>\n");


    if(flag == 0)
    {

        //处理被添加发来的信号，0代表同意添加，1代表拒绝添加
        //收到信号0，代表被添加方同意了添加方的好友请求，更新数据库中添加方与被添加方的好友列表
        QSqlQuery friendquery_1(QString("SELECT * FROM user WHERE account='%1'").arg(account_1));
        QSqlQuery friendquery_2(QString("SELECT * FROM user WHERE account='%1'").arg(account_2));

        friendquery_1.exec();
        friendquery_2.exec();

        friendquery_1.next();
        friendquery_2.next();


        QStringList friendList_1 = friendquery_1.value("friendlist").toString().split("&&");            //添加方好友列表
        QStringList friendList_2 = friendquery_2.value("friendlist").toString().split("&&");            //被添加方好友列表

        friendList_1.append(QString("%1+%2").arg(account_2).arg(name_2));
        friendList_2.append(QString("%1+%2").arg(account_1).arg(name_1));

        QString friendString_1 = friendList_1.join("&&");
        QString friendString_2 = friendList_2.join("&&");

        QSqlQuery updateQuery_1(QString("UPDATE user SET friendlist ='%1' WHERE account='%2'").arg(friendString_1).arg(account_1));
        QSqlQuery updateQuery_2(QString("UPDATE user SET friendlist ='%1' WHERE account='%2'").arg(friendString_2).arg(account_2));


        //记录该操作
        ui->textBrowser_1->append(QString("操作结果:账号:%2 通过了 账号:%1 的好友请求\n\n").arg(account_1).arg(account_2));

        return;
    }

    if(flag == 1)
    {
        //收到信号1，代表被添加方拒绝了添加方的好友请求
        //记录该操作
        ui->textBrowser_1->append(QString("操作结果:账号:%2 拒绝了 账号:%1 的好友请求\n\n").arg(account_1).arg(account_2));
        return;
    }

}


//对应处理客户端发送的聊天消息请求
void Server::processChat(QTcpSocket *socket, const QJsonDocument jsonDoc)
{

    QJsonObject jsonObject = jsonDoc.object();
    QString toAccount = jsonObject.value("toAccount").toString();     //接收方账号
    QString toName = jsonObject.value("toName").toString();           //接受方称呼

    QByteArray message = jsonDoc.toJson(QJsonDocument::Compact);

    //查找目标账户是否在线
    bool isOnline = false; // 默认设为不在线

    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].account == toAccount)
        {
            isOnline = true;
            //如果目标账户在线，将消息转发给目标账户
            clients[i].socket->write(message);
            clients[i].socket->flush();
        }
    }

    if (!isOnline)
    {
        //如果目标账户不在线，则告诉消息发出者
        QString message = "7|" + toAccount + "|" + toName;
        socket->write(message.toUtf8());
        socket->flush();
    }

}


//将在线客户端信息显示在服务器ui界面textBrowser_2上
void Server::showClients()
{
    ui->textBrowser_2->clear();
    for (int i = 0; i < clients.size(); i++)
    {
        ClientInfo info = clients.at(i);
        ui->textBrowser_2->append(QString("IP:%1 端口号:%2 账号:%3").arg(info.ip).arg(info.port).arg(info.account));
    }
}
