# client-server

**** Welcome to the Number Guessing Game ****

You will be using Linux to compile and run this program.

To begin: you will need to open two or more windows of your Linux interface.
Open the directory where the files are located. client.cpp and server.cpp

To compile: 
g++ client.cpp -o client 

g++ server.cpp -o server -lpthread

To run:
You will need to run server.cpp first

./server [port number] (enter your desired port number. ex: 8080)

./client cs1.seattleu.edu [port number] (enter same port number as server)

The server will now run. Every time a client connects you will see a message that says
Thread Created. Once a client connects the client's guess they need to answer will be
displayed. Once the client guesses. Their guess will be displayed.

On the client side. You will be prompted to enter you name. Please only enter (one) name,
first name only as it will only accept one name. 

You will then be prompted to guess what the answer is (0-9999). Please only enter 
integers. If you input a char it will prompt you to guess again. If you just press ENTER
the program will move to a new line and wait for your input.  If correct you will see a 
leader board of the top 3 scores. If incorrect you will be returned the closeness of all
the integers added up. Keep guessing until your answer is correct. 

*** Thank you ***
