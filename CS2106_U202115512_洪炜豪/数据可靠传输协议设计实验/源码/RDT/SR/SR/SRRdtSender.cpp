#include "stdafx.h"
#include "Global.h"
#include "SRRdtSender.h"
#include "SRConfig.h"
SRRdtSender::SRRdtSender(): nextSeqnum(0), waitingState(false), winSize(WIN_SIZ), maxSeq(1 << SEQ_BIT)
{
	f.open("D:\\destop\\Computer_Network_Lab\\lab2\\RDT\\SR\\senderWindow.txt");
}


SRRdtSender::~SRRdtSender()
{
	f.close();
}



bool SRRdtSender::getWaitingState() {
	return waitingState;
}

bool SRRdtSender::send(const Message &message) {
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}
	Packet packetWaitingACK;
	packetWaitingACK.acknum = 0;//0Ϊδ���յ��ظ�
	packetWaitingACK.seqnum = this->nextSeqnum;
	packetWaitingACK.checksum = 0;
	memcpy(packetWaitingACK.payload, message.data, sizeof(message.data));
	packetWaitingACK.checksum = pUtils->calculateCheckSum(packetWaitingACK);
	this->sndpkt.push_back(packetWaitingACK);
	pUtils->printPacket("���ͷ����ͱ���", this->sndpkt.back());
	pns->startTimer(SENDER, Configuration::TIME_OUT, sndpkt.back().seqnum);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->sndpkt.back());								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	if (this->sndpkt.size() >= this->winSize) {
		this->waitingState = true;																	//����ȴ�״̬
	}
	this->nextSeqnum = (this->nextSeqnum + 1) % this->maxSeq;
	return true;
}

void SRRdtSender::receive(const Packet &ackPkt) {
	if (!this->sndpkt.empty()) {
		//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//���У�����ȷ
		if (checkSum == ackPkt.checksum) {
			//�����������ݰ��ڶ����е��±�
			int temp = (ackPkt.acknum + this->maxSeq - this->sndpkt.front().seqnum) % this->maxSeq;
			if (temp >= 0 && temp < this->sndpkt.size()) {
				this->sndpkt[temp].acknum = 1;//���Ϊ�Ѷ�
				pns->stopTimer(SENDER, ackPkt.acknum);		//�رն�ʱ��
				pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
				while (!this->sndpkt.empty() && this->sndpkt.front().acknum) {
					cout << "����ǰ���ͷ������б������Ϊ��";
					f << "����ǰ���ͷ������б������Ϊ��";
					for (auto p : sndpkt) {
						
						f << p.seqnum << " ";
						cout << p.seqnum << " ";
					}
					cout << endl;
					f << endl;
					this->sndpkt.pop_front();
					cout << "�������ͷ������б������Ϊ��";
					f << "�������ͷ������б������Ϊ��";
					for (auto p : sndpkt) {
						f << p.seqnum << " ";
						cout << p.seqnum << " ";
					}
					cout << "\n" << endl;
					f << "\n" << endl;
				}
				if (this->sndpkt.size() < this->winSize) {
					this->waitingState = false;																	//ȡ���ȴ�״̬
				}
			}
		}
	}
}

void SRRdtSender::timeoutHandler(int seqNum) {
	int temp = (seqNum + this->maxSeq - this->sndpkt.front().seqnum) % this->maxSeq;
	if (temp >= 0 && temp < this->sndpkt.size()) {
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->sndpkt[(seqNum + this->maxSeq - this->sndpkt.front().seqnum) % this->maxSeq]);
		pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
		pns->sendToNetworkLayer(RECEIVER, this->sndpkt[(seqNum + this->maxSeq - this->sndpkt.front().seqnum) % this->maxSeq]);			//���·������ݰ�
	}
}
