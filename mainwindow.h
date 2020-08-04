#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QShortcut>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    // for client TCP
    void conect();
    void disconect();

    // for server TCP
    void serverListen();
    void serverSend();
    void serverNewConnected();

    // UDP client
    void udpClientReceive();
    void udpServerReceive();


    // common functions
    void send(); // client
    void receive();

    void serverReceive(); // server
    void serverDisconnect();

private:
    Ui::MainWindow *ui;
    QTcpSocket* socket;
    QUdpSocket* udpClient;

    QTcpServer* server;
    QTcpSocket* server_socket;
    QUdpSocket* udpServer;
};
#endif // MAINWINDOW_H
