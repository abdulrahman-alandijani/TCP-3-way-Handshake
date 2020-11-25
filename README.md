Program Discription: TCP segment simulation; the program stimulates the process
of transmitting a message in a 3 way handshake TCP segment on a client-server based network 
communication program.

To compile:
       server:
              make server
       client: 
              make client
it will create binary files for both codes

To Execute:
       server:
              ./server (port_numer)
       client:
              ./client (port_number) (input_file)
port number is needed in order to run 

To clean:
       make clean
it will remove all binary files

To Test:
       compile code and run executables with port number at command line.
       Example:
              server: 
                     make server
                     ./server 1234
              client:
                     make client
                     ./client 1234
              when done:
                     make clean
