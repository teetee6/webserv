SRCNAME	= \
	main.cpp\
	Webserver.cpp\
	FdType.cpp\
	Server.cpp\
	Location.cpp\
	Kqueue.cpp\
	Connection.cpp\
	Request.cpp\
	Cgi.cpp\
	Response.cpp\
	ConfigParser.cpp\


SRCDIR = ./srcs/

SRCS = ${addprefix ${SRCDIR}, ${SRCNAME}}

INC = -I ./includes/

NAME = webserv

CC = c++

ifdef WITH_BONUS
	CFLAGS = -Wall -Wextra -Werror -std=c++98 -D BONUS ${INC}
else
	CFLAGS = -Wall -Wextra -Werror -std=c++98 ${INC}
endif

DCFLAGS = -g3 -fsanitize=address # -D BONUS

${NAME} : $(SRCS)
	${CC} ${CFLAGS} ${SRCS} -o ${NAME}

test:
	${CC} ${DCFLAGS} ${SRCS} ${INC} -o ${NAME}
	rm -rf ./tests/put_test/file_should_exist_after
	./webserv configs/test.conf

test_hyeonski:
	${CC} ${DCFLAGS} ${SRCS} ${INC} -o ${NAME}
	rm -rf ./tests/put_test/file_should_exist_after
	rm -rf .res_*
	./webserv configs/test_hyeonski.conf

bonus:
	make WITH_BONUS=1 all

fclean:
	rm -rf ./tests/put_test/file_should_exist_after
	rm -rf ./tests/put_test/multiple_same
	rm -rf webserv.dSYM
	rm -rf ${NAME}

re:
	make fclean
	make all

all: ${NAME}

.PHONY: fclean re test
