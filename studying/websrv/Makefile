# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mporras <mporras@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/05/05 22:26:19 by mporras-          #+#    #+#              #
#    Updated: 2022/10/24 12:28:22 by mporras          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS_FILES		=	webserver.cpp \
					http_codes_helper.cpp \
					main.cpp \
					Logger.cpp \
					SocketHandler.cpp \
					ClientData.cpp \
					HttpRequestHandler.cpp \
					HttpResponseHandler.cpp \
					ServerManager.cpp
HEADER_DIR		=	inc
HEADER			=	webserver.hpp \
					http_enum_codes.hpp \
					Logger.hpp \
					SocketHandler.hpp \
					ClientData.hpp \
					HttpRequestHandler.hpp \
					HttpResponseHandler.hpp \
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

export WEBSERVER_PATH


all: $(OBJS_DIR) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -I$(HEADER_DIR) $(CFLAGS) -lstdc++ -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -I$(HEADER_DIR) -c $< -o $@

$(OBJS_DIR):
	-mkdir $(OBJS_DIR)

run: re
	@./webserver

local: all
	@./webserver

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)
	$(RM) $(OBJS_DIR)

re:	fclean $(OBJS_DIR) $(NAME)

.PHONY: all clean fclean re