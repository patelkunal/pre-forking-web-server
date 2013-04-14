pre-forking-web-server
======================

Pre-forking self-regulated web server implentation in C. For more details refer http://httpd.apache.org/docs/2.0/mod/prefork.html

Specifications:

  a. Server should be self-regulated as mentioned in this webpage. Server should regulate according to the parameters specified: StartServers, MinSpareServers, MaxSpareServers, and MaxClients. These parameters are specified as command line   arguments to the server.
  
  b. Server  should  recycle  the  child  once  it  finishes  handling MaxRequestsPerChildnumber of connections. This parameter is also taken as command line parameter. 
  
  c. Child waits over listening socket. Whenever it accepts a connection, it prints its pid, client's ip and port. Child receives the HTTP request and sends a dummy reply.
  
  d. Whenever a parent makes a change to the process-pool, it prints the number of children in process pool, number of clients being handled, action being taken, post-action status.
  
  e. Use UNIX Domain sockets for any parent-child communication.
  
  f. By sending Ctrl-c signal, control process prints number of children currently active, and for each child how many clients it has handled.
  
  g. zombie processes should be handled.
  
  h. Server will be tested using a web server testing tool such as the one mentioned here: 
	   http://httpd.apache.org/docs/2.0/programs/ab.html
