#include "addfriend.h"
#include "ui_addfriend.h"

addFriend::addFriend(QTcpSocket *s, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addFriend)
{
    ui->setupUi(this);
    socket = s;

    this->setWindowIcon(QIcon(":/images/QQ.png"));  //设置图标
    this->setWindowTitle("添加好友");      //设置窗口名称
}

addFriend::~addFriend()
{
    delete ui;
}

//添加好友按钮
void addFriend::on_pushButton_clicked()
{
    QString account = ui->lineEdit->text();

    //判断输入的账号是否为空
    if(account.isEmpty())
    {
        QMessageBox::information(this,tr("添加好友"),tr("账号不能为空！"),QMessageBox::Ok);
        return;
    }

    //发送添加好友请求给服务器
    QString message = "3|" + account;
    socket->write(message.toUtf8());
    socket->flush();

    //等待服务器回应
    connect(socket, &QIODevice::readyRead, this, [&]()
    {
        QString response = socket->readAll();   // 读取服务器回传的信息

        // 处理服务器的响应
        QStringList fields = response.split("|");
        int flag = fields[0].toInt();

        if(flag == 0)
        {
            QMessageBox::information(this,"提示","好友请求已发送，请等待对方确认！");
            // 断开连接
            disconnect(socket, &QIODevice::readyRead, this, nullptr);
        }
        else if(flag == 1)
        {
            QMessageBox::information(this,"提示","该账号不存在！");
            // 断开连接
            disconnect(socket, &QIODevice::readyRead, this, nullptr);
        }
        else if(flag == 2)
        {
            QMessageBox::information(this,"提示","该用户已经是您的好友");
            // 断开连接
            disconnect(socket, &QIODevice::readyRead, this, nullptr);
        }
        else if(flag == 3)
        {
            QMessageBox::information(this,"提示","该账号不在线！");
            // 断开连接
            disconnect(socket, &QIODevice::readyRead, this, nullptr);
        }
        else if(flag == 4)
        {
            QMessageBox::information(this,"提示","不可以添加自己为好友");
            // 断开连接
            disconnect(socket, &QIODevice::readyRead, this, nullptr);
        }

        //向父窗口发出重新连接信号和槽函数的请求
        emit re_ConnectSignal();

        close();

    });


}

