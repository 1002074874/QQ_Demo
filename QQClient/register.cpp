#include "register.h"
#include "ui_register.h"

Register::Register(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/images/QQ.png"));  //设置图标
    this->setWindowTitle("注册");      //设置窗口名称

    socket = new QTcpSocket;
}

Register::~Register()
{
    delete ui;
}

// 注册按钮
void Register::on_pushButton_clicked()
{
    // 获取用户输入的账号、密码和称呼
    QString account = ui->lineEdit_account->text();
    QString password = ui->lineEdit_password->text();
    QString name = ui->lineEdit_name->text();

    // 检查账号、密码和称呼是否为空
    if (account.isEmpty() || password.isEmpty() || name.isEmpty())
    {
        QMessageBox::warning(this, "错误", "请输入账号、密码和称呼");
        return;
    }

    // 连接服务器
     QString serverAddress = "127.0.0.1"; // 服务器地址
     quint16 serverPort = 8888; // 服务器端口号
     socket = new QTcpSocket(this); // 创建QTcpSocket对象
     socket->connectToHost(serverAddress, serverPort); // 与服务器建立连接
     if (!socket->waitForConnected(3000))
     {
         // 等待连接成功
         QMessageBox::warning(this, "错误", "无法连接到服务器");
         return;
     }

     //发送注册请求给服务器端
     QStringList fields;
     fields << account << password << name;
     QString message = "1|" + fields.join("|");
     socket->write(message.toUtf8());

     // 读取服务器端响应结果
        connect(socket, &QIODevice::readyRead, this, [this]()
        {
            QByteArray response = socket->readAll();

            if (response == "0")
            {
                // 注册成功，弹出提示并返回登录窗口
                QMessageBox::information(this, "提示", "注册成功");
                ui->lineEdit_account->clear();
                ui->lineEdit_password->clear();
                ui->lineEdit_name->clear();
            }
            else if (response == "1账号已被注册")
            {
                // 登录失败，弹出错误提示
                QMessageBox::warning(this, "错误", "该账号已被注册，请更换其他账号");
                ui->lineEdit_account->clear();
                ui->lineEdit_password->clear();
                ui->lineEdit_name->clear();
            }
            else
            {
                // 其他错误，弹出错误提示
                QMessageBox::warning(this, "错误", "注册失败，请稍后再试");
                ui->lineEdit_account->clear();
                ui->lineEdit_password->clear();
                ui->lineEdit_name->clear();
            }
        });
}
