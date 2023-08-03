## Check-2

- [check2.pdf](https://cs144.github.io/assignments/check2.pdf)
- code branches [wrap](../src/wrapping_integers.cc), [tcp_receiver](../src/tcp_receiver.cc)

### Translate between 64-bit indexes and 32-bit seqnos

- In the TCP headers, each byte's index in the stream is represented with a 32-bit seqno.
- TCP sequence numbers start at a random value.
- The logical beginning and ending each occupy one sequence number.

|    element     |    SYN    |     c     |  a   |  t   | FIN  |
| :------------: | :-------: | :-------: | :--: | :--: | :--: |
|     seqno      | 2^32^ - 2 | 2^32^ - 1 |  0   |  1   |  2   |
| absolute seqno |     0     |     1     |  2   |  3   |  4   |
|  stream index  |           |     0     |  1   |  2   |      |

- **Target**: Implement the functions below:

  ```c++
  static Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point)
    //Convert absolute seqno(n) to seqno. 
    //zero_point is ISN
  uint64 t unwrap( Wrap32 zero point, uint64 t checkpoint ) const
    //find the nearest absolute sequence number to the checkpoint	
  ```

- `wrap`: simple return `zeropoint + n`, we should notice the data shouldn't overwhelm `uint32_t` so do something more.
- `unwrap`: find the nearest, first check the offset between now and checkpoint. If the offset is just smaller than checkpoint, that means it is the nearest. If not, add a mutiple of 2^32^.

And then you can pass the wrap and unwrap test.

### TCP receiver

- **Target**: Implement the functions below:

  ```c++
  void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
    //receive the message from TCPSender and write it to reassembler
    
  TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
   	//Generate a message send to TCPReceiver which contains the window_size and ackno direct to the next expect number.
  ```

- Create a private variable in class `Receiver` which indicates isn.

  ```c++
  bool isn_set { false };
    Wrap32 isn { 0 };
  ```

- `receive`: Wait until the isn comes, set the `isn_set`, record the isn. `inbound_stream.bytes_pushed` represent the byte has been pushed to the bytestream, and it plus one is the expect number `checkpoint`. And then we get the absolute seqno by `unwrap`. Notice that FIN and ISN take a place in absolute seqno, the corresponding stream index may be `absolute_seq - 1 + SYN`. And then, `insert` the index with the message.

- `send`: If isn has not come yet, return the optional with window_size, if the isn has come, return with an `ackno` indicating the next expect number.

When everything is done, we can pass all the test.

### Problems

​	I used to run the code in docker for mac, but the speed is too slow to pass the test, compile the code in a virtual machine rather than docker!