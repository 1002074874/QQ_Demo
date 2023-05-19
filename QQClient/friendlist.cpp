#include "friendlist.h"
#include "ui_friendlist.h"


FriendList::FriendList(QTcpSocket *s, QString account, QString name, QString friendlist, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendList)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/images/QQ.png"));  //设置图标
    this->setWindowTitle(name);      //设置窗口名称

    socket = s;

    m_name = name;
    m_account = account;

    QVector<QToolButton *> friendButtonList;

    QStringList friends = friendlist.split("&&"); //使用“&&”分隔好友列表
    for (int i = 0; i < friends.size(); i++)

    {
        //根据好友数量生成对应数量的好友按钮并设置名称为好友的称呼
        QStringList friendInfo = friends.at(i).split("+");   //使用“+”分隔账号和姓名
        if (friendInfo.size() == 2)
        {
            //确保friendInfo中包含账号和姓名
            QToolButton *button = new QToolButton(this);
            button->setFixedSize(360, 70);
            button->setText(friendInfo.at(1)); // 设置好友按钮名称
            button->setProperty("account",friendInfo.at(0)); //保存好友账户名信息到按钮的属性
            ui->vLayout->addWidget(button);

            // 加入列表中保存
            friendButtonList.append(button);
        }
        else
        {
            qDebug() << "错误" << friends.at(i);
        }
    }


    // 遍历好友按钮列表并连接对应的槽函数：
    for (int i = 0; i < friendButtonList.size(); i++) // 改为使用列表进行遍历
    {
        connect(friendButtonList[i], &QAbstractButton::clicked,[=]()
        {
            QString account = friendButtonList.at(i)->property("account").toString();
            QString name = friendButtonList.at(i)->text();

            // 检查是否已经打开过聊天窗口
            if (chatWindowMap.contains(account))
            {
                chatWindowMap[account]->show();  // 如果已经打开，直接显示
            }
            else
            {
                Chat *c = new Chat(socket, account, name, m_account, m_name);
                c->show();
                chatWindowMap.insert(account, c);  // 如果未打开，则创建并保存到 QMap 中
            }
        });
    }

    //连接服务器端的信号和槽函数
    connect(socket, &QTcpSocket::readyRead, this, [=]()
    {
        QString message = QString(socket->readAll());
        QStringList fields = message.split("|");
        int flag = fields[0].toInt();

        //判断解析出来的消息是不是 JSON 格式
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError && jsonDoc.isObject())
        {
            flag = 8;
        }

        switch (flag)
        {
        case 7:     //进行该用户不在线
            isNotOnline(fields);
            break;
        case 8:     //处理收到的聊天请求
            receiveChatRequest(socket, jsonDoc);
            break;
        case 9:     //处理收到的添加好友请求
            receiveAddFriend(socket, fields);
            break;
        }
    });

}

FriendList::~FriendList()
{
    delete ui;
}


//点击添加好友按钮事件
void FriendList::on_addFriendButton_clicked()
{
    //移除之前的连接
    disconnect(socket, &QIODevice::readyRead, this, nullptr);

    addFriend *f = new addFriend(socket);

    //f界面关闭时重新挂起connect
    connect(f, &addFriend::re_ConnectSignal, this, &FriendList::re_Connect);  //连接信号和槽函数

    f->show();
}


//收到聊天请求
void FriendList::receiveChatRequest(QTcpSocket *socket, const QJsonDocument jsonDoc)
{
    QJsonObject jsonObject = jsonDoc.object();
    QString Account = jsonObject.value("fromAccount").toString();                        //发送方账号
    QString Name = jsonObject.value("fromName").toString();                              //发送方称呼
    QString Text = jsonObject.value("content").toString();                               //消息内容
    QString Time = jsonObject.value("time").toString();              //消息发送时间

    // 判断是否已经打开过聊天窗口
    if (chatWindowMap.contains(Account))
    {
        chatWindowMap[Account]->receiveMessage(Text , Time);  // 如果已经打开，直接显示
    }
    else
    {
        //弹出提示框，询问是否创建聊天窗口
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("收到聊天消息"), tr("%1(%2)向您发来了新消息，是否打开聊天窗口？").arg(Name).arg(Account),
                                        QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            Chat *c = new Chat(socket, Account, Name, m_account, m_name);
            c->show();
            c->receiveMessage(Text ,Time);       // 显示接收到的聊天内容
            chatWindowMap.insert(Account, c);    // 如果未打开，则创建并保存到 QMap 中
        }
    }

}

//聊天的用户不在线
void FriendList::isNotOnline(const QStringList &fields)
{
    QString account = fields.at(1);
    QString name = fields.at(2);
    QMessageBox::information(this, "提示", QString("%1(%2)该用户不在线，无法收到消息").arg(name).arg(account));
}

//收到添加好友请求
void FriendList::receiveAddFriend(QTcpSocket *socket, const QStringList &fields)
{
    QString account_1 = fields.at(1);                   //添加方账号
    QString name_1 = fields.at(2);                      //添加方账号称呼
    QString account_2 = fields.at(3);                   //被添加方账号
    QString name_2 = fields.at(4);                      //被添加方账号

    //弹出提示框,询问是否同意添加好友，只能选择同意或者拒绝
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("添加好友"), tr("%1(%2)请求添加您为好友，是否同意？").arg(name_1).arg(account_1),
                                    QMessageBox::Yes | QMessageBox::No);

    // 根据用户选择向服务器发送相应的回应信息
    if (reply == QMessageBox::Yes) {

        QString message = "4|0|" + account_1 + "|" + name_1 + "|" + account_2+ "|" + name_2;
        socket->write(message.toUtf8());
        socket->flush();

    }
    else
    {
        QString message = "4|1|" + account_1 + "|" + name_1 + "|" + account_2+ "|" + name_2;
        socket->write(message.toUtf8());
        socket->flush();
    }


}


//重新挂起connect
void FriendList::re_Connect()
{
    //连接服务器端的信号和槽函数
    connect(socket, &QTcpSocket::readyRead, this, [=]()
    {
        QByteArray data = socket->readAll();        // 读取服务器传来的数据
        QString message = QString::fromUtf8(data);  // 将数据转化为QString

        QStringList fields = message.split("|"); // 以管道符“|”将字符串分割成字段

        int flag = fields[0].toInt();

        switch (flag)
        {
        case 8: //聊天请求

            break;
        case 9: //添加好友请求
            receiveAddFriend(socket, fields);
            break;
        }
    });
}
