/*
 * LCDduino Controller for Twisted Pear Opus DAC and Mux
 * Simeon Walker 2011-2012
 *
 * Derived from Volu-Master(tm) by Bryan Levin (Linux-Works Labs)
 * Copyright (c) 2009-2011 Bryan Levin
 * Project website: http://www.amb.org/audio/lcduino-1/
 *
 *  LICENSE
 *  -------
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// general MCP i2c register codes
#define MCP_REG_IODIR                0x00
#define MCP_REG_IPOL                 0x01
#define MCP_REG_GPINTEN              0x02
#define MCP_REG_DEFVAL               0x03
#define MCP_REG_INTCON               0x04
#define MCP_REG_IOCON                0x05
#define MCP_REG_GPPU                 0x06
#define MCP_REG_INTF                 0x07
#define MCP_REG_INTCAP               0x08
#define MCP_REG_GPIO                 0x09
#define MCP_REG_OLAT                 0x0A

//command bytes for LCD
#define CMD_CLR                      0x01
#define CMD_RIGHT                    0x1C
#define CMD_LEFT                     0x18
#define CMD_HOME                     0x02

