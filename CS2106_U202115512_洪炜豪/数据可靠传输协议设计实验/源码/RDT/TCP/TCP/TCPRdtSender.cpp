#include "stdafx.h"
#include "Global.h"
#include "TCPRdtSender.h"
#include "TCPConfig.h"

TCPRdtSender::TCPRdtSender() : nextSeqnum(0),waitingState(false),winSize(WIN_SIZE),maxSeq(1<<SEQ_BIT),base(0), invalidAck(0),lastACK(-1)
{
	f.open("D:\\destop\\Computer_Network_Lab\\lab2\\RDT\\TCP\\senderWindow.txt");
}


TCPRdtSender::~TCPRdtSender()
{
	f.close();
}



bool TCPRdtSender::getWaitingState() {
	return waitingState;
}




bool TCPRdtSender::send(const Message& message) {
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}
	Packet packetWaitingACK;
	packetWaitingACK.acknum = -1;
	packetWaitingACK.seqnum = this->nextSeqnum;
	packetWaitingACK.checksum = 0;
	memcpy(packetWaitingACK.payload, message.data, sizeof(message.data));
	packetWaitingACK.checksum = pUtils->calculateCheckSum(packetWaitingACK);
	this->sndpkt.push_back(packetWaitingACK);
	pUtils->printPacket("���ͷ����ͱ���", this->sndpkt.back());
	if(base==nextSeqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->sndpkt.back());								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	if (this->sndpkt.size() >= this->winSize) {
		this->waitingState = true;																	//����ȴ�״̬
	}
	this->nextSeqnum = (this->nextSeqnum + 1) % this->maxSeq;
	return true;
}

void TCPRdtSender::receive(const Packet& ackPkt) {
	if (!this->sndpkt.empty()) {
		//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//���У�����ȷ
		if (checkSum == ackPkt.checksum) {
			pns->stopTimer(SENDER, 0);		//�رն�ʱ��
			this->base = (ackPkt.acknum + 1) % this->maxSeq;
			if(base != nextSeqnum){
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//�������ͷ���ʱ��
			}
			//�����ش�
			if (ackPkt.acknum == this->lastACK) {
				pUtils->printPacket("���ͷ��յ�����ȷ��", ackPkt);
				invalidAck++;
			}
			else {
				pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
				this->lastACK = ackPkt.acknum;
				invalidAck = 0;
			}
			if (this->invalidAck == 3) {
				pUtils->printPacket("���ܵ���������ȷ�ϣ�ִ�п����ش����ط��ϴη��͵ı���", this->sndpkt.front());
				pns->sendToNetworkLayer(RECEIVER, this->sndpkt.front());			//���·������ݰ�
				this->invalidAck = 0;
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//�������ͷ���ʱ��
			}
			//����Ƿ���Ҫ��������
			while (!sndpkt.empty() && sndpkt.front().seqnum != base) {
				cout << "����ǰ�����б������Ϊ��";
				f << "����ǰ�����б������Ϊ��";
				for (auto p : sndpkt) {
					f << p.seqnum << " ";
					cout << p.seqnum << " ";
				}
				cout << endl;
				f << endl;
				int newbase = (sndpkt.front().seqnum + 1) % maxSeq;
				this->sndpkt.pop_front();
				cout << "�����󴰿��б������Ϊ��";
				f << "�����󴰿��б������Ϊ��";
				for (auto p : sndpkt) {
					f << p.seqnum << " ";
					cout << p.seqnum << " ";
				}
				cout << "\n" << endl;
				f << "\n" << endl;
			}
			if (this->sndpkt.size() < this->winSize) {
				this->waitingState = false;																	//����ȴ�״̬
			}
		}
	}
}

void TCPRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->sndpkt.front());
	pns->sendToNetworkLayer(RECEIVER, this->sndpkt.front());			//���·������ݰ�

}