#include "stdafx.h"
#include "Global.h"
#include "SRRdtReceiver.h"
#include "SRConfig.h"

SRRdtReceiver::SRRdtReceiver():rcvPkt(WIN_SIZ),maxSeq(1<<SEQ_BIT)
{
	f.open("D:\\destop\\Computer_Network_Lab\\lab2\\RDT\\SR\\receiverWindow.txt");
	for (int i = 0; i < WIN_SIZ; i++) {
		rcvPkt[i].acknum = 0;//����Ƿ���յ�
		rcvPkt[i].checksum = 0;
		rcvPkt[i].seqnum = i;
	}
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


SRRdtReceiver::~SRRdtReceiver()
{
	f.close();
}

void SRRdtReceiver::receive(const Packet &packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum) {
		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
		int t = (packet.seqnum + this->maxSeq - this->rcvPkt.front().seqnum) % this->maxSeq;//������յ������ݰ��ڴ����е��±�
		if (t<WIN_SIZ) {
			this->rcvPkt[t]=packet;
			this->rcvPkt[t].acknum=1;
		}
		while (this->rcvPkt.front().acknum == 1) {
			cout << "����ǰ���շ������б������Ϊ��";
			f << "����ǰ���շ������б������Ϊ��";
			for (int i = 0; i < rcvPkt.size(); i++) {
				if (rcvPkt[i].acknum) {
					f << rcvPkt[i].seqnum << " ";
					cout << rcvPkt[i].seqnum << " ";
				}
			}
			f << endl;
			cout << endl;
			Message msg;
			memcpy(msg.data, rcvPkt.front().payload, sizeof(rcvPkt.front().payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			rcvPkt.pop_front();
			Packet p;
			p.acknum = 0;
			p.seqnum = (rcvPkt.back().seqnum + 1) % maxSeq;
			rcvPkt.push_back(p);
			cout << "��������շ������б�����Ż���Ϊ��";//��ӡ��Ϣ����ʾ�ڻ�������֮ǰ�������е����ݰ����к�
			f << "��������շ������б�����Ż���Ϊ��";
			for (int i = 0; i < rcvPkt.size(); i++) {
				if (rcvPkt[i].acknum) {
					f << rcvPkt[i].seqnum << " ";
					cout << rcvPkt[i].seqnum << " ";
				}
			}
			f << "\n"<<endl;
			cout <<"\n"<< endl;
		}
		lastAckPkt.acknum = packet.seqnum; //�����ϴ�ȷ�ϵ����ݰ���ȷ�Ϻ�Ϊ���յ������ݰ������к�
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
	}
}