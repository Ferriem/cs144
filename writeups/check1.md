v## Check-1

- [check1.pdf](https://cs144.github.io/assignments/check1.pdf)
- code branches [reassember](../src/reassember.cc)

### Putting substrings in sequence

- Target: Implement `insert` and `bytes_pending` in *src/reassembler.cc*

​	The network may reorder, drop, resend the datagrams, we should reassemble the segments into th contiguous stream of bytes.

​	Considered the uncontigous stream, I apply `std::map<uint64_t, std::string>` to reserve the data. And some necessary variable to reflect the state.

```c++
//in reassembler.hh
class Reassembler
{
private:
  std::map<uint64_t, std::string> data_ {};
  uint64_t unreassembled_bytes_ {0}; //bytes which remain in data_ and didn't write to output
  uint64_t bytes_now_ {0}; //the begin bytes which need to be written to output
  bool closed_ {false};
}
```

​	The function `bytes_pending` just simply return the variable `unreassembled_bytes_`.

​	Then to design the `insert` function

- Do some necessary preparations.

  ```c++
  	if(is_last_substring) closed_ = true;
  
  
    if(first_index + data.size() <= bytes_now_ || first_index >= bytes_now_ + output.available_capacity() || data.empty() || output.available_capacity() == 0){
      if(closed_ && bytes_pending() == 0){
        output.close();
      }
      return ;
    }
  ```

- Handle the coming data. There are some situations.

  - `first_index < bytes_now_`, overlap occurs, we should cut off the overlapped section in the head and don't make the datagram overwhelm the output.
  - `first_index >= bytes_now_`, just notice the overwhelming, no overlap in the front.

- Handle the overlap with other buffered datagram(didn't match to the `bytes_now_` then buffered in `map`). (in a loop) Situations:

  - The `lower_bound` higher than the datagram(no overlap), just do nothing and go next.
  - Notice the `lower_bound >= first_index`, we now handle the overlap situations.
    - The buffered datagram is fully contained in the datagram now, erase the buffered datagram in map and modify `reassembled_bytes_`.
    - Else, cut off the overlap in datagram now.

- Now we have dealt with the overlap after the datagram, what about someone ahead of it? (Needed:`first_index > bytes_now_ `)

  - If the ahead datagram didn't reach to the datagram now, do nothing, go ahead.
  - Else, overlap occurs.
    - If the datagram now is fully contained in the buffered datagram, clear the data.
    - Else, cut off the datagram, modify `first_index`.

- Insert the datagram.

- Handle the output. Just notice that sometimes output may occur to capacity, we should record the overwhelm and buffer it for next insert.