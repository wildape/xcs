# 
#  Copyright (C) 2015 Richard Preen <rpreen@gmail.com>
# 
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
CC=gcc

FLAGS=
CFLAGS=$(FLAGS) -Wall -Wextra -std=gnu11 -pipe -g
LDFLAGS=$(FLAGS)
LIB=-lm
 
OPT=1
GENPROF=0
USEPROF=0
XCSF=1
SAM=0

ifeq ($(XCSF),1)
	CFLAGS+= -DXCSF
else ifeq ($(XCSF),2)
	CFLAGS+= -DXCSF -DQUADRATIC
endif
ifeq ($(SAM),1)
	CFLAGS+= -DSELF_ADAPT_MUTATION
endif
ifeq ($(OPT),1)
	FLAGS+= -Ofast -march=native
endif
ifeq ($(GENPROF),1)
	FLAGS+= -fprofile-generate
endif
ifeq ($(USEPROF),1)
	FLAGS+= -fprofile-use
endif

INC=$(wildcard *.h)
SRC=$(wildcard *.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

BIN=xcs

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LDFLAGS) $(LIB)

$(OBJ): $(INC)

clean:
	$(RM) $(OBJ) $(BIN)

.PHONY: all clean
