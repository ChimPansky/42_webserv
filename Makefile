all: up

up:
	docker compose up --build

down:
	docker compose down

.PHONY: all up down
