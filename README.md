Description:
    A chat app for Linux that uses sockets in UNIX Domain and TCP to communicate. 
    There is a main server file that opens the port and the client files can connect 
    to the server.
    
    Once connected, each client can either participate in a group chat or message  
    any connected user personally.
    
    Every time a user connects, he/she is assigned a unique ID by the server which 
    can be used by others to message him personally.

    The System uses threads, the server creates a new thread every time a new
    user is created and the thread continuously listens for user input and act accordingly.
    The client creates a new thread which continuously listens for inputs from the server
    while the main thread takes input from user and sends it to the server.

    
    '#h'- for help, which brings the help menu
    '#a'- shows all active users
    '#exit' - Exits the chat
    '#c'- Shows who you are messaging currently
    '#i'- Shows Your unique ID assigned by the server
    '#any_correct_client_ID'- Switches you to message that particular client(0 for the group chat)

How to setup?
    Compiling:
        Run the makefile using the make command, execute the 'server' executable by
        $./server
        now In a different terminal, run the 'client' executable by
        $./client user_name
        The user name can be left blank and the server provides you with a name

Testing:
    You can run the client scipts in 2 or more different terminals.
    
    Testing the Group Chat:
        -send any message and test if the message was recieved in all the other terminals.
        -Try switching to group chat by sending '#0', should recieve 
            'Switched to Group chat!'
        -when in group chat, send '#c' to get the current recepient and you 
            should get 'You are messaging: 0' where 0 is the ID for group.
    
    Testing Personal chat:
        -Send '#a' to get the list of all active users
        -Select any active user by sending '#user_id' and you should get
            'Sending message to client with ID user_id'
        -Sending '#c' now should give 'You are messaging: user_id'
        -If the user_id you send is not present, you should receive an 
            error message 'ERROR!, either the ID is Invalid or Disconnected, Switching to group chat!'
    
    Testing help menu:
        -Sending '#h' anywhere should bring the help menu
    

    Testing exit:
        -Send '#exit' to exit the chat with a message, "Goodbye, come back again!"
        -Send '^c' to exit with the above message

Handled Errors:
    -If the client does not provide a name, the server provides the name and the program does not crash
    -Two clients can have the same name and the program won't crash or confuse 
    -You cannot send yourself a message, if you select it, the program does not crash
    -You cannot select a non-existent user a message, if you do, the program does not crash
    -The User you are talking to right now can leave and you will be shifted to the 
        group chat and the program does not crash
    -Taken care of race conditions if 2 users leave together, the thread waits the other
        thread to unlock and then removes the new user
    -Handled SIGINT or '^c' which won't crash the program and exits the user normally
    
