## Check-0

- [check0.pdf](https://cs144.github.io/assignments/check0.pdf)

### Writing `webget`

​	Edit the `get_URL` in *apps/webget.cc*. Read `util/file_descriptor.hh` and `util/socket.hh`. Some useful functions are mentioned below.

```c++
class Socket : public FileDescriptor
{
  Address get_address( const std::string& name_of_function,
                         const std::function<int( int, sockaddr*, socklen_t* )>& function ) const;
  void connect( const Address& address );
}

class FileDescriptor
{
  size_t write( std::string_view buffer );
}
```

```c++
void get_URL( const string& host, const string& path )
{
  cerr << "Function called: get_URL(" << host << ", " << path << ")\n";
  TCPSocket sock;
  sock.connect(Address(host, "http"));
  sock.write("GET " + path + " HTTP/1.1\r\n");
  sock.write("Host: " + host + "\r\n");
  sock.write("Connection: close\r\n");
  sock.write("\r\n");

  while(!sock.eof()){
    string ans;
    sock.read(ans);
    cout << ans;
  }
  sock.close();
}
```

```sh
.../minnow > cmake--build build
.../minnow/build > ./apps/webget cs144.keithw.org /hello
Function called: get_URL(cs144.keithw.org, /hello)
HTTP/1.1 200 OK
Connection: close
Content-Length: 14
Accept-Ranges: bytes
Content-Type: text/plain
Date: ...
Etag: ...
Last-Modified: Thu, 13 Dec 2018 15:45:29 GMT
Server: Apache

Hello, CS144!
.../minnow > cmake --build build --target check_webget
...
100% tests passed, 0 tests failed out of 2
```

### An in-memory reliable byte stream

Implement some function for `ByteStream` in *src/byte_stream.cc* and *src_stream.hh*.

Add additional state in *stream.hh*

```c++
class ByteStream
{
protected:
  uint64_t capacity_ {};
  
  // Please add any additional state to the ByteStream here, and not to the Writer and Reader interfaces.
  std::string buf {};
  uint64_t bytes_written_ {};
  uint64_t bytes_read_ {};
  bool closed_ {};
  bool error_ {};
}
```

And then complete the functions in *bytestream.cc*.

```sh
.../minnow > cmake --build build --target check0
...
100% tests passed, 0 tests failed out of 10
```





