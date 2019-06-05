#include "remote_process_manager.h"
//#include "ui_remote_process_manager.h"

namespace m1
{
    remote_process_manager::remote_process_manager()
    {

        //Window settings
        this->setWindowTitle("RemoteProcessManager");
        QScreen* screen = QApplication::screens().at(0);
        this->setFixedSize(screen->availableSize());

        //Network
        m_socket = new QTcpSocket();
        connect( m_socket, &QTcpSocket::connected, this ,  &remote_process_manager::connected_to_server );
        m_timer_receive = new QTimer();

        m_socket_find = new QUdpSocket();
        m_socket_find->bind();

        m_timer_find = new QTimer();
        connect(m_timer_find, &QTimer::timeout, this, &remote_process_manager::find_server);

        //UI CORE
        m_parent_widget = new QWidget();
        m_parent_widget->setParent(this);

        m_layout_parent = new QGridLayout();
        m_layout_parent->setMargin(0);
        m_layout_parent->setSpacing(0);
        m_layout_parent->setSizeConstraint(m_layout_parent->SetMinAndMaxSize);

        m_parent_widget->setLayout(m_layout_parent);

        m_toolbutton = new QToolButton();
        m_toolbutton->setFixedHeight(size().height() * 0.06);
        m_toolbutton->setFont(QFont("Verdana", 12));
        m_toolbutton->setText("Menu");

        m_toolbutton->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);

        m_menu_toolbutton = new QMenu();
        std::string style_sheet = "QMenu::item {font-family: Verdana; font-size: 20pt; color: #000000; padding: ";
        style_sheet += std::to_string(size().height() * 0.05) + " ";
        style_sheet += std::to_string(size().width() * 0.3) + " ";
        style_sheet += std::to_string(size().height() * 0.05) + " ";
        style_sheet += "0";
        style_sheet += ";} QMenu::item:selected {background: #AAAAAA;}";
        m_menu_toolbutton->setStyleSheet(QString::fromStdString(style_sheet));
        m_menu_toolbutton->addAction("Servers", this, &remote_process_manager::ui_startscreen)/*->setFont(QFont("Verdana", 28))*/;
        m_menu_toolbutton->addAction("Process Manager", this, &remote_process_manager::ui_process_manager)/*->setFont(QFont("Verdana", 28))*/;
        m_menu_toolbutton->addAction("Disconnect from Server", this, &remote_process_manager::disconnected_from_server);
        m_menu_toolbutton->addAction("About", this, &remote_process_manager::ui_about)/*->setFont(QFont("Verdana", 28))*/;

        m_toolbutton->setMenu(m_menu_toolbutton);
        m_layout_parent->setMenuBar(m_toolbutton);

        //UI Startscreen
        m_scrollarea_server = new QScrollArea();
        m_scrollarea_server->setFixedWidth(size().width());
        m_scrollarea_server->setFixedHeight(size().height() * 0.74);
        m_scrollarea_server->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

        m_layout_scrollarea_server = new QGridLayout();
        m_layout_scrollarea_server->setMargin(0);
        m_layout_scrollarea_server->setSpacing(0);
        m_layout_scrollarea_server->setSizeConstraint(m_layout_scrollarea_server->SetMinAndMaxSize);

        m_container_scrollarea_server = new QWidget();
        m_container_scrollarea_server->setLayout(m_layout_scrollarea_server);

        m_scrollarea_server->setWidget(m_container_scrollarea_server);

        m_label_server = new QLabel("Servers");
        m_label_server->setFixedWidth(size().width());
        m_label_server->setFixedHeight(size().height() * 0.1);
        m_label_server->setAlignment(Qt::AlignCenter);
        m_label_server->setFont(QFont("Verdana", 22));

        m_button_rescan = new QPushButton("Rescan");
        m_button_rescan->setFixedWidth(size().width());
        m_button_rescan->setFixedHeight(size().height() * 0.1);
        m_button_rescan->setFont(QFont("Verdana", 14));
        connect(m_button_rescan, &QPushButton::pressed, this, &remote_process_manager::contact_server);

        //UI MAIN
        m_listwidget_processes = new QListWidget();
        m_listwidget_processes->setFixedWidth(size().width());
        m_listwidget_processes->setFixedHeight(size().height() * 0.86);

        m_button_kill = new QPushButton();
        m_button_kill->setText("Kill selected process");
        m_button_kill->setFixedHeight(size().height()*0.08);
        m_button_kill->setFont(QFont("Verdana", 12));

        //UI ABOUT
        m_textedit_about = new QTextEdit();
        m_textedit_about->setFixedWidth(size().width() * 1);
        m_textedit_about->setFixedHeight(size().height() * 0.94);
        m_textedit_about->setReadOnly(true);
        m_textedit_about->setAlignment(Qt::AlignCenter);
        m_textedit_about->setFont(QFont("Arial", 12));

        //Load about.txt
        QFile file(":/data/txt/about.txt");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            //qDebug()<<"Could not find about.txt";
        }
        QString text = QString::fromStdString(file.readAll().toStdString());
        m_textedit_about->setText(text);

        //ETC
        m_button_connect = new QPushButton();
        m_button_connect->setFixedWidth(size().width());
        m_button_connect->setText("Connect to server");

        m_lineedit_ip = new QLineEdit();
        m_lineedit_ip->setFixedWidth(size().width());
        m_lineedit_ip->setText("Insert server address");

        //
        m_layout_parent->addWidget(m_label_server);
        m_label_server->setVisible(true);
        m_layout_parent->addWidget(m_scrollarea_server);
        m_scrollarea_server->setVisible(true);
        m_layout_parent->addWidget(m_button_rescan);
        m_button_rescan->setVisible(true);

        m_timer_find->start(500);
        contact_server();
    }

    remote_process_manager::~remote_process_manager()
    {
        m_button_connect->deleteLater();
        m_lineedit_ip->deleteLater();
        m_button_kill->deleteLater();
        m_listwidget_processes->deleteLater();
        m_label_server->deleteLater();
        m_button_rescan->deleteLater();
        for(int i = 0; i < m_buttons_scrollarea_server.size(); ++i)
        {
            m_buttons_scrollarea_server.at(i)->deleteLater();
        }
        m_container_scrollarea_server->deleteLater();
        m_scrollarea_server->deleteLater();
        m_layout_scrollarea_server->deleteLater();
        m_menu_toolbutton->deleteLater();
        m_toolbutton->deleteLater();
        m_layout_parent->deleteLater();
        m_parent_widget->deleteLater();
        m_timer_find->deleteLater();
        m_timer_receive->deleteLater();
        m_socket_find->deleteLater();
        m_socket->deleteLater();

    }

    void remote_process_manager::connect_to_server(QHostAddress address)
    {
        m_socket->abort();
        m_socket->connectToHost(address, 55556);
    }

    void remote_process_manager::connected_to_server()
    {
        //qDebug()<<"Connected to server";

        m_layout_parent->removeWidget(m_button_connect);
        m_button_connect->setVisible(false);
        m_layout_parent->removeWidget(m_lineedit_ip);
        m_lineedit_ip->setVisible(false);
        m_layout_parent->removeWidget(m_scrollarea_server);
        m_scrollarea_server->setVisible(false);
        m_layout_parent->removeWidget(m_label_server);
        m_label_server->setVisible(false);
        m_layout_parent->removeWidget(m_button_rescan);
        m_button_rescan->setVisible(false);

        m_layout_parent->addWidget(m_listwidget_processes, 0, 0);
        m_listwidget_processes->setVisible(true);
        m_layout_parent->addWidget(m_button_kill, 1, 0);
        m_button_kill->setVisible(true);

        connect(m_button_kill, &QPushButton::pressed, this, &remote_process_manager::kill_button_pressed);
        connect(m_timer_receive, SIGNAL( timeout() ), this, SLOT(receive_packets()) );
        connect(m_socket, &QTcpSocket::disconnected, this, &remote_process_manager::disconnected_from_server);

        m_timer_receive->start(1000);
    }

    void remote_process_manager::receive_packets()
    {
        //qDebug()<<"trying to receive packets!";

        if(m_socket && m_socket->state() == QAbstractSocket::ConnectedState && m_socket->bytesAvailable() > 0)
        {
            //qDebug()<<"receiving packets!";

            static unsigned int num_processes_in_packet;
            num_processes_in_packet = m_socket->bytesAvailable() / sizeof(m1::network_packet_process);

            //qDebug()<<"num_processes_received:"<<num_processes_in_packet<<" bytes available: "<<m_socket->bytesAvailable();
            static char buffer[sizeof(m1::network_packet_process)];
            static m1::network_packet_process cache_packet;

            for(int i = 0; i < num_processes_in_packet; ++i)
            {
                m_socket->read(buffer, sizeof(m1::network_packet_process));
                memcpy(&cache_packet, buffer, sizeof(m1::network_packet_process));

                if(cache_packet.packet_id == NETWORK_PACKET_PROCESS_FIRST)
                {
                    m_processes.clear();
                    m_processes.shrink_to_fit();
                }
                m_processes.push_back(ui_process());
                m_processes.back().packet = cache_packet;
                m_processes.back().create_ui_string();

                if(cache_packet.packet_id == NETWORK_PACKET_PROCESS_LAST)
                {
                    //Delete every entry, that is not valid anymore
                    bool found_entry = false;
                    for(int i = 0; i < m_listwidget_processes->count();++i)
                    {
                        found_entry = false;
                        for(int j = 0; j < m_processes.size(); ++j)
                        {
                            if(m_processes.at(j).ui_string == m_listwidget_processes->item(i)->text().toStdString())
                            {
                                found_entry = true;
                            }
                        }
                        if(found_entry == false)
                        {
                            m_listwidget_processes->takeItem(i);
                        }
                    }

                    //Add all new entries
                    for(int i= 0; i < m_processes.size();++i)
                    {
                        found_entry = false;
                        //Search if an entry with the same pid/name exists
                        for(int j = 0; j < m_listwidget_processes->count();++j)
                        {
                            if(m_listwidget_processes->item(j)->text().toStdString() == m_processes.at(i).ui_string)
                            {
                                found_entry = true;
                            }
                        }
                        if(found_entry == false)
                        {
                            m_listwidget_processes->addItem(m_processes.at(i).ui_string.c_str());
                            m_listwidget_processes->item(m_listwidget_processes->count()-1)->setFont(QFont("Verdana", 16 ));
                            //Set QListWidgetItem custom value to pid of process
                            m_listwidget_processes->item(m_listwidget_processes->count()-1)->setData(256, m_processes.at(i).packet.process_id);
                        }
                    }
                }
            }
        }
    }

    void remote_process_manager::disconnected_from_server()
    {
        //qDebug()<<"Disconnected from Server";
        m_socket->abort();
        m_timer_receive->stop();
        m_listwidget_processes->clear();

        reset_server_list();

        ui_startscreen();

    }

    void remote_process_manager::kill_button_pressed()
    {
        m1::network_packet_kill kill_packet;
        kill_packet.packet_id = NETWORK_PACKET_KILL;
        char cache[sizeof(m1::network_packet_kill)];
        //kill_packet.process_id = m_listwidget_processes->
        for( int i= 0; i < m_listwidget_processes->selectedItems().size(); ++i)
        {
            kill_packet.process_id = m_listwidget_processes->selectedItems().at(i)->data(256).toLongLong();
            memcpy(cache, &kill_packet, sizeof(m1::network_packet_kill));
            //qDebug()<<"Send kill packet with pid: "<<kill_packet.process_id;
            m_socket->write(cache, sizeof(m1::network_packet_kill));
        }
    }

    void remote_process_manager::find_server()
    {
        //qDebug()<<"find_server";
        if (m_socket_find)
        {
            if (m_socket_find->hasPendingDatagrams())
            {
                QNetworkDatagram dg = m_socket_find->receiveDatagram();
                //qDebug()<<"Server has responded: "<<dg.senderAddress().toString();

                for(int i = 0; i < m_found_server_addresses.size(); ++i)
                {
                    if(m_found_server_addresses.at(i) == dg.senderAddress())
                    {
                        //qDebug()<<"This Server was already found";
                        return;
                    }
                }

                static char cache[20];

                memcpy(cache, dg.data().data(), sizeof(packet_identifier));

                packet_identifier packet_id = 0;
                memcpy(&packet_id, cache, sizeof(packet_identifier));

                if(packet_id != NETWORK_PACKET_FIND)
                {
                    //qDebug()<<"Non expected packet type received";
                    return;
                }

                static unsigned short num_server = 0;
                ++num_server;

                //copy computername from packet to cache
                memcpy(cache, dg.data().data() + sizeof(packet_identifier), 16);

                static std::string cname = cache;
                cname = cache;
                cname.insert(0, "Connect to: ");

                //Add server to scrollarea;
                m_buttons_scrollarea_server.push_back(new QPushButton( QString::fromStdString(cname) ));
                m_buttons_scrollarea_server.back()->setFixedWidth(size().width());
                m_buttons_scrollarea_server.back()->setFixedHeight(size().height() * 0.1);
                m_layout_scrollarea_server->addWidget(m_buttons_scrollarea_server.back());
                m_buttons_scrollarea_server.back()->setVisible(true);
                connect( m_buttons_scrollarea_server.back(), &QPushButton::pressed, this, [=](){ this->connect_to_server( dg.senderAddress() ); } );

                m_found_server_addresses.push_back(dg.senderAddress());
            }
        }
    }



    void remote_process_manager::contact_server()
    {

        m_button_rescan->setText("Rescanning...");
        QCoreApplication::processEvents();

        reset_server_list();

        static std::string mask_prefix;

        QList<QHostAddress> local_ips = QNetworkInterface::allAddresses();

        for (int i = 0; i < local_ips.size(); ++i)
        {
            if (local_ips.at(i) != QHostAddress::LocalHost && local_ips.at(i).toIPv4Address())
            {
                mask_prefix = local_ips.at(i).toString().toStdString();
            }
        }

        static unsigned short removed_parts = 0;
        removed_parts = 0;
        while(removed_parts < 2)
        {

            if(mask_prefix.back() == '.')
            {
                ++removed_parts;
            }
            mask_prefix.pop_back();

        }

        //qDebug()<<"Mask prefix: " << mask_prefix.c_str();
        static QHostAddress address;
        static std::string address_string;

        //qDebug()<<"Contacting server";
        for(int i = 0; i < 256; ++i)
        {
            for(int j = 0; j < 256; ++j)
            {
                address_string = mask_prefix + "." + std::to_string(i) + "." + std::to_string(j);
                address.setAddress(QString::fromStdString(address_string));
                m_socket_find->writeDatagram(QByteArray("Hello There"), address, 55557);
            }
        }
        //qDebug()<<"Contacted server";
        m_button_rescan->setText("Rescan");
    }

    void remote_process_manager::ui_startscreen()
    {
        remove_all_widgets_from_parent_layout();

        for(int i=0; i < m_buttons_scrollarea_server.size(); ++i)
        {
            m_layout_parent->removeWidget(m_buttons_scrollarea_server.at(i));
            DEF_SAFE_DELETE(m_buttons_scrollarea_server.at(i));
        }

        m_found_server_addresses.clear();
        m_found_server_addresses.shrink_to_fit();

        m_buttons_scrollarea_server.clear();
        m_buttons_scrollarea_server.shrink_to_fit();

        contact_server();

        m_layout_parent->addWidget(m_label_server, 0, 0);
        m_label_server->setVisible(true);
        m_layout_parent->addWidget(m_scrollarea_server, 1, 0);
        m_scrollarea_server->setVisible(true);
        m_layout_parent->addWidget(m_button_rescan, 2, 0);
        m_button_rescan->setVisible(true);
    }

    void remote_process_manager::ui_process_manager()
    {
        remove_all_widgets_from_parent_layout();
        m_layout_parent->addWidget(m_listwidget_processes);
        m_listwidget_processes->setVisible(true);
        m_layout_parent->addWidget(m_button_kill);
        m_button_kill->setVisible(true);
    }

    void remote_process_manager::ui_about()
    {
        remove_all_widgets_from_parent_layout();

        m_layout_parent->addWidget(m_textedit_about);
        m_textedit_about->setVisible(true);

    }

    void remote_process_manager::remove_all_widgets_from_parent_layout()
    {
        while (QLayoutItem* item = m_layout_parent->takeAt(0))
        {
            item->widget()->setVisible(false);
            delete item;
        }
    }

    void remote_process_manager::reset_server_list()
    {
        m_found_server_addresses.clear();
        m_found_server_addresses.shrink_to_fit();

        for(int i= 0; i < m_buttons_scrollarea_server.size(); ++i)
        {
            m_layout_scrollarea_server->removeWidget(m_buttons_scrollarea_server.at(i));
            m_buttons_scrollarea_server.at(i)->deleteLater();
        }

        m_buttons_scrollarea_server.clear();
        m_buttons_scrollarea_server.shrink_to_fit();
    }



}//End of namespace m1
