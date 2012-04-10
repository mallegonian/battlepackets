/**
 * client.cpp
 * Battlepackets
 * Norwich University IS460 Spring 2012
 * Battlepackets Group - battlepackets@googlegroups.com

    Copyright 2012 Battlepackets Group

    This file is part of Battlepackets.

    Battlepackets is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Battlepackets is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Battlepackets.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "client.h"
#include "net/net.h"
#include <iostream>
#include <boost/thread.hpp>

#define log(a) m_log_buf->insert(m_log_buf->get_iter_at_offset(0), a);
int mj[BOARDSIZE][BOARDSIZE];


using namespace std;

/*
 * Main client function
 * Arguments:
 *      int argc
 *              the number of arguments on the command line
 *      int argv
 *              the text of the arguments
 */
int main(int argc, char** argv) {
    // Create structures
    Gtk::Main kit(argc, argv);
    BPwin bpwin;

    // spawn net listener
    boost::thread netin(netrecv, nethandler);

    // gtkmm main loop
    Gtk::Main::run(bpwin);

    // die
    netin.interrupt();
    return 0;
}

BPwin::BPwin() {
    gamemode = GM_START;
    placing.x = 255;





    set_title("Battlepackets!");
    set_border_width(10);
    resize(1, 1);
    add(m_box_everything);
    m_box_everything.pack_start(m_menu_bar);
    m_box_everything.pack_start(m_box_boards);
    boards[0].init(*this, 0);
    boards[1].init(*this, 1);
    m_box_boards.pack_start(boards[0].m_box_board);
    m_box_boards.pack_start(boards[1].m_box_board);
    m_box_everything.pack_start(m_box_chat);
    m_box_chat.pack_start(m_entry);
    m_box_chat.pack_start(m_log_scroll);
    m_log_scroll.add(m_log);
    m_log.set_size_request(-1, 100);
    m_log.set_editable(false);
    m_log.set_cursor_visible(false);
    m_log_buf = m_log.get_buffer();
    m_entry.signal_key_press_event().connect(
            sigc::mem_fun(*this, &BPwin::chat_key_press)
            , false);
    // Menus
    add(m_menu_game); // broken atm



    show_all_children();
}

BPwin::board::board() {
    for (int j = 0; j < BOARDSIZE; j++) { // for each column
        m_box_board.pack_start(m_box_tile_column[j]);
        for (int i = 0; i < BOARDSIZE; i++) { // for each in a column
            // Set up the tile
            m_button[i][j].set_image_position(Gtk::POS_LEFT);
            m_button[i][j].set_border_width(0);
        }
    }
}

void BPwin::board::init(BPwin & that, char which) { // which: 0=mine,1=enemy
    if (which > 1) {
        cout << "What did you break -.-\n";
        return;
    }

    int i, j;
    for (j = 0; j < BOARDSIZE; j++) { // for each column
        for (i = 0; i < BOARDSIZE; i++) { // for each in a column
            // create images
            m_img_set[0][i][j].set(M_IMG_EMPTY);
            m_img_set[0][i][j].set_padding(0, 0);
            m_img_set[1][i][j].set(M_IMG_HIT);
            m_img_set[1][i][j].set_padding(0, 0);
            if (!which) {
                m_img_set[2][i][j].set(M_IMG_SHIP);
                m_img_set[2][i][j].set_padding(0, 0);
            } else {
                m_img_set[2][i][j].set(M_IMG_MISS);
                m_img_set[2][i][j].set_padding(0, 0);
            }

            // assign image
            m_button[i][j].set_image_position(Gtk::POS_LEFT);
            m_button[i][j].set_image(m_img_set[0][i][j]);
            m_button[i][j].set_border_width(0);
            m_box_tile_column[j].pack_start(m_button[i][j]);
            if (which == 0) { // only my board needs signals
                m_button[i][j].signal_clicked().connect(
                        sigc::bind<int>(
                        sigc::mem_fun(that, &BPwin::tile_clicked)
                        , i * BOARDSIZE + j));
            }
        }
    }
}

BPwin::~BPwin() {
}

void BPwin::tile_clicked(int btn_num) {
    location loc((btn_num / BOARDSIZE), (btn_num % BOARDSIZE));
    std::cout << "Clicked: " << (int) loc.x << "," << (int) loc.y << ", gm=" << gamemode << "\n";
    switch (gamemode) {
        case GM_START: // not connected, in a game, etc.: just started the app
            log("You need to connect to a server first!\n");
            break;
        case GM_CONNECT: // connected, no initial board received
            log("Server has not sent game info!\n");
            break;
        case GM_SHIP1: // placing ship: see client.h typedef enum t_gamemode
            log("You placed a submarine!\n");
            mj[loc.x][loc.y] = 1;
#warning Change tile
#warning Send to server
            gamemode = GM_SHIP2;
            break;
        case GM_SHIP2: // placing ship: see client.h typedef enum t_gamemode
            log("You placed a submarine!\n");
            if (mj[(int) loc.x][(int) loc.y] == 1) {
                log("Dude. There's already a ship there. Try again.");
                break;
            } else
                mj[(int) loc.x][(int) loc.y] = 1;

            gamemode = GM_SHIP3;

            break;
        case GM_SHIP3: // placing ship: see client.h typedef enum t_gamemode
            if (mj[(int) loc.x][(int) loc.y] == 1) {
                log("Dude. There's already a ship there. Try again.");
                break;
            }
            if (placing.x == 255) {
                log("You placed a destroyer!\n");
                mj[(int) loc.x][(int) loc.y] = 1;
                prev.x = (int) loc.x;
                prev.y = (int) loc.y;

                placing.x--;
                break;

            } else if (placing.x == 244) {
                loc.x = (int) loc.x;
                loc.y = (int) loc.y;

                if ((loc.x == prev.x && ((loc.y == prev.y || loc.y == (prev.y - 1)) || loc.y == (prev.y + 1))) || (loc.x == (prev.x - 1) && loc.y == (prev.y + 1)) || (loc.x == (prev.x + 1) && loc.y == (prev.y + 1))) {
                    log("You placed a destroyer!!!\n");
                    mj[(int) loc.x][(int) loc.y] = 1;
                    placing.x = 255;
                    gamemode = GM_SHIP4;

                    break;
                } else {
                    log("You can't split up your ship!!!\n");
                    break;
                }

            }



        case GM_SHIP4: // placing ship: see client.h typedef enum t_gamemode
            if (mj[(int) loc.x][(int) loc.y] == 1) {
                log("Dude. There's already a ship there. Try again.");
                break;
            }
            if (placing.x == 255) {
                log("You placed a destroyer!\n");
                mj[(int) loc.x][(int) loc.y] = 1;
                prev.x = (int) loc.x;
                prev.y = (int) loc.y;

                placing.x--;
                break;

            } else if (placing.x == 244) {
                loc.x = (int) loc.x;
                loc.y = (int) loc.y;

                if ((loc.x == prev.x && ((loc.y == prev.y || loc.y == (prev.y - 1)) || loc.y == (prev.y + 1))) || (loc.x == (prev.x - 1) && loc.y == (prev.y + 1)) || (loc.x == (prev.x + 1) && loc.y == (prev.y + 1))) {
                    log("You placed a destroyer!!!\n");
                    mj[(int) loc.x][(int) loc.y] = 1;
                    placing.x = 255;
                    gamemode = GM_SHIP5;

                    break;
                } else {
                    log("You can't split up your ship!!!\n");
                    break;
                }
            }



            break;
        case GM_SHIP5: // placing ship: see client.h typedef enum t_gamemode
            if (mj[(int) loc.x][(int) loc.y] == 1) {
                log("Dude. There's already a ship there. Try again.");
                break;
            }
            if (placing.x == 255) {
                log("You placed a cruiser!\n");
                mj[(int) loc.x][(int) loc.y] = 1;
                prev.x = (int) loc.x;
                prev.y = (int) loc.y;

                placing.x--;
                break;

            } else if (placing.x == 244 || placing.x == 243) {
                loc.x = (int) loc.x;
                loc.y = (int) loc.y;

                if ((loc.x == prev.x && ((loc.y == prev.y || loc.y == (prev.y - 1)) || loc.y == (prev.y + 1))) || (loc.x == (prev.x - 1) && loc.y == (prev.y + 1)) || (loc.x == (prev.x + 1) && loc.y == (prev.y + 1))) {
                    log("You placed a cruiser!!!\n");
                    mj[(int) loc.x][(int) loc.y] = 1;
                    prev.x = (int) loc.x;
                    prev.y = (int) loc.y;
                    if (placing.x == 243) {
                        placing.x = 255;
                        gamemode = GM_SHIP6;
                        break;
                    }
                    placing.x--;

                    break;
                } else {
                    log("You can't split up your ship!!!\n");
                    break;
                }
            }



        case GM_SHIP6: // placing ship: see client.h typedef enum t_gamemode
            if (mj[(int) loc.x][(int) loc.y] == 1) {
                log("Dude. There's already a ship there. Try again.");
                break;
            }
            if (placing.x == 255) {
                log("You placed a battleship!\n");
                mj[(int) loc.x][(int) loc.y] = 1;
                prev.x = (int) loc.x;
                prev.y = (int) loc.y;

                placing.x--;
                break;

            } else if (placing.x == 244 || placing.x == 243 || placing.x == 242) {
                loc.x = (int) loc.x;
                loc.y = (int) loc.y;

                if ((loc.x == prev.x && ((loc.y == prev.y || loc.y == (prev.y - 1)) || loc.y == (prev.y + 1))) || (loc.x == (prev.x - 1) && loc.y == (prev.y + 1)) || (loc.x == (prev.x + 1) && loc.y == (prev.y + 1))) {
                    log("You placed a battleship!!\n");
                    mj[(int) loc.x][(int) loc.y] = 1;
                    prev.x = (int) loc.x;
                    prev.y = (int) loc.y;
                    if (placing.x == 242) {
                        placing.x = 255;
                        gamemode = GM_SHIP7;
                        break;
                    }
                    placing.x--;

                    break;
                } else {
                    log("You can't split up your ship!!!\n");
                    break;
                }
            }

        case GM_SHIP7: // placing ship: see client.h typedef enum t_gamemode
            if (mj[(int) loc.x][(int) loc.y] == 1) {
                log("Dude. There's already a ship there. Try again.");
                break;
            }
            if (placing.x == 255) {
                log("You placed a carrier!\n");
                mj[(int) loc.x][(int) loc.y] = 1;
                prev.x = (int) loc.x;
                prev.y = (int) loc.y;

                placing.x--;
                break;

            } else if (placing.x == 244 || placing.x == 243 || placing.x == 242 || placing.x == 241) {
                loc.x = (int) loc.x;
                loc.y = (int) loc.y;

                if ((loc.x == prev.x && ((loc.y == prev.y || loc.y == (prev.y - 1)) || loc.y == (prev.y + 1))) || (loc.x == (prev.x - 1) && loc.y == (prev.y + 1)) || (loc.x == (prev.x + 1) && loc.y == (prev.y + 1))) {
                    log("You placed a carrier!!\n");
                    mj[(int) loc.x][(int) loc.y] = 1;
                    prev.x = (int) loc.x;
                    prev.y = (int) loc.y;
                    if (placing.x == 241) {
                        placing.x = 255;
                        gamemode = GM_PLAYTIME;
                        break;
                    }
                    placing.x--;

                    break;
                } else {
                    log("You can't split up your ship!!!\n");
                    break;
                }
            }
#warning Do stuff
            break;
        case GM_PLAYTIME: // ingame
            log("Server has not sent game info!\n");
            // TODO: Do stuff
            // TODO: TODO: Change board based on clicked tile
            // TODO: TODO: Send change, if valid, to server
            break;
        default: // should never get here
            log("What just happened?!\n");
            break;
    }
}

/* Called when a key is pressed in the chat input box
 * return value is apparently whether or not to show the char in the box?
 * possibly whether or not the event has been handled - seems to spam this when false.
 */
bool BPwin::chat_key_press(GdkEventKey* k) {
    //std::cout << "chat box key press: " << k->keyval << "=" << (char)k->keyval << "\n";
    if (k->keyval == 65293) { // return
        if (m_entry.get_text_length() < 1) {
            return true;
        }
        //log
        log("\n");
        log(m_entry.get_text());
        log("Me: ");
        //send
        // TODO: TODO: Send chat to server
        std::cout << "Send chat: " << m_entry.get_text() << std::endl;
        m_entry.set_text("");
        return true;
    }

    // else a key other than enter
    return false;
}

void nethandler(int sockfd, char * data[], int datalen) {
    // grab first char (packet id)
    // switch(packetID){etc}
    // do stuff like update the gui
}
