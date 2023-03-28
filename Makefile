NAME=ircserv
CC=c++
CXX=c++
CXXFLAGS=-Wall -Wextra -Werror -std=c++98 -pedantic -g
SRC=ircserv.cpp
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