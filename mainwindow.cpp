#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    udpClient = new QUdpSocket(this);

    server_socket = new QTcpSocket(this);
    udpServer = new QUdpSocket(this);

    // set initial UI details
    ui->ip->setText(/*"192.0.0.1"*/ "localhost"); // default IP
    ui->port->setText("12345");   // default port

    // connect statements for TCP client socket
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::receive);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::disconect);

    connect(udpClient, &QUdpSocket::readyRead, this, &MainWindow::receive);
    connect(udpServer, &QUdpSocket::readyRead, this, &MainWindow::serverReceive);

    connect(ui->connectToServer, &QPushButton::clicked, this, &MainWindow::conect);
    connect(ui->clientSend, &QPushButton::clicked, this, &MainWindow::send);

    ui->clientSend->setShortcut(QKeySequence(Qt::Key_Enter));

    /*auto enter = new QShortcut(QKeySequence(Qt::Key_Enter), this);
    enter->setContext(Qt::WidgetWithChildrenShortcut);
    connect(enter, &QShortcut::activated, this, &MainWindow::send);*/

    // first start up a QTcpServer and then proceed
    // QTcp server
    server = new QTcpServer();

    // connect statements for TCP server sockets
    connect(server, &QTcpServer::newConnection, this, &MainWindow::serverNewConnected);
    connect(ui->serverListen, &QPushButton::clicked, this, &MainWindow::serverListen);
    connect(ui->serverSend, &QPushButton::clicked, this, &MainWindow::serverSend);

    // connect statements for client udp socket
    connect(udpClient, &QUdpSocket::readyRead, this, &MainWindow::udpClientReceive);
    udpClient->bind(QHostAddress::LocalHost, ui->port->text().toUInt());

    // connect statements for client udp socket
    connect(udpServer, &QUdpSocket::readyRead, this, &MainWindow::udpServerReceive);


}

MainWindow::~MainWindow()
{
    delete ui;
    delete server_socket;
    server->deleteLater();
    server->close();
    delete udpServer;
    delete udpClient;
}

void MainWindow::conect()
{
    //////////////////////////////////////////////////////
    // get port and IP addresses from the input boxes
    const QString IP = /*ui->ip->text().toUtf8()*/ "localhost";
    const quint16 port = ui->port->text().toUInt();

    // now connect to the remote server
    socket->abort();
    socket->connectToHost(IP, port);

    // wait for successfull connection
    if(!socket->waitForConnected())
    {
        QMessageBox::information(this, "Error", socket->errorString());
    }
    else
    {
        ui->connectToServer->setText("Connected!");
        ui->connectToServer->setEnabled(false);
        QMessageBox::information(this, "Success", "Successfully connected to the server");

        // enable the widgets
        ui->client_box->setEnabled(true);
        ui->client_input->setEnabled(true);
        ui->clientSend->setEnabled(true);
    }
}

void MainWindow::disconect()
{
    ui->client_input->setEnabled(false);
    ui->client_box->setEnabled(false);
    ui->clientSend->setEnabled(false);

    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        socket->disconnectFromHost();
    }

    QMessageBox::information(this, "Socket Disconnected", socket->errorString());
}

void MainWindow::receive()
{
    if(ui->protocol->currentText() == "TCP")
    {
        const QByteArray receivedData = socket->readAll();

        if(!receivedData.isEmpty())
        {
            // display the received data in the chat box
            const QString& data = QString(receivedData);

            ui->client_box->append(QTime::currentTime().toString());
            ui->client_box->append("Server: " + data.toUtf8());
            socket->flush();
        }
    }
    else // UDP
    {
        QByteArray buffer;
        buffer.resize(udpServer->pendingDatagramSize());

        QHostAddress sender = QHostAddress(ui->ip->text());
        quint16 senderPort = ui->port->text().toUInt();

        udpClient->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

        if(!buffer.isEmpty())
        {
            // display the received data in the chat box
            const QString& data = QString(buffer);

            ui->client_box->append(QTime::currentTime().toString());
            ui->client_box->append("Server: " + data.toUtf8());
            udpClient->flush();
        }
    }
}

void MainWindow::send()
{
    // send the input  message string  to the server and display the same in the text browser
    const QString& msg = ui->client_input->toPlainText();

    if(msg == QString())
        QMessageBox::information(this, "Failure", "Message can not be empty, please enter text");
    else
    {
        // take the message, add it to the text browser and then write it to the socket to be read by the server
        ui->client_box->append(QTime::currentTime().toString());
        ui->client_box->append("Me: " + msg.toUtf8());
        ui->client_input->clear();
        ui->client_input->setFocus();

        socket->write(msg.toUtf8());
        socket->flush();
    }
}


// server logic
void MainWindow::serverListen()
{
    const qint16 port = ui->port->text().toInt();

    if(!server->listen(QHostAddress::Any, port))
    {
        QMessageBox::critical(this,"Error！", server->errorString());
        return;
    }

    ui->serverListen->setText("Server Running");
    ui->serverListen->setEnabled(false);
    ui->serverMsgBox->setEnabled(true);
    ui->serverInputBox->setEnabled(true);
    ui->serverSend->setEnabled(true);
}

void MainWindow::serverSend()
{
    if(ui->protocol->currentText() == "TCP")
    {
        // send the input  message string  to the server and display the same in the text browser
        const QString& msg = ui->serverInputBox->toPlainText();

        if(msg == QString())
            QMessageBox::information(this, "Failure", "Message can not be empty, please enter text");
        else
        {
            // take the message, add it to the text browser and then write it to the socket to be read by the server
            ui->serverMsgBox->append(QTime::currentTime().toString());
            ui->serverMsgBox->append("Me: " + msg.toUtf8());
            ui->serverInputBox->clear();
            ui->serverInputBox->setFocus();

            server_socket->write(msg.toUtf8());
            server_socket->flush();
        }
    }
    else // UDP
    {
        // send the input  message string  to the server and display the same in the text browser
        const QString& msg = ui->serverInputBox->toPlainText();

        if(msg == QString())
            QMessageBox::information(this, "Failure", "Message can not be empty, please enter text");
        else
        {
            // take the message, add it to the text browser and then write it to the socket to be read by the server
            ui->serverMsgBox->append(QTime::currentTime().toString());
            ui->serverMsgBox->append("Me: " + msg.toUtf8());
        }
        QByteArray Data;
        Data.append(msg);
        udpServer->writeDatagram(Data, /*QHostAddress::LocalHost*/QHostAddress(ui->ip->text()), ui->port->text().toUInt());
    }
}

void MainWindow::serverNewConnected()
{
    server_socket = server->nextPendingConnection();

    connect(server_socket, &QTcpSocket::readyRead, this, &MainWindow::serverReceive);
    connect(server_socket, &QTcpSocket::disconnected, this, &MainWindow::serverDisconnect);
}

void MainWindow::serverReceive()
{
    const QByteArray receivedData = server_socket->readAll();

    if(!receivedData.isEmpty())
    {
        // display the received data in the chat box
        const QString& data = QString(receivedData);

        ui->serverMsgBox->append(QTime::currentTime().toString());
        ui->serverMsgBox->append("Client: " + data.toUtf8());
        server_socket->flush();
    }
}

void MainWindow::serverDisconnect()
{
    server->close();
}

void MainWindow::udpClientReceive()
{

}

void MainWindow::udpServerReceive()
{

}










