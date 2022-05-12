# System-Programming-1
This is the first project in System-Programming. Through this project we learned about pipes, named pipes, signal handling, low-level I/O and process intercommunication.

## Installation / Build / Run
🔨 Download the files locally and run:
 ```
  $ make
  $ ./sniffer -p path
  ```
The path is the relative path of the directory you want to monitor about any file changes.

## Programm Functionality
🔩 The programm uses <ins>inotifywait()</ins> to monitor a given folder about new created files or moved_into files. When the programm gets notified about a new file the manager sends signal to a forked worker to process that file. The worker searches the file about any links that start with <ins>http://</ins>. After the process he makes a <filename>.out file with all the links he found and each link accompanied with the number it appears in the file. The manager communicates with each worker about the <filename> through a named pipe (aka FIFO). Each worker after he is done with his job he sends a STOP signal to himself so he can run again later and does not exit. The worker pool is also a FIFO queue implemented by me. The programm terminates with CTRL+C (SIGINT).

## Built with
  <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/1/18/C_Programming_Language.svg/380px-C_Programming_Language.svg.png" style="height: 100px; width:90px;"/>
