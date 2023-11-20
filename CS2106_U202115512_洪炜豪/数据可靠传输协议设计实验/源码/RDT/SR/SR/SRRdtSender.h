#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include "SRConfig.h"
#include <deque>
#include <fstream>
class SRRdtSender :public RdtSender
{
private:
	ofstream f;
	bool waitingState;				// �Ƿ��ڵȴ�Ack��״̬
	deque<Packet> sndpkt; //������
	int winSize; //���ڴ�С
	int maxSeq;
	int nextSeqnum;
public:

	bool getWaitingState();
	bool send(const Message& message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet& ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};

#endif

