#include "chat.h"
#include "ui_chat.h"

#include <QIcon>
#include <QtGui/QPalette>
#include <QDateTime>
#include <QJsonDocument>

Chat::Chat(QTcpSocket *s, QString account, QString name, QString m_account, QString m_name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat)
{
    ui->setupUi(this);
    socket = s;

    this->setWindowIcon(QIcon(":/images/QQ.png"));  //设置图标
    this->setWindowTitle(name +"("+ account +")");                     //设置窗口名称

    friendAccount = account;
    friendName = name;

    my_account = m_account;
    my_name = m_name;

}

Chat::~Chat()
{
    delete ui;
}

//关闭按钮
void Chat::on_close_Button_clicked()
{
    this->close();
}

//发送消息按钮
void Chat::on_sendmsg_Button_clicked()
{
    //记录发送时间
    QDateTime dateTime = QDateTime::currentDateTime();
    ui->textBrowser->insertHtml("<span style='color:rgb(38,121,57);'>" + dateTime.toString("yyyy-MM-dd hh:mm:ss") + "</span>");

    // 将发送者名字及换行设置为textBrowser的内容：
    ui->textBrowser->insertPlainText("\n" + my_name + ": ");

    //获取textEdit中的文本：
    QString text = ui->textEdit->toPlainText();

    //将text设置为textBrowser的内容：
    ui->textBrowser->insertPlainText(text + "\n\n");

    //发送完清空textEdit
    ui->textEdit->clear();

    //向服务器发送消息
    QJsonObject jsonObject;
    jsonObject.insert("fromAccount", QJsonValue(my_account));                            //发送方账号
    jsonObject.insert("fromName",QJsonValue(my_name));                                   //发送方称呼
    jsonObject.insert("toAccount", QJsonValue(friendAccount));                           //接收方账号
    jsonObject.insert("toName",QJsonValue(friendName));                                  //接收方称呼
    jsonObject.insert("content", QJsonValue(text));                                      //消息内容
    jsonObject.insert("time", QJsonValue(dateTime.toString("yyyy-MM-dd hh:mm:ss")));     //消息发送时间

    QJsonDocument jsonDoc(jsonObject);
    QByteArray message = jsonDoc.toJson(QJsonDocument::Compact);

    socket->write(message);
}

//收到消息按钮
void Chat::receiveMessage(QString text, QString time)
{
    //记录发送者发出时间
    ui->textBrowser->insertHtml("<span style='color:rgb(38,121,57);'>" + time + "</span>");

    //将发送者名字及换行设置为textBrowser的内容：
    ui->textBrowser->insertPlainText("\n" + friendName + ": ");

    //将text设置为textBrowser的内容：
    ui->textBrowser->insertPlainText(text + "\n\n");
}
