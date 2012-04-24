/**
 * server.h
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

#ifndef SERVER_H
#define	SERVER_H

#include "common.h"
#include <boost/thread.hpp>

int main_server(int argc, char** argv);

struct player_t;
struct game_t;

struct player_t {
public:
    player_t();
    ~player_t();
    void set_tid(boost::thread * tid);
    boost::thread * get_tid();
    bool setgame(); // returns success
private:
    game_t * game;
    boost::thread *tid;
};

struct game_t {
public:
    game_t();
    ~game_t();
    bool addplayer(player_t player); // returns success
private:
    lboard_t board;
    player_t * players[2];
};


void serverhandler(int socket);




#endif	/* SERVER_H */
