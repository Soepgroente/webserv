NAME := webserv
CXX := clang++
CPPFLAGS := -g
SRCDIR := src
INCDIR := include
OBJDIR := obj

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)


all: $(NAME)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@$(CXX) $(CPPFLAGS) -I$(INCDIR) -c $< -o $@

$(NAME): $(OBJS)
	@$(CXX) $(CPPFLAGS) $(OBJS) -o $(NAME)

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

run: $(NAME)
	@./$^
