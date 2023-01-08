#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>

#include <iostream>
#include <string>
#include <vector>

#include <QMainWindow>
#include <QTimer>
#include <QTextCursor>

class Timer
{
private:

    using clock_type = std::chrono::steady_clock;
    using second_type = std::chrono::duration< double, std::ratio<1,60> >;

    std::chrono::time_point<clock_type> m_beg;

public:

    Timer() : m_beg { clock_type::now() } { }

    void reset() { m_beg = clock_type::now(); }

    double elapsed() const
    { return std::chrono::duration_cast<second_type>( clock_type::now() - m_beg ).count(); }
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace asio; using namespace asio::ip;

class Connection
{
    public : tcp::socket* socket; void* r_handler; void* a_handler;
    Connection ( tcp::socket* s0 ) : socket {s0} {}
};

class Reader
{
    public : void* r_handler; std::size_t tbytes; tcp::socket* socket;
    Reader ( void* r0, std::size_t tb0, tcp::socket* s0 ) : r_handler {r0}, tbytes{tb0}, socket{s0} {}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:



    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:

    void on_textEdit_textChanged();

    void mfnc();

    void backf();

    void on_pushQuit_clicked();

private:

    Ui::MainWindow *ui;

    bool isback{false};
    double looptime{0};
    uint chcnt{0};

    QTimer *t0, *t1;
    Timer timer;
    QTextCursor csr;

    io_context iOc;
    tcp::acceptor acceptor { iOc , tcp::endpoint { tcp::v4(), 31313 } };

    std::vector<Connection*> conncol;


};
#endif // MAINWINDOW_H





