#include "login.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/images/QQ.png"));  //设置图标

    //实现无边框窗口阴影
    this->setWindowFlags(Qt::FramelessWindowHint);  //去边框
    setAttribute(Qt::WA_TranslucentBackground);     //窗体透明
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(5);                       //设置圆角半径
    shadow->setColor(QColor(104,104,100));          //设置边框颜色
    shadow->setXOffset(-1);                         //设置向右下方偏移
    shadow->setYOffset(-1);                         //设置向右下方偏移
    ui->MainWidget->setGraphicsEffect(shadow);

}

Widget::~Widget()
{
    delete ui;
}


//实现无边框拖动**************************************************************************************************************************
void Widget::mousePressEvent(QMouseEvent *event)
{
    isPressedWidget = true; // 当前鼠标按下的即是QWidget而非界面上布局的其它控件
    last = event->globalPos();
}
void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if (isPressedWidget)
        {
            int dx = event->globalX() - last.x();
            int dy = event->globalY() - last.y();
            last = event->globalPos();
            move(x()+dx, y()+dy);
        }
}
void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    int dx = event->globalX() - last.x();
    int dy = event->globalY() - last.y();
    move(x()+dx, y()+dy);
    isPressedWidget = false; // 鼠标松开时，置为false
}
//*************************************************************************************************************************************



//关闭按钮
void Widget::on_pushButton_Close_clicked()
{
    this->close();
}

//最小化按钮
void Widget::on_pushButton_Min_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

//注册按钮
void Widget::on_pushButton_Register_clicked()
{
    Register *r= new Register();
    r->show();
}

//登录按钮
void Widget::on_pushButton_Login_clicked()
{
    //获取用户输入的账号和密码
    QString account = ui->lineEdit_account->text();
    QString password = ui->lineEdit_password->text();

    //检查账号和密码是否为空
    if (account.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "错误", "请输入账号和密码");
        return;
    }

    // 连接服务器
    QString serverAddress = "127.0.0.1"; // 服务器地址
    quint16 serverPort = 8888; // 服务器端口号
    socket = new QTcpSocket(this); // 创建QTcpSocket对象
    socket->connectToHost(serverAddress, serverPort); // 与服务器建立连接
    if (!socket->waitForConnected(3000))
    { // 等待连接成功
            QMessageBox::warning(this, "错误", "无法连接到服务器");
            return;
    }

    //发送登录请求给服务器端
    QStringList fields;
    fields << account << password;
    QString message = "0|" + fields.join("|");
    socket->write(message.toUtf8());

    //读取服务器端响应结果
    connect(socket, &QIODevice::readyRead, this, [this, account]()
    {
        QByteArray response = socket->readAll();
        QStringList fields = QString(response).split("|");



        int flag = fields[0].toInt();

        if (flag == 0)
        {
            //登录成功，给好友列表窗口传输socket, name, friendlist,并显示好友列表窗口

            // 断开当前连接
            QObject::disconnect(socket, &QIODevice::readyRead, this, nullptr);

            QString name = fields.at(1);
            QString friendlist = fields.at(2);

            FriendList *f = new FriendList(socket, account ,name, friendlist);
            f->show();
            this->close();

        }
        else if (flag == 1)
        {
            //登录失败，弹出错误提示
            QMessageBox::warning(this, "错误", "该账号已在其他设备上登陆，请检查账号信息");
            ui->lineEdit_account->clear();
            ui->lineEdit_password->clear();

        }
        else if (flag == 2)
        {
            //登录失败，弹出错误提示
            QMessageBox::warning(this, "错误", "账号或密码错误");
            ui->lineEdit_account->clear();
            ui->lineEdit_password->clear();
        }
        else
        {
            //其他错误，弹出错误提示
            QMessageBox::warning(this, "错误", "登录失败，请稍后再试");
            ui->lineEdit_account->clear();
            ui->lineEdit_password->clear();
        }
    });
}



