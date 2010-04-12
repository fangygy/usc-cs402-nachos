//ServerLock.h

int MAX_SERVER_LOCKS = 100; //the max number of server locks

struct LockOwner {
  int machineID;
  int mailboxNum;
};

enum LockState { BUSY, FREE };

class ServerLock {

 private:
  //private stuff here
  Lock *WaitQueueLock;

 public:

  LockOwner owner;
  LockOwner waitQueue[1000];
  LockState state;
  int waitQueueSize;
  int myLockID;
  bool toBeDestroyed;

  ServerLock();
  ServerLock(int theLockID);
  
  void Acquire(LockOwner theOwner);
  void Release(LockOwner theOwner);
  void Destroy();

  void SendMsg(LockOwner theOwner, int msg);
  
};
