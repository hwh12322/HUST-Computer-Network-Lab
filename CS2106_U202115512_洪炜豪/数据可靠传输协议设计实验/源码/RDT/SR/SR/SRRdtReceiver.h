#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include "SRConfig.h"
#include <deque>
#include <fstream>
class SRRdtReceiver :public RdtReceiver
{
private:
	ofstream f;
	bool isInWin(int seq);
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���
	deque<Packet> rcvPkt;
	int maxSeq;
public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	
	void receive(const Packet &packet);	//���ձ��ģ�����NetworkService����
};

#endif

