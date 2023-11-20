#include "stdafx.h"
#include "Global.h"
#include "GBNRdtSender.h"
#include "GBNConfig.h"

GBNRdtSender::GBNRdtSender() : nextSeqnum(0),waitingState(false),winSize(WIN_SIZE),maxSeq(1<<SEQ_BIT),base(0)
{
	f.open("D:\\destop\\Computer_Network_Lab\\lab2\\RDT\\GBN\\senderWindow.txt");
}
//功能：初始化序列号为0。
//设置发送方的等待状态为false（非等待状态）。
//根据预定义的WIN_SIZE来设置窗口大小。
//使用SEQ_BIT来计算序列号的最大值。
//初始化基序号为0。
//打开一个文件用于写入记录。

GBNRdtSender::~GBNRdtSender()//析构函数，释放对象资源，并确保打开的文件被关闭。
{
	f.close();
}



bool GBNRdtSender::getWaitingState() //返回当前发送方是否处于等待状态
{
	return waitingState;
}




bool GBNRdtSender::send(const Message& message) {
	if (this->waitingState) { //发送方处于等待确认状态
		return false;
	}
	Packet packetWaitingACK;
	packetWaitingACK.acknum = -1; //忽略该字段
	packetWaitingACK.seqnum = this->nextSeqnum;
	packetWaitingACK.checksum = 0;
	memcpy(packetWaitingACK.payload, message.data, sizeof(message.data));//拷贝数据到缓冲区
	packetWaitingACK.checksum = pUtils->calculateCheckSum(packetWaitingACK);
	this->sndpkt.push_back(packetWaitingACK);//将 packetWaitingACK数据包添加到sndpkt的末尾
	pUtils->printPacket("发送方发送报文", this->sndpkt.back());
	if(base==nextSeqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, this->sndpkt.back());	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	if (this->sndpkt.size() >= this->winSize) {
		this->waitingState = true;																	//进入等待状态
	}
	this->nextSeqnum = (this->nextSeqnum + 1) % this->maxSeq;
	return true;
}

void GBNRdtSender::receive(const Packet& ackPkt) {
	if (!this->sndpkt.empty()) {
		//检查校验和是否正确
		int checkSum = pUtils->calculateCheckSum(ackPkt);

		//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
		if (checkSum == ackPkt.checksum) {
			pns->stopTimer(SENDER, 0);		//关闭定时器
			this->base = (ackPkt.acknum + 1) % this->maxSeq;
			pUtils->printPacket("发送方正确收到确认", ackPkt);
			if(base != nextSeqnum){//检查基序号是否等于下一个发送序列号
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//启动发送方定时器
			}
			while (!sndpkt.empty() && sndpkt.front().seqnum != base) {//队列不为空且队首数据包的序列号不等于当前的基序号
				cout << "滑动前窗口中报文序号为：";
				f << "滑动前窗口中报文序号为：";
				for (auto p:sndpkt) {//循环遍历sndpkt，打印和记录每个数据包的序列号
					f << p.seqnum<< " ";
					cout << p.seqnum<< " ";
				}
				cout << endl;
				f << endl;
				int newbase = (sndpkt.front().seqnum + 1) % maxSeq;//计算新的基序号
				this->sndpkt.pop_front();//从队列中删除已确认的数据包
				cout << "滑动后窗口中报文序号为：";
				f << "滑动后窗口中报文序号为：";
				for (auto p : sndpkt) {
					f << p.seqnum << " ";
					cout << p.seqnum << " ";
				}
				cout<<"\n"<< endl;
				f <<"\n"<< endl;
			}
			if (this->sndpkt.size() < this->winSize) {//检查当前待确认的数据包数量是否小于窗口大小。如果是，则发送方可以退出等待状态
				this->waitingState = false;																	
			}
		}
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	for (Packet& p : this->sndpkt) {
		pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", p);
		pns->sendToNetworkLayer(RECEIVER, p);			//重新发送数据包
	}

}