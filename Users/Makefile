NAME = Server
G++ = g++
VERSAO = -std=c++20
FLAGS = -lboost_system -lpthread -lpqxx -lpq -IIncludes/third_party/jwt-cpp/include -lcrypto

SRCS = main.cpp User_utils.cpp Auth_utils.cpp

all: $(NAME)

$(NAME):
		$(G++) $(VERSAO) $(SRCS) $(FLAGS) -o $(NAME)

clean:
	rm -rf $(NAME)

fclean:
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
