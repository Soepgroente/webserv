NAME:= webserv
CPPFLAGS:= -Wall -Wextra -Werror -std=c++20
SRCS:= main.cpp \
      WebServer.cpp
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