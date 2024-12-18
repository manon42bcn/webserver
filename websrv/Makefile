# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vaguilar <vaguilar@student.42barcelona.    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/05/05 22:26:19 by mporras-          #+#    #+#              #
#    Updated: 2024/11/11 22:37:56 by vaguilar         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS_FILES		=	general_helpers.cpp \
					WebServerResponseHandler.cpp \
					HttpCGIHandler.cpp \
					HttpRangeHandler.cpp \
					HttpMultipartHandler.cpp \
					HttpAutoIndex.cpp \
					main.cpp \
					Logger.cpp \
					SocketHandler.cpp \
					ClientData.cpp \
					HttpRequestHandler.cpp \
					HttpResponseHandler.cpp \
					ServerManager.cpp \
					parse/parse.cpp \
					parse/verifications.cpp \
					parse/utils.cpp \
					parse/print.cpp \
					parse/split.cpp \
					parse/parse_server.cpp \
					parse/parse_location.cpp
HEADER_DIR		=	inc
HEADER			=	webserver.hpp \
					ws_general_defines.hpp \
					ws_structs.hpp \
					ws_permissions_bitwise.hpp \
					WebserverCache.hpp \
					WebServerResponseHandler.hpp \
					HttpCGIHandler.hpp \
					HttpRangeHandler.hpp \
					HttpMultipartHandler.hpp \
					HttpAutoIndex.hpp \
					http_enum_codes.hpp \
					Logger.hpp \
					SocketHandler.hpp \
					ClientData.hpp \
					HttpRequestHandler.hpp \
					HttpResponseHandler.hpp \
					WebserverException.hpp \
					ServerManager.hpp
SRCS_DIR		=	srcs
OBJS_DIR		=	obj
HEADERS			=	$(wildcard $(HEADER_DIR)/*.hpp)
SRCS 			=	$(addprefix $(SRCS_DIR)/,$(SRCS_FILES))
OBJS			=	$(addprefix $(OBJS_DIR)/,$(SRCS_FILES:.cpp=.o))
CC				=	c++
RM				= 	rm -rf
CFLAGS			=	-std=c++98 -pedantic -Wall -Wextra -Werror -g
NAME			=	webserver
WEBSERVER_PATH 	:= $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
OS := $(shell uname)

export WEBSERVER_PATH

all: $(OBJS_DIR) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -I$(HEADER_DIR) $(CFLAGS) -lstdc++ -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(HEADER_DIR) -c $< -o $@

$(OBJS_DIR):
	-mkdir $(OBJS_DIR)

run: re
	@./webserver configs/main.conf

tests: re
	@./webserver configs/tests.conf

local: all
	@./webserver configs/servers.conf

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)
	$(RM) $(OBJS_DIR)

re:	fclean $(OBJS_DIR) $(NAME)

.PHONY: all clean fclean re
