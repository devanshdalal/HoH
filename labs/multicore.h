#pragma once

#include <atomic>
//
// INVARIANT: w_deleted_count <= w_deleting_count <= w_cached_read_count <= shared_read_count <= r_reading_count <= r_cached_write_count <= shared_write_count <= w_writing_count <= w_deleted_count + MAX_SIZE
//
// INVARIANT:      w_writing_count      - w_deleted_count     <= MAX_SIZE
// =========>      w_writing_count      - w_cached_read_count <= MAX_SIZE
// =========>      shared_write_count   - w_cached_read_count <= MAX_SIZE
// =========>      shared_write_count   - shared_read_count   <= MAX_SIZE
//
//
// INVARIANT: 0 <= r_cached_write_count - r_reading_count
// =========> 0 <= r_cached_write_count - shared_read_count
// =========> 0 <= shared_write_count   - shared_read_count
//
//
// THEOREM: =========> 0 <= shared_write_count   - shared_read_count   <= MAX_SIZE
//





//
//
// Channel/Queue:
//
// Shared between Producer and Consumer
//
struct channel_t{
  public:

    //Atomic variables for shared state
    std::atomic<uint32_t> r,w;

  public:

    //
    // Intialize
    //
    channel_t(){
      r=w=0;
      // insert your code here
    }
};

//
// Producer's (not shared)
//
struct writeport_t{
public:
    //insert your code here
    uint32_t w,wi;                                // write head and writing head
    uint32_t d,di;                                // delete head and deleting head
    uint32_t r;                                   // read head
    uint32_t qsize;                               // size of queue

public:

  //
  // Intialize
  //
  writeport_t(size_t tsize)
  {
    //insert code here
    d=di=w=wi=0;
    qsize = tsize;
  }

public:

  //
  // no of entries available to write
  //
  // helper function for write_canreserve
  //
  size_t write_reservesize(){
    // insert your code here
    return wi-w;
    // return 0;
  }

  //
  // Can write 'n' entries?
  //
  bool write_canreserve(size_t n){
    // insert your code here
    return n<=qsize-w+d;
  }

  //
  // Reserve 'n' entries for write
  //
  size_t write_reserve(size_t n){
    // insert your code here
    wi=w+n;
    return w;
  }

  //
  // Commit
  //
  // Read/Write shared memory data structure
  //
  void write_release(channel_t& ch){
    ch.w=w=wi;
    // insert your code here
  }


public:

  //
  //
  // Read/Write shared memory data structure
  //
  void read_acquire(channel_t& ch){
    //insert your code here
    r=ch.r;
  }

  //
  // No of entires available to delete
  //
  size_t delete_reservesize(){
    return 0;
  }

  //
  // Can delete 'n' entires?
  //
  bool delete_canreserve(size_t n){
    return n<=r-d;
  }

  //
  // Reserve 'n' entires for deletion
  //
  size_t delete_reserve(size_t n){
    di=d+n;
    return d;
  }


  //
  // Update the state, if any.
  //
  void delete_release(){
    d=di;
  }


};


//
// Consumer's (not shared)
//
//
struct readport_t{
public:

  //insert your code here
  uint32_t w;                                       // write head
  uint32_t r,ri;                                    // read head and reading head
  uint32_t qsize;                                   // size of queue

public:
  //
  // Initialize
  //
  readport_t(size_t tsize)
  {
    r=ri=0;
    qsize=tsize;
  }

  public:
  //
  // Read/Write shared memory data structure
  //
  void write_acquire(channel_t& ch){
    w=ch.w;
  }

  //
  // no of entries available to read
  //
  size_t read_reservesize(){
    return ri-r;
  }

  //
  // Can Read 'n' entires?
  //
  bool read_canreserve(size_t n){
    return n<=w-r;
  }

  //
  // Reserve 'n' entires to be read
  //
  size_t read_reserve(size_t n){
    ri=r+n;
    return r;
  }

  //
  // Read/write shared memory data structure
  //
  void read_release(channel_t& ch){
    ch.r=r=ri;
  }

};


