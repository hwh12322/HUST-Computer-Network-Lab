#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"
#include "GBNConfig.h"

GBNRdtSender::GBNRdtSender() : nextSeqnum(0),waitingState(false),winSize(WIN_SIZE),maxSeq(1<<SEQ_BIT),base(0)
{
	f.open("D:\\destop\\Computer_Network_Lab\\lab2\\RDT\\GBN\\senderWindow.txt");
}
//���ܣ���ʼ�����к�Ϊ0��
//���÷��ͷ��ĵȴ�״̬Ϊfalse���ǵȴ�״̬����
//����Ԥ�����WIN_SIZE�����ô��ڴ�С��
//ʹ��SEQ_BIT���������кŵ����ֵ��
//��ʼ�������Ϊ0��
//��һ���ļ�����д���¼��

GBNRdtSender::~GBNRdtSender()//�����������ͷŶ�����Դ����ȷ���򿪵��ļ����رա�
{
	f.close();
}



bool GBNRdtSender::getWaitingState() //���ص�ǰ���ͷ��Ƿ��ڵȴ�״̬
{
	return waitingState;
}




bool GBNRdtSender::send(const Message& message) {
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}
	Packet packetWaitingACK;
	packetWaitingACK.acknum = -1; //���Ը��ֶ�
	packetWaitingACK.seqnum = this->nextSeqnum;
	packetWaitingACK.checksum = 0;
	memcpy(packetWaitingACK.payload, message.data, sizeof(message.data));//�������ݵ�������
	packetWaitingACK.checksum = pUtils->calculateCheckSum(packetWaitingACK);
	this->sndpkt.push_back(packetWaitingACK);//�� packetWaitingACK���ݰ���ӵ�sndpkt��ĩβ
	pUtils->printPacket("���ͷ����ͱ���", this->sndpkt.back());
	if(base==nextSeqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->sndpkt.back());	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	if (this->sndpkt.size() >= this->winSize) {
		this->waitingState = true;																	//����ȴ�״̬
	}
	this->nextSeqnum = (this->nextSeqnum + 1) % this->maxSeq;
	return true;
}

void GBNRdtSender::receive(const Packet& ackPkt) {
	if (!this->sndpkt.empty()) {
		//���У����Ƿ���ȷ
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
		if (checkSum == ackPkt.checksum) {
			pns->stopTimer(SENDER, 0);		//�رն�ʱ��
			this->base = (ackPkt.acknum + 1) % this->maxSeq;
			pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
			if(base != nextSeqnum){//��������Ƿ������һ���������к�
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//�������ͷ���ʱ��
			}
			while (!sndpkt.empty() && sndpkt.front().seqnum != base) {//���в�Ϊ���Ҷ������ݰ������кŲ����ڵ�ǰ�Ļ����
				cout << "����ǰ�����б������Ϊ��";
				f << "����ǰ�����б������Ϊ��";
				for (auto p:sndpkt) {//ѭ������sndpkt����ӡ�ͼ�¼ÿ�����ݰ������к�
					f << p.seqnum<< " ";
					cout << p.seqnum<< " ";
				}
				cout << endl;
				f << endl;
				int newbase = (sndpkt.front().seqnum + 1) % maxSeq;//�����µĻ����
				this->sndpkt.pop_front();//�Ӷ�����ɾ����ȷ�ϵ����ݰ�
				cout << "�����󴰿��б������Ϊ��";
				f << "�����󴰿��б������Ϊ��";
				for (auto p : sndpkt) {
					f << p.seqnum << " ";
					cout << p.seqnum << " ";
				}
				cout<<"\n"<< endl;
				f <<"\n"<< endl;
			}
			if (this->sndpkt.size() < this->winSize) {//��鵱ǰ��ȷ�ϵ����ݰ������Ƿ�С�ڴ��ڴ�С������ǣ����ͷ������˳��ȴ�״̬
				this->waitingState = false;																	
			}
		}
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	for (Packet& p : this->sndpkt) {
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", p);
		pns->sendToNetworkLayer(RECEIVER, p);			//���·������ݰ�
	}

}