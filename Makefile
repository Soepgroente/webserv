NAME		:= webserv
T_EXEC		:= webserv_tester
CC			:= c++
CPPFLAGS	= -Wall -Wextra -Werror -std=c++20 $(HEADERS) -Ofast -flto #-DNDEBUG #-g #-fsanitize=address #-DNDEBUG#
HEADERS		:= -I include
T_HEADERS	:= -I $(T_DIR)/include

ifeq ($(shell uname), Darwin)
    CPUCORES := $(shell sysctl -n hw.ncpu)
else
    CPUCORES := $(shell nproc)
endif
MAKEFLAGS	+= -j$(CPUCORES)
export MAKEFLAGS

CPPFILES	:=	Client.cpp \
				ClientCGI.cpp \
				ClientGetPostDelete.cpp \
				ClientIn.cpp \
				ClientOut.cpp \
				ClientRequest.cpp \
				ClientUtils.cpp \
				HttpRequest.cpp \
				HttpResponse.cpp \
				signals.cpp \
				Server.cpp \
				WebServer.cpp \
				WebServerParseConfig.cpp \
				WebServerRequests.cpp \
				WebServerSocket.cpp \
				WebServerUtils.cpp \
				utils.cpp \

TFILES		:=	StressTester.cpp \
				StressTesterClient.cpp \
				StressTesterRequest.cpp \
				StressTesterResponse.cpp \
				StressTesterUtils.cpp \
				testerMain.cpp \
				testerUtils.cpp \

MAIN		:= main.cpp

SRC_DIR		:= src
T_DIR		:= webservTester
OBJ_DIR		:= obj

OBJECTS		= $(addprefix $(OBJ_DIR)/,$(notdir $(CPPFILES:%.cpp=%.o)))
M_OBJ		= $(addprefix $(OBJ_DIR)/,$(notdir $(MAIN:%.cpp=%.o)))
T_OBJ		= $(addprefix $(OBJ_DIR)/,$(notdir $(TFILES:%.cpp=%.o)))

vpath %.cpp $(shell find $(SRC_DIR) -type d)

all: $(NAME)

test: $(T_EXEC)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(NAME): $(OBJ_DIR) $(OBJECTS) $(M_OBJ)
	$(CC) $(CPPFLAGS) $(OBJECTS) $(M_OBJ) -o $(NAME)

$(T_EXEC): $(OBJ_DIR) $(OBJECTS) $(T_OBJ)
	$(CC) $(CPPFLAGS) $(OBJECTS) $(T_OBJ) -o $(T_EXEC)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) -c $(CPPFLAGS) $(HEADERS) -o $@ $^

$(T_DIR)/%.o : $(T_DIR)/%.cpp
	$(CC) -c $(CPPFLAGS) $(T_HEADERS) -o $@ $^

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)
	rm -f $(T_EXEC)

re:
	$(MAKE) -j1 fclean
	$(MAKE) all

retest:
	$(MAKE) -j1 fclean
	$(MAKE) test

debug: CPPFLAGS += -fsanitize=address
debug: re

.PHONY: all clean fclean re debug test retest