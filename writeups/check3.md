## Check-3

[check3.pdf](https://cs144.github.io/assignments/check3.pdf)

- **Target**: Implement TCPSender

  - Keep track of the receiver's window.
  - Create new TCP segments, send them. The sender should keep sending the segments until either the window is full or the outbound Bytestream has nothing to send.
  - Keep track of which segments have been but not yet acknowledged by the receiver.
  - Re-send outstandinf segments if enough time passeed since they were sent.

- Implement

  - First do some preparations for status.

    ```c++
    class TCPSender
    {
      Wrap32 isn_;
      uint64_t initial_RTO_ms_;
      uint64_t RTO_ms_ {0}; //The retransmission timeout now
      uint64_t timer {0}; //The time passed since last tick
      bool set_syn {false}; 
      bool set_fin {false};
      uint64_t window_size {1}; //The window size of the receiver
      uint64_t out_seqno {0}; //The sequence number of the next byte to be sent
      uint64_t next_abs_seqno {0}; //The next absolute sequence number to be sent
      uint64_t consecutive_retransmissions_ {0}; //The number of consecutive retransmissions
      std::queue<TCPSenderMessage> message_queue {}; //The queue of messages to be sent
      std::map<uint64_t, TCPSenderMessage> sent_messages {}; //The map of sent messages, used to check if a message is acknowledged
      Wrap32 get_next_seqno() const {return isn_ + next_abs_seqno;}
      uint64_t get_next_abs_seqno() const {return next_abs_seqno;}
    }
    ```

  - `void push(Reader& outbound_stream)` 

    - The `window size` must not be zero, firstly modify the `window size`.
    - Then push the segments, remember that push until the window size run out or the outbound has nothing to send.
    - Set the first segment with `SYN`, load the `payload` from `outbound` Bytestream.
    - Set FIN if there is space for window size.
    - If the ackno map is empty, start the timer.
    - Push the segment to the queue and wait for sending, insert it to the ackno map waiting for ack. Modify seqno and next_abs_seqno.

  - `std::optional<TCPSenderMessage> maybe_send()`

    - Generatee a `TCPSenderMessage`.
    - If there is segment in the queue, extract it, and return it, remember to remove it from the queue. Else return `nullopt`.

  - `void receive( const TCPReceiverMessage& msg )`

    - The msg includes ackno and window_size.
    - First check the ackno's effectiveness, then use it to generate a recv_abs_seqno represent the ackno number, every segment buffered in the map whose seqno plus the datasize is smaller than ackno should be remove from the map, since they are acked.
    - Remember to modify the seqno since they are removed from the map.
    - And then, after the remove, if the map is totally emptied, restart the timer. Clear the retransmissions, modify the window_size.

  - `void tick( const size_t ms_since_last_tick )`

    - Update the timer by plus the tick.
    - If time has passed and there are something haven't acked, double the time, resend the segment(push it to queue to send). Restart the timer.