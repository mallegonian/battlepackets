/**
 * common.h
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

#ifndef COMMON_H
#define	COMMON_H

#define SERVPORT 7777
#define BOARDSIZE 10
#define PROTOVERSION 0
#define MAXDATASIZE 1500

#include <stdint.h>
#include <string.h>
#include "iostream"

struct location {
    uint8_t x;
    uint8_t y;
    location();
    location(uint8_t ix, uint8_t iy);
    void set(uint8_t ix, uint8_t iy);
    void set(location il);
};

struct lboard_t {
public:
    lboard_t();
    void import(uint8_t * board);
    // bool player: 0=self 1=enemy
    bool get_ship(bool player, location loc); // won't cheat on client: no data
    void set_ship(bool player, location loc);
    bool get_fired(bool player, location loc);
    void set_fired(bool player, location loc);
    uint8_t get_tile_raw(location loc);
private:
    uint8_t board_data[BOARDSIZE][BOARDSIZE];
};


// Packet types

struct handshake_t {
private:
    char pktid; // = 0
public:
    char protover; // = PROTOVERSION
    char boardsize; // = BOARDSIZE
    char username[19 - 3];
    char gameid[52 - 20];

    handshake_t();
    handshake_t(char * data, int datalen);
};

struct move_t {
private:
    char pktid; // = 1
public:
    location loc;

    typedef enum {
        ACT_MOVE = 0,
        ACT_PLACE = 1
    } action_t;
    action_t action; // (see protocol)

    move_t();
    move_t(char * data, int datalen);
};

struct refresh_t {
private:
    char pktid; // = 2
public:
    lboard_t board; //each [x][y] cordinate will have a specific absolute state (0-4) (see protocol)

    refresh_t();
    refresh_t(char * data, int datalen);
};

struct chat_t {
private:
    char pktid; // = 3
public:
    char msg[255];
    chat_t();
    chat_t(char * data, int datalen);
};


#endif	/* COMMON_H */
