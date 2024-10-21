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

SERVER_DIR		=	websrv/

.PHONY: all
all:
	@docker compose -f $(SERVER_DIR)docker-compose.yml up --build

.PHONY: clean
clean:
	docker compose -f $(SERVER_DIR)docker-compose.yml down --rmi all -v
	docker system prune -a -f;
	docker network prune -f