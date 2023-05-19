#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>

//无边框拖动所需头文件
#include<QPoint>
#include<QMouseEvent>
//添加阴影头文件
#include <QGraphicsDropShadowEffect>//图像投影效果

#include <register.h>
#include <friendlist.h>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_Login_clicked();

    void on_pushButton_Close_clicked();

    void on_pushButton_Min_clicked();

    void on_pushButton_Register_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket *socket;

    //无边框拖动
    QPoint last;
    bool isPressedWidget;
    void mousePressEvent(QMouseEvent *event);//鼠标点击
    void mouseMoveEvent(QMouseEvent *event);//鼠标移动
    void mouseReleaseEvent(QMouseEvent *event);//鼠标释放
};

#endif // WIDGET_H
