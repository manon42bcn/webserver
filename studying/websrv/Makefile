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

SRCS_FILES		=	main.cpp
HEADER_DIR		=	inc
HEADER			=	webserver.hpp
SRCS_DIR		=	srcs
OBJS_DIR		=	obj
HEADERS			=	$(addprefix $(HEADER_DIR)/,$(HEADER))
SRCS 			=	$(addprefix $(SRCS_DIR)/,$(SRCS_FILES))
OBJS			=	$(addprefix $(OBJS_DIR)/,$(SRCS_FILES:.cpp=.o))
CC				=	c++
RM				=	rm -rf
CFLAGS			=	-std=c++98 -Wall -Wextra -Werror
NAME			=	webserver

.PHONY: all
all: $(OBJS_DIR) $(NAME)

$(NAME): $(OBJS)
	@echo "Make: Link"
	$(CC) $(OBJS) -I$(HEADER_DIR) $(CFLAGS) -lstdc++ -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) -I$(HEADER_DIR) -c $< -o $@

$(OBJS_DIR):
	@echo "Make: Create objects dir."
	-mkdir $(OBJS_DIR)

.PHONY: clean
clean:
	$(RM) $(OBJS)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)
	$(RM) $(OBJS_DIR)

.PHONY: re
re:	fclean $(OBJS_DIR) $(NAME)
