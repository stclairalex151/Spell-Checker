# Alex St.Clair, tug89508, TUID: 915495534
# 3207 Project3: Networked Spell Checker
# Lab Section 001

server: 3207project3.c open_listenfd.c server.h
	gcc -o server 3207project3.c open_listenfd.c -lpthread
