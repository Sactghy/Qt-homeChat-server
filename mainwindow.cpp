#include "mainwindow.h"
#include "ui_mainwindow.h"

uint totalc{0};
std::vector<Reader> rhc;

struct r_handler
{
    tcp::socket* cur_sct;
    char readbuf[128];
    r_handler* th;

    r_handler( tcp::socket* c_t0 ) : cur_sct{ c_t0 } { }

    void operator()( const error_code& error, std::size_t tbytes )
    {
        if ( !error ) {

            rhc.push_back( Reader{ th, tbytes, cur_sct } );
            std::cout << readbuf << std::endl;
            cur_sct->async_read_some( asio::buffer( th->readbuf, 128 ), *th );

        }

        else { std::cout << error.message() << " -> socket closed" << std::endl; cur_sct->close(); }

    }

};

struct a_handler
{
    Ui::MainWindow* uu;
    io_context& iOc;
    std::vector<Connection*>& conncol;
    tcp::acceptor& acceptor;
    tcp::socket* cur_sct;
    a_handler* th;

    a_handler( Ui::MainWindow* uu0,
               io_context& iOc0,
               std::vector<Connection*>& conncol0,
               tcp::acceptor& acceptor0,
               tcp::socket* c_s0 )

     : uu{uu0}, iOc{iOc0}, conncol{conncol0}, acceptor{acceptor0}, cur_sct{c_s0} {}

    void operator()( const error_code &error )
    {
        std::string ss; QString s;

        if ( !error ) { totalc++;

             uu->txtTotal->setText( "N connections: " + s.setNum( totalc, 10 ) );

             r_handler* r_h0 = new r_handler( cur_sct ); r_h0->th = r_h0;
             cur_sct->async_read_some( asio::buffer( r_h0->readbuf, 128 ), *r_h0 );
             Connection* c0 = new Connection( cur_sct ); c0->r_handler = r_h0; c0->a_handler = th;
             std::string s0 = ">>> A client came in -";
             uu->textBrowser->append( s0.c_str() );
             for ( auto& c : conncol ) c->socket->write_some( asio::buffer( s0, s0.length() ) );
             conncol.push_back( c0 );


             tcp::socket* socket0 = new tcp::socket { iOc };
             a_handler* a_h0 = new a_handler { uu, iOc, conncol, acceptor, socket0 }; a_h0->th = a_h0;
             acceptor.async_accept( *socket0, *a_h0 );

        }

        else { ss = error.message();
               uu->txtTotal->setText( "Error - " + QString( ss.c_str() ) ); }

    }

};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this); ui->textEdit->setPlainText("Admin: ");
    ui->textBrowser->setFocusProxy(ui->textEdit);
    csr = ui->textEdit->textCursor(); csr.movePosition( QTextCursor::End, QTextCursor::KeepAnchor);

    tcp::socket* socket0 = new tcp::socket { iOc };
    a_handler* a_h0 = new a_handler { ui, iOc, conncol, acceptor, socket0 }; a_h0->th = a_h0;
    acceptor.async_accept( *socket0, *a_h0 );

    t0 = new QTimer(this); connect(t0, SIGNAL(timeout()), this, SLOT(mfnc())); t0->start(65);
    t1 = new QTimer(this); connect(t1, SIGNAL(timeout()), this, SLOT(backf())); t1->start(0);

}

MainWindow::~MainWindow()
{
    delete ui;

    for ( auto c : conncol ) { c->socket->close();
      delete c->socket;
      delete static_cast<r_handler*>(c->r_handler);
      delete static_cast<a_handler*>(c->a_handler);
      delete c;
    }
}

void MainWindow::backf()
{
    if ( !isback ) { timer.reset();

        iOc.poll();

        isback = true;

        looptime = timer.elapsed(); }
}

void MainWindow::mfnc()
{
    std::stringstream ss;

    for ( auto& r : rhc ) { uint ii{0}, res{0};

          while ( ii < r.tbytes && res != 198 ) {
          res += static_cast<r_handler*>(r.r_handler)->readbuf[ii];
          ss << static_cast<r_handler*>(r.r_handler)->readbuf[ii]; ii++;
          } std::cout << ss.str() << "-" << res << " " <<  std::endl;

          if ( r.tbytes == 3 && res == 198 ) { r.socket->close(); }

          else {
                 std::string s = ss.str();
                 ui->textBrowser->append( QString( s.data() ) );
                 for ( auto& cc : conncol ) { if ( r.socket != cc->socket )
                 cc->socket->write_some( asio::buffer( s, s.length() ) ); }
               }


     } rhc.clear();

      int oi{9999}, ii{};

            for ( auto& c : conncol ) {
                //std::cout << c->socket << " " << c->socket->is_open() << " - ";
                if ( !c->socket->is_open() ) { oi = ii; } ii++;
            } //std::cout << std::endl;

      if ( oi < 9999 ) { conncol.erase( conncol.begin() + oi ); totalc--; QString s;
      ui->txtTotal->setText( "N connections: " + s.setNum( totalc, 10 ) );
      std::string s0 = ">>> A client went away -";
      ui->textBrowser->append( s0.c_str() );
      for ( auto& c : conncol ) c->socket->write_some( asio::buffer( s0, s0.length() ) ); }


    QString s;

    ui->txtT0->setText( "T0: " + s.setNum( looptime, 'f', 6 ) );
    ui->txtT1->setText( "T1: " + s.setNum( timer.elapsed(), 'f', 6 ) );

    isback=false;
}


void MainWindow::on_pushQuit_clicked()
{
        QApplication::instance()->quit();
}

void MainWindow::on_textEdit_textChanged()
{
    QString ts = ui->textEdit->toPlainText(); int len = ts.length();
    if ( len == 6 ) {

        ui->textEdit->setPlainText("Admin: "); ui->textEdit->setTextCursor( csr );
    }
    else { QChar cc; cc = *( ts.end() - 1 );

    if ( cc.unicode() == 10 ) {

        QString ss = ui->textEdit->toPlainText();
        ss.truncate( ss.size() - 1 );
        ui->textBrowser->append( ss );
        ui->textEdit->setPlainText("Admin: "); std::string s0 = ss.toStdString();

        for ( auto& c : conncol ) { std::cout << c->socket << " " << c->socket->is_open() << std::endl;
                                    c->socket->write_some( asio::buffer( s0, ss.length() ) );
                                    std::cout << c->socket << " " << ss.toStdString() << std::endl; }

    } }

}
