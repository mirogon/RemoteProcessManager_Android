#ifndef REMOTE_PROCESS_MANAGER_H
#define REMOTE_PROCESS_MANAGER_H

//Core
#include <memory>
#include <vector>
#include <string>

//Qt
#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QLabel>
#include <QTcpSocket>
#include <QtWidgets/QLineEdit>
#include <QFile>
#include <QtWidgets/QGridLayout>
#include <QUdpSocket>
#include <QToolButton>
#include <QMenu>
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QTextEdit>
#include <QTimer>
#include <QPixmap>
#include <QScreen>
#include <QtWidgets/QPushButton>
#include <QHostAddress>
#include <QtWidgets/QMainWindow>
#include <QApplication>

/*namespace Ui {
class remote_process_manager;
}*/

namespace m1
{
    #define DEF_SAFE_DELETE(x) if(x != nullptr){delete x; x = nullptr;};

    enum e_packet_identifier{NETWORK_PACKET_PROCESS_FIRST = 5, NETWORK_PACKET_PROCESS = 6, NETWORK_PACKET_PROCESS_LAST = 7, NETWORK_PACKET_KILL = 8, NETWORK_PACKET_FIND = 9};
    typedef uint32_t packet_identifier;

    struct network_packet_process
    {
        packet_identifier packet_id;
        uint32_t process_id;
        char process_name[64];
    };

    struct network_packet_kill
    {
        packet_identifier packet_id;
        uint32_t process_id;
    };

    struct ui_process
    {
        network_packet_process packet;
        std::string ui_string;

        void create_ui_string()
        {
            ui_string = packet.process_name;
            ui_string +=  " | PID: " + std::to_string(packet.process_id);
        }

    };

    class remote_process_manager : public QWidget
    {
       Q_OBJECT

    public:
        explicit remote_process_manager();
        ~remote_process_manager();


    public slots:

        void connect_to_server(QHostAddress address);
        void connected_to_server();

        void receive_packets();
        void disconnected_from_server();

        void kill_button_pressed();

        void find_server();

        void ui_startscreen();
        void ui_process_manager();
        void ui_about();


    private /*functions*/:
        void contact_server();
        void remove_all_widgets_from_parent_layout();
        void reset_server_list();

    private:

        //Network
        QTcpSocket* m_socket;
        QTimer* m_timer_receive;

        QUdpSocket* m_socket_find;
        QTimer* m_timer_find;

        //UI Core
        QWidget* m_parent_widget;
        QGridLayout* m_layout_parent;

        QToolButton* m_toolbutton;
        QMenu* m_menu_toolbutton;

        //UI Startscreen
        QScrollArea* m_scrollarea_server;
        QGridLayout* m_layout_scrollarea_server;
        QWidget* m_container_scrollarea_server;
        QLabel* m_label_server;

        std::vector<QPushButton*> m_buttons_scrollarea_server;
        QPushButton* m_button_rescan;

        //UI Main
        std::vector<ui_process> m_processes;
        QListWidget* m_listwidget_processes;
        QPushButton* m_button_kill;

        //UI About
        QTextEdit* m_textedit_about;

        //etc
        std::vector<QHostAddress> m_found_server_addresses;

        QPushButton* m_button_connect;
        QLineEdit* m_lineedit_ip;
    };

}//End of namespace m1
#endif // REMOTE_PROCESS_MANAGER_H
