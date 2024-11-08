# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: akasiota <akasiota@student.codam.nl>         +#+                      #
#                                                    +#+                       #
#    Created: 2024/09/24 15:07:50 by akasiota      #+#    #+#                  #
#    Updated: 2024/11/08 14:26:12 by akasiota      ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

NAME:= webserv
CPPFLAGS:= -Wall -Wextra -Werror -std=c++20
SRCS:= main.cpp \
      WebServer.cpp \
	  WebServerGeters.cpp \
	  WebServerClientStuff.cpp \
	  WebServerHandleRequests.cpp \
	  WebServerHandleErrors.cpp
OBJDIR:= objs
OBJS:= $(SRCS:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(OBJDIR):
	mkdir objs

$(OBJDIR)/%.o: %.cpp
	c++ $(CPPFLAGS) -c $^ -o $@

$(NAME): $(OBJDIR) $(OBJS)
	c++ $(CPPFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re