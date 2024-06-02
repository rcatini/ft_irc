NAME=ircserv
CC=$(CXX)
CXXFLAGS=-Wall -Wextra -Werror -Wshadow -Wconversion -Wuninitialized -Wunused -g -std=c++98 -MMD
SRC=ircserv.cpp server.cpp user.cpp
OBJ=$(SRC:.cpp=.o)
DEPENDS=$(SRC:.cpp=.d)

all: $(NAME)

$(NAME): $(OBJ)

clean:
	rm -f $(OBJ) $(DEPENDS)

fclean: clean
	rm -f $(NAME)

re: fclean
	$(MAKE) all
