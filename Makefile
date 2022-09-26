SRCNAME	= \
	main.cpp\
	Webserver.cpp\
	KqueueMonitoredFdInfo.cpp\
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
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -I ./includes/

CPP = c++

NAME = webserv

${NAME} : $(SRCS)
	${CPP} ${CPPFLAGS} ${SRCS} -o ${NAME}

fclean:
	rm -rf ./YoupiBanane/test_output/file_should_exist_after
	rm -rf ./YoupiBanane/test_output/multiple_same
	rm -rf webserv.dSYM
	rm -rf ${NAME}

re:
	make fclean
	make all

all: ${NAME}

.PHONY: fclean re test
