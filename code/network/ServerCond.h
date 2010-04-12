//ServerCond.h

int MAX_SERVER_CONDS = 100;

struct CondOwner {
  int machineID;
  int mailboxNum;
  int lockID;
};

struct ClientAddr {
  int machineID;
  int mailboxNum;
};

class ServerCond {

 private:
  Lock *CondWaitQueueLock;
  ClientAddr condWaitQueue[1000];
  int condWaitQueueSize;

 public:
  //CondOwner owner;
  //ClientAddr condWaitQueue[];
  //int condWaitQueueSize;
  int myCondID;
  int condLockID;

  ServerCond();
  ServerCond(int theCondID);

  void Wait(CondOwner theOwner); //CondOwner contains the lockID
  ClientAddr Signal(CondOwner theOwner); //returns info about the caller to wake up
  
  ClientAddr* Broadcast();

  void SendMsg(CondOwner theOwner, int msg);

  

};
