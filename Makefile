NAME=ircserv
CC=$(CXX)
CXXFLAGS=-Wall -Wextra -Werror -std=c++98 -pedantic -g -MMD
SRC=ircserv.cpp server.cpp client.cpp
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